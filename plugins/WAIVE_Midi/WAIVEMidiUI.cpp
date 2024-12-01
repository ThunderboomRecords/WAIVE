#include "WAIVEMidiUI.hpp"

START_NAMESPACE_DISTRHO

WAIVEMidiUI::WAIVEMidiUI() : UI(UI_W, UI_H),
                             fScaleFactor(getScaleFactor())
{
    plugin = static_cast<WAIVEMidi *>(getPluginInstancePointer());

    float width = UI_W * fScaleFactor;
    float height = UI_H * fScaleFactor;
    float padding = 8.f * fScaleFactor;

    std::cout << "UI_W: " << UI_W << " UI_H: " << UI_H << std::endl;

    logo_font = createFontFromMemory("VG5000", VG5000, VG5000_len, false);

    float col2_width = (350 + 35 + 35) * fScaleFactor + 3.f * padding;
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

    score_label->above(score_grid, Widget_Align::START, padding * 2);

    score_genre = new DropDown(this);
    score_genre->setSize(80, score_genre->getFontSize(), true);
    for (int i = 0; i < 22; i++)
        score_genre->addItem(score_genres[i]);
    score_genre->setDisplayNumber(8);
    score_genre->resizeToFit();
    score_genre->setItem(plugin->getParameterValue(kScoreGenre), false);
    score_genre->setFontSize(18.0f);
    score_genre->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    score_genre->menu->setFontSize(18.0f);
    score_genre->menu->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    score_genre->above(score_grid, Widget_Align::END, padding);
    score_genre->setCallback(this);

    new_score->leftOf(score_genre, Widget_Align::END, padding);
    score_genre->rightOf(new_score, Widget_Align::CENTER, padding);

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

    groove_genre = new DropDown(this);
    groove_genre->setSize(80, groove_genre->getFontSize(), true);
    for (int i = 0; i < 22; i++)
        groove_genre->addItem(groove_genres[i]);
    groove_genre->setDisplayNumber(8);
    groove_genre->resizeToFit();
    groove_genre->setItem(plugin->getParameterValue(kGrooveGenre), false);
    groove_genre->setFontSize(18.0f);
    groove_genre->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    groove_genre->menu->setFontSize(18.0f);
    groove_genre->menu->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    groove_genre->above(groove_graph, Widget_Align::END, padding);
    groove_genre->setCallback(this);

    new_groove = new Button(this);
    new_groove->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    new_groove->setLabel("new");
    new_groove->resizeToFit();
    new_groove->setCallback(this);
    new_groove->leftOf(groove_genre, Widget_Align::END, padding);

    groove_genre->rightOf(new_groove, Widget_Align::CENTER, padding);

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

    VBox thresholdsContainer(this);
    thresholdsContainer.setHeight(drum_pattern->getHeight());

    thresholds.reserve(9);
    for (int i = 8; i >= 0; i--)
    {
        auto t = std::make_shared<Knob>(this);
        t->setId(kThreshold1 + i);
        t->setSize(35, 20);
        t->setRadius(12.f);
        t->gauge_width = 3.f * fScaleFactor;
        t->min = 0.0f;
        t->max = 1.0f;
        t->setValue(plugin->getParameterValue(kThreshold1 + i));
        t->setCallback(this);
        t->resizeToFit();

        thresholds.push_back(std::move(t));
        thresholdsContainer.addWidget(thresholds.back().get());
    }

    thresholdsContainer.align_items = VBox::Align_Items::left;
    thresholdsContainer.justify_content = VBox::Justify_Content::space_evenly;
    thresholdsContainer.rightOf(drum_pattern, Widget_Align::CENTER, padding);
    thresholdsContainer.positionWidgets();

    threshold = new Knob(this);
    threshold->setId(kThreshold);
    threshold->setCallback(this);
    threshold->min = 0.0f;
    threshold->max = 1.0f;
    threshold->setValue(plugin->getParameterValue(kThreshold));
    threshold->gauge_width = 3.f * fScaleFactor;
    threshold->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    threshold->setRadius(12.f);
    threshold->resizeToFit();
    threshold->above(thresholds[0].get(), Widget_Align::CENTER, padding);

    threshold_label = new Label(this, "complexity");
    threshold_label->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    threshold_label->text_color = WaiveColors::text;
    threshold_label->resizeToFit();
    threshold_label->leftOf(threshold, Widget_Align::CENTER, padding);

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
        midiNote->rightOf(thresholds[i].get(), Widget_Align::CENTER, padding);

        midi_notes.push_back(std::move(midiNote));
    }

    drum_playhead = new Playhead(this);
    drum_playhead->setAbsolutePos(drum_pattern->getAbsolutePos());
    drum_playhead->setSize(drum_pattern->getSize(), true);
    drum_playhead->progress = &plugin->progress;
    addIdleCallback(drum_playhead);

    quantize = new Button(this);
    quantize->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    quantize->setLabel("quantize");
    quantize->resizeToFit();
    quantize->above(drum_pattern, Widget_Align::START, padding);
    quantize->isToggle = true;
    quantize->setToggled(false);
    quantize->setCallback(this);

    export_btn = new Button(this);
    export_btn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    export_btn->setLabel("export...");
    export_btn->resizeToFit();
    export_btn->below(drum_pattern, Widget_Align::END, padding);
    export_btn->setCallback(this);

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
        for (int i = 0; i < 9; i++)
            thresholds[i].get()->setValue(value);
        break;
    case kScoreGenre:
        score_genre->setItem((int)value, false);
        break;
    case kGrooveGenre:
        groove_genre->setItem((int)value, false);
        break;
    case kThreshold1:
    case kThreshold2:
    case kThreshold3:
    case kThreshold4:
    case kThreshold5:
    case kThreshold6:
    case kThreshold7:
    case kThreshold8:
    case kThreshold9:
        thresholds[index - kThreshold1]->setValue(value);
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
    else if (button == quantize)
    {
        plugin->quantize = button->getToggled();
        plugin->generateFullPattern();
    }
    else if (button == export_btn)
    {
        // create suitable file name
        std::string saveName = score_genres[plugin->score_genre];
        if (plugin->score_genre != plugin->groove_genre)
        {
            saveName += "_";
            saveName += groove_genres[plugin->groove_genre];
        }
        saveName += ".mid";

        char const *filename = nullptr;
        char const *filterPatterns[1] = {"*.mp3"};
        filename = tinyfd_saveFileDialog(
            "Export MIDI file...",
            saveName.c_str(),
            1,
            filterPatterns,
            "MIDI files");

        if (filename)
            setState("export", filename);
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
    setParameterValue(knob->getId(), value);

    if (knob == threshold)
        for (int i = 0; i < 9; i++)
            thresholds[i].get()->setValue(value);
};

void WAIVEMidiUI::dropdownSelection(DropDown *widget, int item)
{
    std::cout << widget->getId() << " set to " << item << std::endl;

    if (widget == score_genre)
        setParameterValue(kScoreGenre, item);
    else if (widget == groove_genre)
        setParameterValue(kGrooveGenre, item);
    else
        plugin->setMidiNote(widget->getId(), (uint8_t)item);
}

END_NAMESPACE_DISTRHO
