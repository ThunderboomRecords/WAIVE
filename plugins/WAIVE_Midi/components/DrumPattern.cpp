#include "DrumPattern.hpp"

START_NAMESPACE_DISTRHO

DrumPattern::DrumPattern(Widget *parent) noexcept
    : WAIVEWidget(parent), selectedNote(nullptr), hoveredNote(nullptr)
{
    for (int i = 0; i < 9; i++)
        midiToRow.insert({midiMap[i], i});
}

bool DrumPattern::onMouse(const MouseEvent &ev)
{
    if (ev.button != kMouseButtonLeft)
        return false;

    if (ev.press && dragState == DragState::NONE && contains(ev.pos))
    {
        dragState = DragState::CLICKING;
        return true;
    }
    else if (!ev.press)
    {
        if (dragState == DragState::CLICKING)
        {
            // Click released
            std::cout << "Click released" << std::endl;

            if (hoveredNote == nullptr)
            {
                int instrument = std::floor((1.0 - (ev.pos.getY() / static_cast<double>(getHeight()))) * 9.0);

                int sixteenth = std::floor(ev.pos.getX() / static_cast<double>(getWidth()) * 16.f);
                sixteenth = std::clamp(sixteenth, 0, 15);

                if (callback != nullptr)
                    callback->onDrumPatternClicked(this, instrument, sixteenth);

                hoveredNote = findHoveredNote(ev.pos);
            }
            else
            {
                hoveredNote->trigger->active = !hoveredNote->trigger->active;
                hoveredNote->active = hoveredNote->trigger->active;
                if (hoveredNote->other)
                    hoveredNote->other->active = hoveredNote->trigger->active;

                if (hoveredNote->user && callback != nullptr)
                    callback->onNoteDeleted(this, hoveredNote);
            }
        }
        else if (dragState == DragState::DRAGGING)
        {
            // Drag released
            std::cout << "Drag released" << std::endl;

            if (selectedNote != nullptr)
            {
                if (callback != nullptr)
                    callback->onDrumPatternNoteMoved(this, selectedNote, newTick);
                selectedNote = nullptr;
                hoveredNote = findHoveredNote(ev.pos);
            }
        }

        dragState = DragState::NONE;
        return false;
    }

    return false;
}

bool DrumPattern::onMotion(const MotionEvent &ev)
{
    if (dragState == CLICKING)
    {
        dragState = DragState::DRAGGING;
        std::cout << "Drag started" << std::endl;
        dragStart = ev.pos;

        selectedNote = hoveredNote;

        if (selectedNote != nullptr)
            newTick = selectedNote->tick;

        return true;
    }
    else if (dragState == DRAGGING)
    {
        auto difference = DGL::Point(ev.pos) - dragStart;
        // std::cout << "Drag continued: dX " << difference.getX() << ", dY " << difference.getY() << std::endl;

        if (selectedNote != nullptr)
        {
            // selectedNote->offset = std::floor(1920.0f * 4.f * difference.getX() / getWidth());
            newTick = selectedNote->tick + std::floor(1920.0f * 4.f * difference.getX() / getWidth());
            newTick = std::clamp(newTick, static_cast<uint32_t>(0), static_cast<uint32_t>(1920 * 4));
            // std::cout << "newTick: " << newTick << std::endl;
        }

        return true;
    }
    else if (contains(ev.pos))
    {
        hoveringGrid = true;
        hoveredNote = findHoveredNote(ev.pos);
        hover = ev.pos;

        return true;
    }

    hoveringGrid = false;
    hoveredNote = nullptr;

    return false;
}

bool DrumPattern::onScroll(const ScrollEvent &ev)
{
    if (!contains(ev.pos))
        return false;

    if (hoveredNote != nullptr)
    {
        // std::cout << "DrumPattern::onScroll " << ev.direction << ": " << ev.delta.getX() << ", " << ev.delta.getY() << std::endl;
        if (callback != nullptr)
            callback->onDrumPatternScrolled(this, hoveredNote, ev.delta.getY());
    }
    return true;
}

std::shared_ptr<Note> DrumPattern::findHoveredNote(const DGL::Point<double> &pos)
{
    const float width = getWidth();
    const float height = getHeight();

    const float gridWidth = width / 16.0f;
    const float gridHeight = height / 9.0f;

    float tpb = 1920.0f;
    float beatWidth = width / 4.0f;

    // TODO: make a bit more efficient? first check if still in hoveredNote
    for (auto noteOn : notes[0])
    {
        if (!noteOn->noteOn)
            continue;

        if (noteOn->other == nullptr)
            continue;

        Note noteOff = *noteOn->other;
        uint32_t startTick = noteOn->tick + noteOn->offset;
        uint32_t endTick = noteOff.tick + noteOff.offset;
        int row = noteOn->instrument;

        float x = beatWidth * (startTick / tpb);
        float y = gridHeight * (8 - row);
        float h = gridHeight;
        float w = beatWidth * (endTick - startTick) / tpb;

        Rectangle<double> bounds(x, y, w, h);

        if (bounds.contains(pos))
            return noteOn;
    }

    return nullptr;
}

void DrumPattern::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    const float gridWidth = width / 16.0f;
    const float gridHeight = height / 9.0f;

    int hoverX = std::floor(16.f * hover.getX() / width);
    int hoverY = std::floor(9.f * hover.getY() / height);

    beginPath();
    fillColor(WaiveColors::grey2);
    rect(0, 0, width, height);
    fill();
    closePath();

    beginPath();
    fillColor(WaiveColors::grey3);
    rect(4 * gridWidth, 0, 4 * gridWidth, height);
    rect(12 * gridWidth, 0, 4 * gridWidth, height);
    fill();
    closePath();

    beginPath();
    strokeColor(WaiveColors::grey1);
    strokeWidth(2.f);
    for (int i = 1; i < 16; i++)
    {
        moveTo(i * gridWidth, 0);
        lineTo(i * gridWidth, height);
    }

    for (int i = 1; i < 9; i++)
    {
        moveTo(0, i * gridHeight);
        lineTo(width, i * gridHeight);
    }
    stroke();
    closePath();

    float tpb = 1920.0f;
    float beatWidth = width / 4.0f;

    std::lock_guard<std::mutex> lk(*noteMtx);

    if (hoveringGrid && dragState == DragState::NONE && hoveredNote == nullptr)
    {
        beginPath();
        fillColor(1.f, 1.f, 1.f, 0.1f);
        rect(gridWidth * hoverX, gridHeight * hoverY, gridWidth, gridHeight);
        fill();
        closePath();
    }

    for (auto noteOn : notes[0])
    {
        if (!noteOn->noteOn)
            continue;

        if (noteOn->other == nullptr)
            continue;

        Note noteOff = *noteOn->other;
        uint32_t startTick = noteOn->tick + noteOn->offset;
        uint32_t endTick = noteOff.tick + noteOff.offset;
        int row = noteOn->instrument;

        float x = beatWidth * (startTick / tpb);
        float y = gridHeight * (8 - row);
        float h = gridHeight;
        float w = beatWidth * (endTick - startTick) / tpb;

        uint8_t velocity = noteOn->velocity;

        Color base, borderColor, top;
        float hue = noteOn->user ? 280.f : 240.f;

        base = Color::fromHSL(hue / 360.f, 0.55f, 0.20f);
        top = Color::fromHSL(hue / 360.f, 0.55f, 0.58f);
        base.interpolate(top, static_cast<float>(velocity) / 127.0f);
        borderColor = Color::fromHSL(hue / 360.f, 0.64f, 0.66f);

        if (!noteOn->active)
        {
            base.alpha = 0.3f;
            borderColor.alpha = 0.5f;
        }

        beginPath();
        fillColor(base);
        rect(x, y, w, h);
        fill();
        strokeWidth(1.f);
        strokeColor(borderColor);
        stroke();
        closePath();
    }

    if (hoveredNote != nullptr && hoveredNote->other != nullptr && selectedNote == nullptr)
    {
        uint32_t startTick = hoveredNote->tick + hoveredNote->offset;
        uint32_t endTick = hoveredNote->other->tick + hoveredNote->other->offset;
        int row = hoveredNote->instrument;

        float x = beatWidth * (startTick / tpb);
        float y = gridHeight * (8 - row);
        float h = gridHeight;
        float w = beatWidth * (endTick - startTick) / tpb;
        beginPath();
        rect(x - 1, y - 1, w, h);
        strokeWidth(2.f);
        strokeColor(text_color);
        stroke();
        closePath();
    }

    if (selectedNote != nullptr)
    {
        uint32_t endTick = newTick + tpb / 4;
        int row = selectedNote->instrument;

        float x = beatWidth * (newTick / tpb);
        float y = gridHeight * (8 - row);
        float h = gridHeight;
        float w = beatWidth * (endTick - newTick) / tpb;
        beginPath();
        rect(x - 1, y - 1, w, h);
        strokeWidth(2.f);
        strokeColor(text_color);
        stroke();
        closePath();
    }
}

void DrumPattern::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO