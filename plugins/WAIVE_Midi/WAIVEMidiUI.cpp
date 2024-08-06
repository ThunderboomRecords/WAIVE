#include "WAIVEMidiUI.hpp"

START_NAMESPACE_DISTRHO

WAIVEMidiUI::WAIVEMidiUI() : UI(UI_W, UI_H),
                             fScaleFactor(getScaleFactor())
{
    plugin = static_cast<WAIVEMidi *>(getPluginInstancePointer());

    float width = UI_W * fScaleFactor;
    float height = UI_H * fScaleFactor;

    std::cout << "UI_W: " << UI_W << " UI_H: " << UI_H << std::endl;

    logo_font = createFontFromMemory("VG5000", VG5000, VG5000_len, false);

    float padding = 10.f;
    float col2_width = (350 + 35) * fScaleFactor + 3.f * padding;
    float col1_width = width - col2_width - 3.f * padding;
    float col_height = height - 2.f * padding;

    edit_panel = new Panel(this);
    edit_panel->setSize(col1_width, col_height, true);
    edit_panel->setAbsolutePos(padding, padding);
    edit_panel->setFont("VG5000", VG5000, VG5000_len);
    edit_panel->label = "1";
    edit_panel->title = "Edit";

    score_label = new Label(this, "score");
    score_label->setFont("VG5000", VG5000, VG5000_len);
    score_label->text_color = WaiveColors::text;
    score_label->resizeToFit();

    new_score = new Button(this);
    new_score->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    new_score->setLabel("new");
    new_score->resizeToFit();
    new_score->setCallback(this);

    score_grid = new ScoreGrid(this);
    score_grid->setSize(350, 250);
    score_grid->onTop(edit_panel, Widget_Align::END, Widget_Align::START, padding, edit_panel->getFontSize() * 2.f + new_score->getHeight() + padding);
    score_grid->fScore = &plugin->fScore;
    score_grid->ui = this;

    score_label->above(score_grid, Widget_Align::START, padding);

    new_score->above(score_grid, Widget_Align::END, padding);

    var_score = new Button(this);
    var_score->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    var_score->setLabel("variation");
    var_score->resizeToFit();
    var_score->setCallback(this);
    var_score->leftOf(new_score, Widget_Align::CENTER, padding);

    VBox labels(this);
    labels.setHeight(score_grid->getHeight());

    drum_names.reserve(9);
    for (int i = 8; i >= 0; i--)
    {
        auto l = std::make_shared<Label>(this, midiNoteLabels[i]);

        l->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        l->text_color = WaiveColors::text;
        l->resizeToFit();

        drum_names.push_back(std::move(l));
        labels.addWidget(drum_names.back().get());
    }

    labels.align_items = VBox::Align_Items::right;
    labels.justify_content = VBox::Justify_Content::space_evenly;
    labels.leftOf(score_grid, Widget_Align::START, padding);
    labels.positionWidgets();

    groove_graph = new GrooveGraph(this);
    groove_graph->setSize(350, 50);
    groove_graph->fGroove = &plugin->fGroove;
    groove_graph->callback = this;
    groove_graph->onTop(edit_panel, Widget_Align::END, Widget_Align::END, padding);

    groove_label = new Label(this, "groove");
    groove_label->setFont("VG5000", VG5000, VG5000_len);
    groove_label->text_color = WaiveColors::text;
    groove_label->resizeToFit();
    groove_label->above(groove_graph, Widget_Align::START, padding * 2);

    new_groove = new Button(this);
    new_groove->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    new_groove->setLabel("new");
    new_groove->resizeToFit();
    new_groove->setCallback(this);
    new_groove->above(groove_graph, Widget_Align::END, padding);

    var_groove = new Button(this);
    var_groove->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    var_groove->setLabel("variation");
    var_groove->resizeToFit();
    var_groove->setCallback(this);
    var_groove->leftOf(new_groove, Widget_Align::CENTER, padding);

    result_panel = new Panel(this);
    result_panel->setSize(col2_width, col_height, true);
    result_panel->rightOf(edit_panel, Widget_Align::START, padding);
    result_panel->setFont("VG5000", VG5000, VG5000_len);
    result_panel->label = "2";
    result_panel->title = "Result";

    drum_pattern = new DrumPattern(this);
    drum_pattern->setSize(350, 250);
    drum_pattern->notes = &plugin->notes;
    drum_pattern->noteMtx = &plugin->noteMtx;
    drum_pattern->onTop(result_panel, Widget_Align::START, Widget_Align::START, padding, Layout::measureVertical(edit_panel, Widget_Align::START, score_grid, Widget_Align::START));

    VBox midiNoteSelects(this);
    midiNoteSelects.setHeight(drum_pattern->getHeight());

    midi_notes.reserve(9);
    for (int i = 8; i >= 0; i--)
    {
        auto midiNote = std::make_shared<DropDown>(this);
        for (int j = 0; j < 128; j++)
            midiNote->addItem(std::to_string(j).c_str());
        midiNote->setDisplayNumber(6);
        midiNote->alignment = Align::ALIGN_RIGHT;
        midiNote->setItem(midiMap[i], false);
        midiNote->setFont("VG5000", VG5000, VG5000_len);
        midiNote->menu->setFont("VG5000", VG5000, VG5000_len);
        midiNote->menu->alignment = Align::ALIGN_RIGHT;
        midiNote->setFontSize(16.f);
        midiNote->setSize(35, 20);
        midiNote->setId(i);
        midiNote->setCallback(this);

        midi_notes.push_back(std::move(midiNote));
        midiNoteSelects.addWidget(midi_notes.back().get());
    }

    midiNoteSelects.align_items = VBox::Align_Items::left;
    midiNoteSelects.justify_content = VBox::Justify_Content::space_evenly;
    midiNoteSelects.rightOf(drum_pattern, Widget_Align::CENTER, padding);
    midiNoteSelects.positionWidgets();

    threshold = new Knob(this);
    threshold->setId(kThreshold);
    threshold->setCallback(this);
    threshold->min = 0.1f;
    threshold->max = 1.0f;
    threshold->setValue(0.8f);
    threshold->gauge_width = 3.f * fScaleFactor;
    threshold->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    threshold->setRadius(12.f);
    threshold->resizeToFit();
    threshold->above(drum_pattern, Widget_Align::END, padding);

    threshold_label = new Label(this, "complexity");
    threshold_label->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    threshold_label->text_color = WaiveColors::text;
    threshold_label->resizeToFit();
    threshold_label->leftOf(threshold, Widget_Align::CENTER, padding);

    drum_playhead = new Playhead(this);
    drum_playhead->setAbsolutePos(drum_pattern->getAbsolutePos());
    drum_playhead->setSize(drum_pattern->getSize(), true);
    drum_playhead->progress = &plugin->progress;
    addIdleCallback(drum_playhead);

    setGeometryConstraints(width, height, false, false);

    if (fScaleFactor != 1.0)
        setSize(width, height);
}

WAIVEMidiUI::~WAIVEMidiUI() {}

void WAIVEMidiUI::parameterChanged(uint32_t index, float value)
{
    switch (index)
    {
    case kThreshold:
        threshold->setValue(value);
        break;
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
    fillColor(WaiveColors::dark);
    rect(0.0f, 0.0f, width, height);
    fill();
    closePath();
}

void WAIVEMidiUI::uiScaleFactorChanged(const double scaleFactor)
{
    fScaleFactor = scaleFactor;
}

void WAIVEMidiUI::buttonClicked(Button *button)
{
    // std::cout << "button clicked" << std::endl;
    if (button == new_score)
    {
        plugin->generateScore();
        plugin->generateFullPattern();
    }
    else if (button == new_groove)
    {
        plugin->generateGroove();
        plugin->generateFullPattern();
    }
    else if (button == var_score)
    {
        plugin->variationScore();
        plugin->generateFullPattern();
    }
    else if (button == var_groove)
    {
        plugin->variationGroove();
        plugin->generateFullPattern();
    }
}

void WAIVEMidiUI::grooveClicked(GrooveGraph *graph)
{
    plugin->encodeGroove();
    repaint();
}

void WAIVEMidiUI::knobDragStarted(Knob *knob) {};

void WAIVEMidiUI::knobDragFinished(Knob *knob, float value) {};

void WAIVEMidiUI::knobValueChanged(Knob *knob, float value)
{
    if (knob == threshold)
    {
        std::cout << "threshold changed" << std::endl;
        setParameterValue(kThreshold, value);
    }
};

void WAIVEMidiUI::dropdownSelection(DropDown *widget, int item)
{
    std::cout << widget->getId() << " set to " << item << std::endl;
    // midiMap[widget->getId()] = item;
    plugin->setMidiNote(widget->getId(), (uint8_t)item);
    // plugin->computeNotes();
}

END_NAMESPACE_DISTRHO
