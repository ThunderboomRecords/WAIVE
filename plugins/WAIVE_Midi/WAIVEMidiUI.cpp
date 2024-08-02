#include "WAIVEMidiUI.hpp"

START_NAMESPACE_DISTRHO

WAIVEMidiUI::WAIVEMidiUI() : UI(UI_W, UI_H),
                             fScaleFactor(getScaleFactor())
{
    plugin = static_cast<WAIVEMidi *>(getPluginInstancePointer());

    std::cout << "UI_W: " << UI_W << " UI_H: " << UI_H << std::endl;

    logo_font = createFontFromMemory("VG5000", VG5000, VG5000_len, false);

    float padding = 10.f;

    score_label = new Label(this, "score");
    score_label->setFont("VG5000", VG5000, VG5000_len);
    score_label->text_color = WaiveColors::text;
    score_label->resizeToFit();
    score_label->setAbsolutePos(120 * fScaleFactor, 40 * fScaleFactor);

    score_grid = new ScoreGrid(this);
    score_grid->setSize(350, 250);
    score_grid->below(score_label, Widget_Align::START, padding);
    score_grid->fScore = &plugin->fScore;
    score_grid->ui = this;

    VBox labels(this);
    labels.setHeight(score_grid->getHeight());

    drum_names.reserve(9);
    for (int i = 0; i < 9; i++)
    {
        auto l = std::make_shared<Label>(this, midiNoteLabels[i]);

        l->setFont("VG5000", VG5000, VG5000_len);
        l->text_color = WaiveColors::text;
        l->resizeToFit();

        drum_names.push_back(std::move(l));
        labels.addWidget(drum_names.back().get());
    }

    labels.align_items = VBox::Align_Items::right;
    labels.justify_content = VBox::Justify_Content::space_evenly;
    labels.leftOf(score_grid, Widget_Align::START, padding);
    labels.positionWidgets();

    groove_label = new Label(this, "groove");
    groove_label->setFont("VG5000", VG5000, VG5000_len);
    groove_label->text_color = WaiveColors::text;
    groove_label->resizeToFit();
    groove_label->below(score_grid, Widget_Align::START, padding * 2);

    groove_graph = new GrooveGraph(this);
    groove_graph->setSize(350, 50);
    groove_graph->fGroove = &plugin->fGroove;
    groove_graph->callback = this;
    groove_graph->below(groove_label, Widget_Align::START, padding);

    drum_pattern = new DrumPattern(this);
    drum_pattern->setSize(350, 250);
    drum_pattern->notes = &plugin->notes;
    drum_pattern->rightOf(score_grid, Widget_Align::START, padding * 2);

    drum_label = new Label(this, "result");
    drum_label->setFont("VG5000", VG5000, VG5000_len);
    drum_label->text_color = WaiveColors::text;
    drum_label->resizeToFit();
    drum_label->above(drum_pattern, Widget_Align::START, padding);

    drum_playhead = new Playhead(this);
    drum_playhead->setAbsolutePos(drum_pattern->getAbsolutePos());
    drum_playhead->setSize(drum_pattern->getSize(), true);
    drum_playhead->progress = &plugin->progress;
    addIdleCallback(drum_playhead);

    setGeometryConstraints(UI_W * fScaleFactor, UI_H * fScaleFactor, false, false);

    if (fScaleFactor != 1.0)
        setSize(UI_W * fScaleFactor, UI_H * fScaleFactor);
}

WAIVEMidiUI::~WAIVEMidiUI() {}

void WAIVEMidiUI::parameterChanged(uint32_t index, float value)
{
    switch (index)
    {
    default:
        break;
    }

    repaint();
}

void WAIVEMidiUI::stateChanged(const char *key, const char *value)
{
    printf("WAIVEMidiUI::stateChanged()\n");

    repaint();
}

void WAIVEMidiUI::onNanoDisplay()
{
    float width = getWidth();
    float height = getHeight();

    beginPath();
    fillColor(WaiveColors::grey1);
    rect(0.0f, 0.0f, width, height);
    fill();
    closePath();

    // beginPath();
    // fillColor(WaiveColors::text);
    // fontSize(32 * fScaleFactor);
    // textAlign(Align::ALIGN_RIGHT | Align::ALIGN_TOP);
    // fontFaceId(logo_font);
    // text(width - 10 * fScaleFactor, 4 * fScaleFactor, "waive", nullptr);
    // closePath();

    // beginPath();
    // fillColor(WaiveColors::text);
    // fontSize(16 * fScaleFactor);
    // textAlign(Align::ALIGN_LEFT | Align::ALIGN_BOTTOM);
    // fontFaceId(logo_font);
    // text(score_grid->getAbsoluteX(), score_grid->getAbsoluteY(), "score:", nullptr);
    // text(groove_graph->getAbsoluteX(), groove_graph->getAbsoluteY(), "groove:", nullptr);
    // text(drum_pattern->getAbsoluteX(), drum_pattern->getAbsoluteY(), "result:", nullptr);
    // closePath();

    // int row_height = score_grid->getHeight() / 9;
    // beginPath();
    // fillColor(WaiveColors::text);
    // fontSize(14 * fScaleFactor);
    // textAlign(Align::ALIGN_RIGHT | Align::ALIGN_CENTER);
    // fontFaceId(logo_font);
    // int x, y;
    // x = score_grid->getAbsoluteX() - 2;
    // y = score_grid->getAbsoluteY() + score_grid->getHeight() - row_height / 2;
    // for (int i = 0; i < 9; i++)
    // {
    //     text(x, y, midiNoteLabels[i], nullptr);
    //     y -= row_height;
    // }
    // closePath();
}

void WAIVEMidiUI::uiScaleFactorChanged(const double scaleFactor)
{
    fScaleFactor = scaleFactor;
}

void WAIVEMidiUI::grooveClicked(GrooveGraph *graph)
{
    plugin->encodeGroove();
    repaint();
}

END_NAMESPACE_DISTRHO
