#include "WAIVESamplerUI.hpp"
START_NAMESPACE_DISTRHO

WAIVESamplerUI::WAIVESamplerUI() : UI(UI_W, UI_H),
                                   fScaleFactor(getScaleFactor()),
                                   fScale(1.0f),
                                   filebrowserOpen(false),
                                   map_full(false)
{
    plugin = static_cast<WAIVESampler *>(getPluginInstancePointer());

    // register notifications
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskStartedNotification>(*this, &WAIVESamplerUI::onTaskStarted));
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFinishedNotification>(*this, &WAIVESamplerUI::onTaskFinished));

    logo_font = createFontFromMemory("VG5000", VG5000, VG5000_len, false);

    sample_map = new SampleMap(this);
    sample_map->setSize(520, 300);
    sample_map->setAbsolutePos(10, 46);
    sample_map->allSamples = &plugin->sd.fAllSamples;
    sample_map->selectedSample = &plugin->fCurrentSample;
    sample_map->background_color = Color(40, 40, 40);
    sample_map->setCallback(this);

    expand_map_btn = new Button(this);
    expand_map_btn->setSize(26, 26);
    expand_map_btn->setLabel("↘");
    expand_map_btn->setBackgroundColor(Color(40, 40, 40));
    expand_map_btn->setLabelColor(Color(100, 100, 100));
    expand_map_btn->setCallback(this);
    Layout::onTop(expand_map_btn, sample_map, Widget_Align::END, Widget_Align::END, 5);

    slots_container = new VBox(this);
    slots_container->setSize(UI_W - sample_map->getWidth() - 30, sample_map->getHeight());
    slots_container->padding = 2;
    slots_container->justify_content = VBox::Justify_Content::space_evenly;
    Layout::rightOf(slots_container, sample_map, Widget_Align::START, 10);
    createSampleSlots();
    slots_container->positionWidgets();

    map_label = new Label(this, "sample map");
    map_label->setFont("VG5000", VG5000, VG5000_len);
    map_label->label_size = 16.0f;
    map_label->resizeToFit();
    Layout::above(map_label, sample_map, Widget_Align::START, 5);

    import_sample_btn = new Button(this);
    import_sample_btn->setLabel("import sample");
    import_sample_btn->setBackgroundColor(Color(220, 220, 220));
    import_sample_btn->setLabelColor(Color(10, 10, 10));
    import_sample_btn->setSize(100, 20);
    Layout::rightOf(import_sample_btn, map_label, Widget_Align::CENTER, 20);
    import_sample_btn->setCallback(this);

    import_spinner = new Spinner(this);
    import_spinner->setSize(20, 20);
    Layout::rightOf(import_spinner, import_sample_btn, Widget_Align::CENTER, 5);
    addIdleCallback(import_spinner);

    sample_controls_label = new Label(this, "sample creator");
    sample_controls_label->setFont("VG5000", VG5000, VG5000_len);
    sample_controls_label->label_size = 16.0f;
    sample_controls_label->resizeToFit();
    Layout::below(sample_controls_label, sample_map, Widget_Align::START, 5);

    new_sample_btn = new Button(this);
    new_sample_btn->setLabel("new");
    new_sample_btn->setBackgroundColor(Color(220, 220, 220));
    new_sample_btn->setLabelColor(Color(10, 10, 10));
    new_sample_btn->setSize(70, 20);
    Layout::rightOf(new_sample_btn, sample_controls_label, Widget_Align::CENTER, 20);
    new_sample_btn->setCallback(this);

    sample_name = new TextInput(this);
    sample_name->setSize(200, 20);
    Layout::rightOf(sample_name, new_sample_btn, Widget_Align::CENTER, 20);
    sample_name->setCallback(this);

    source_display = new Waveform(this);
    source_display->setSize(UI_W - 20, 80);
    Layout::below(source_display, sample_controls_label, Widget_Align::START, 5);
    source_display->selectable = true;
    source_display->setCallback(this);
    source_display->lineColor = Color(255, 255, 255);
    source_display->setWaveform(&plugin->fSourceWaveform);
    source_display->setWaveformFeatures(&plugin->fSourceFeatures);

    open_source_btn = new Button(this);
    open_source_btn->setLabel("import source");
    open_source_btn->setBackgroundColor(Color(40, 40, 40));
    open_source_btn->setLabelColor(Color(200, 200, 200));
    open_source_btn->setSize(100, 20);
    Layout::onTop(open_source_btn, source_display, Widget_Align::END, Widget_Align::START, 2);
    open_source_btn->setCallback(this);

    browser_sources_btn = new Button(this);
    browser_sources_btn->setLabel("browse...");
    browser_sources_btn->setBackgroundColor(Color(40, 40, 40));
    browser_sources_btn->setLabelColor(Color(200, 200, 200));
    browser_sources_btn->setSize(100, 20);
    Layout::leftOf(browser_sources_btn, open_source_btn, Widget_Align::CENTER, 5);
    browser_sources_btn->setCallback(this);

    sample_display = new Waveform(this);
    sample_display->setSize(180, 80);
    Layout::below(sample_display, source_display, Widget_Align::END, 10.0f);
    sample_display->setWaveform(plugin->editorPreviewWaveform);

    save_sample_btn = new Button(this);
    save_sample_btn->setLabel("add");
    save_sample_btn->setBackgroundColor(Color(220, 220, 220));
    save_sample_btn->setLabelColor(Color(10, 10, 10));
    save_sample_btn->setSize(70, 20);
    Layout::below(save_sample_btn, sample_display, Widget_Align::END, 10.f);
    save_sample_btn->setCallback(this);
    save_sample_btn->setEnabled(false);

    play_btn = new Button(this);
    play_btn->setLabel("preview");
    play_btn->setBackgroundColor(Color(220, 220, 220));
    play_btn->setLabelColor(Color(10, 10, 10));
    play_btn->setSize(70, 20);
    Layout::below(play_btn, sample_display, Widget_Align::START, 10.f);
    play_btn->setCallback(this);
    play_btn->setEnabled(false);

    ampADSRKnobs = new HBox(this);
    ampADSRKnobs->setSize(300, 60);
    ampADSRKnobs->justify_content = HBox::Justify_Content::right;
    ampADSRKnobs->padding = 5;

    shapeKnobs = new HBox(this);
    shapeKnobs->setSize(300, 60);
    shapeKnobs->justify_content = HBox::Justify_Content::right;
    shapeKnobs->padding = 5;

    filterKnobs = new HBox(this);
    filterKnobs->justify_content = HBox::Justify_Content::right;
    filterKnobs->padding = 5;

    // Wave shaping
    pitch = createWAIVEKnob(this, kSamplePitch, "pitch", 0.25f, 4.f, 1.0f, logo_font);
    volume = createWAIVEKnob(this, kSampleVolume, "volume", 0.0f, 2.0f, 1.0f, logo_font);
    percussionBoost = createWAIVEKnob(this, kPercussiveBoost, "perc.", 0.0f, 1.0f, 1.0f, logo_font);

    // Amp ADSR
    ampAttack = createWAIVEKnob(this, kAmpAttack, "attack", 0.0f, 500.0f, 10.0f, logo_font);
    ampAttack->format = "{:.0f}ms";

    ampDecay = createWAIVEKnob(this, kAmpDecay, "decay", 0.0f, 500.0f, 50.0f, logo_font);
    ampDecay->format = "{:.0f}ms";

    ampSustain = createWAIVEKnob(this, kAmpSustain, "sustain", 0.0f, 1.0f, 0.7f, logo_font);

    ampRelease = createWAIVEKnob(this, kAmpRelease, "release", 0.0f, 500.0f, 100.0f, logo_font);
    ampRelease->format = "{:.0f}ms";

    sustainLength = createWAIVEKnob(this, kSustainLength, "length", 0.0f, 1000.0f, 100.f, logo_font);
    sustainLength->format = "{:.0f}ms";

    // Filter
    filterCutoff = createWAIVEKnob(this, kFilterCutoff, "cutoff", 0.0, 0.999, 0.999, logo_font);
    filterResonance = createWAIVEKnob(this, kFilterResonance, "res.", 0.0, 1.0, 0.0, logo_font);
    filterType = new DropDown(this);
    filterType->font_size = 16.0f;
    filterType->addItem("LP");
    filterType->addItem("HP");
    filterType->addItem("BP");
    filterType->setId(kFilterType);
    filterType->setFont("VG5000", VG5000, VG5000_len);
    filterType->setDisplayNumber(3);
    filterType->setSize(40, 20);
    filterType->setCallback(this);

    shapeKnobs->addWidget(pitch);
    shapeKnobs->addWidget(percussionBoost);
    shapeKnobs->addWidget(volume);
    shapeKnobs->resizeToFit();
    Layout::leftOf(shapeKnobs, sample_display, Widget_Align::CENTER, 10.f);
    shapeKnobs->positionWidgets();

    ampADSRKnobs->addWidget(ampAttack);
    ampADSRKnobs->addWidget(ampDecay);
    ampADSRKnobs->addWidget(ampSustain);
    ampADSRKnobs->addWidget(ampRelease);
    ampADSRKnobs->addWidget(sustainLength);
    ampADSRKnobs->resizeToFit();
    Layout::leftOf(ampADSRKnobs, shapeKnobs, Widget_Align::CENTER, 10.f);
    ampADSRKnobs->positionWidgets();

    filterKnobs->addWidget(filterType);
    filterKnobs->addWidget(filterCutoff);
    filterKnobs->addWidget(filterResonance);
    filterKnobs->resizeToFit();
    Layout::leftOf(filterKnobs, ampADSRKnobs, Widget_Align::CENTER, 10.f);
    filterKnobs->positionWidgets();

    sample_editor_controls = new WidgetGroup(this);
    sample_editor_controls->addChildWidget(filterKnobs);
    sample_editor_controls->addChildWidget(ampADSRKnobs);
    sample_editor_controls->addChildWidget(shapeKnobs);

    sample_map_menu = new Menu(this);
    sample_map_menu->addItem("Add to slot 1");
    sample_map_menu->addItem("Add to slot 2");
    sample_map_menu->addItem("Add to slot 3");
    sample_map_menu->addItem("Add to slot 4");
    sample_map_menu->addItem("Add to slot 5");
    sample_map_menu->addItem("Add to slot 6");
    sample_map_menu->addItem("Add to slot 7");
    sample_map_menu->addItem("Add to slot 8");

    sample_map_menu->setWidth(100);
    sample_map_menu->setFont("VG5000", VG5000, VG5000_len);
    sample_map_menu->setDisplayNumber(8);
    sample_map_menu->hide();
    sample_map_menu->setCallback(sample_map);

    sample_map->menu = sample_map_menu;

    dropdown_menu = new Menu(this);
    dropdown_menu->hide();
    dropdown_menu->setFont("VG5000", VG5000, VG5000_len);

    for (int i = 0; i < sampleSlots.size(); i++)
    {
        sampleSlots[i]->updateWidgetPositions();
        sampleMidiDropdowns[i]->menu = dropdown_menu;
    }
    filterType->menu = dropdown_menu;

    value_indicator = new ValueIndicator(this);
    value_indicator->setSize(70, 20);
    value_indicator->fontId = logo_font;
    value_indicator->hide();

    setSampleEditorVisible(false);

    addIdleCallback(this);

    setGeometryConstraints(UI_W * fScaleFactor, UI_H * fScaleFactor, false, false);

    if (fScaleFactor != 1.0)
        setSize(UI_W * fScaleFactor, UI_H * fScaleFactor);

    std::cout << "WAIVESamplerUI initialised" << std::endl;
}

WAIVESamplerUI::~WAIVESamplerUI()
{
    if (open_dialog.joinable())
        open_dialog.join();
}

void WAIVESamplerUI::parameterChanged(uint32_t index, float value)
{
    int slot = 0;
    switch (index)
    {
    case kSamplePitch:
        pitch->setValue(value, false);
        break;
    case kPercussiveBoost:
        percussionBoost->setValue(value, false);
        break;
    case kSampleVolume:
        volume->setValue(value, false);
        break;
    case kAmpAttack:
        ampAttack->setValue(value, false);
        break;
    case kAmpDecay:
        ampDecay->setValue(value, false);
        break;
    case kAmpSustain:
        ampSustain->setValue(value, false);
        break;
    case kAmpRelease:
        ampRelease->setValue(value, false);
        break;
    case kSustainLength:
        sustainLength->setValue(value, false);
        break;
    case kFilterCutoff:
        filterCutoff->setValue(value, false);
        break;
    case kFilterResonance:
        filterResonance->setValue(value, false);
        break;
    case kFilterType:
        filterType->setItem(value, false);
        break;
    case kSlot1MidiNumber:
    case kSlot2MidiNumber:
    case kSlot3MidiNumber:
    case kSlot4MidiNumber:
    case kSlot5MidiNumber:
    case kSlot6MidiNumber:
    case kSlot7MidiNumber:
    case kSlot8MidiNumber:
        slot = index - kSlot1MidiNumber;
        sampleMidiDropdowns[slot]->setItem(value, false);
        break;
    default:
        break;
    }

    repaint();
}

void WAIVESamplerUI::stateChanged(const char *key, const char *value)
{
    std::cout << "WAIVESamplerUI::stateChanged: " << key << " -> " << value << std::endl;

    if (strcmp(key, "filename") == 0)
    {
    }

    repaint();
}

void WAIVESamplerUI::knobDragStarted(Knob *knob)
{
    value_indicator->setAbsoluteX(knob->getAbsoluteX());
    value_indicator->setWidth(knob->getWidth());
    value_indicator->setAbsoluteY(knob->getAbsoluteY() + knob->getHeight());
    value_indicator->setFormatString(knob->getFormat());
    value_indicator->setValue(knob->getValue());
    value_indicator->toFront();
    value_indicator->show();
}

void WAIVESamplerUI::knobDragFinished(Knob *knob, float value)
{
    value_indicator->hide();
    repaint();

    plugin->triggerPreview();
}

void WAIVESamplerUI::knobValueChanged(Knob *knob, float value)
{
    setParameterValue(knob->getId(), value);
    value_indicator->setValue(knob->getValue());
}

void WAIVESamplerUI::buttonClicked(Button *button)
{
    if (button == open_source_btn || button == import_sample_btn)
    {
        if (filebrowserOpen)
            return;
        if (open_dialog.joinable())
            open_dialog.join();

        if (button == open_source_btn)
            open_dialog = std::thread(&WAIVESamplerUI::openFileBrowser, this, (char *)"filename", false);
        else if (button == import_sample_btn)
            open_dialog = std::thread(&WAIVESamplerUI::openFileBrowser, this, (char *)"import", true);
    }
    else if (button == save_sample_btn)
        plugin->addCurrentSampleToLibrary();
    else if (button == play_btn)
        plugin->triggerPreview();
    else if (button == new_sample_btn)
        plugin->newSample();
    else if (button == browser_sources_btn)
        plugin->sd.getSourcesList();
    else if (button == expand_map_btn)
    {
        if (map_full)
        {
            sample_map->toFront();
            sample_map->setSize(520, 300);

            expand_map_btn->setLabel("↘");
            expand_map_btn->toFront();
            Layout::onTop(expand_map_btn, sample_map, Widget_Align::END, Widget_Align::END, 5);
            map_full = false;
        }
        else
        {
            sample_map->toFront();
            sample_map->setSize(UI_W - 20, UI_H - 60);

            expand_map_btn->setLabel("↖");
            expand_map_btn->toFront();
            Layout::onTop(expand_map_btn, sample_map, Widget_Align::END, Widget_Align::END, 5);
            map_full = true;
        }
    }
    else
    {
        for (int i = 0; i < sampleTriggerButtons.size(); i++)
        {
            if (sampleTriggerButtons[i] == button)
            {
                plugin->samplePlayers[i].state = PlayState::TRIGGERED;
                break;
            }
        }
    }

    repaint();
}

void WAIVESamplerUI::openFileBrowser(char *state, bool multiple)
{
    filebrowserOpen = true;
    char const *filename;
    char const *filterPatterns[2] = {"*.mp3", "*.wav"};
    filename = tinyfd_openFileDialog(
        "Open audio file...",
        "./",
        2,
        filterPatterns,
        "Audio files",
        multiple);

    std::cout << filename << std::endl;
    if (filename)
        setState(state, filename);
    else
        std::cout << "No file selected" << std::endl;

    filebrowserOpen = false;
}

void WAIVESamplerUI::waveformSelection(Waveform *waveform, uint selectionStart)
{
    plugin->selectWaveform(&plugin->fSourceWaveform, (int)selectionStart);
}

void WAIVESamplerUI::mapSampleHovered(int id)
{
    plugin->loadPreview(id);
}

void WAIVESamplerUI::mapSampleSelected(int id)
{
    plugin->loadSample(id);
}

void WAIVESamplerUI::mapSampleLoadSlot(int id, int slot)
{
    plugin->loadSlot(slot, id);
}

void WAIVESamplerUI::textEntered(TextInput *textInput, std::string text)
{
    std::cout << "WAIVESamplerUI::textEntered " << text << std::endl;
    if (textInput == sample_name)
    {
        if (plugin->fCurrentSample != nullptr)
            plugin->sd.renameSample(plugin->fCurrentSample, text);
    }
}

void WAIVESamplerUI::dropdownSelection(DropDown *widget, int item)
{
    if (widget == filterType)
    {
        setParameterValue(widget->getId(), item);
    }
}

void WAIVESamplerUI::onNanoDisplay()
{
    float width = getWidth();
    float height = getHeight();

    beginPath();
    fillColor(Color(240, 240, 240));
    rect(0.0f, 0.0f, width, height);
    fill();
    closePath();

    beginPath();
    fillColor(Color(40, 40, 40));
    fontSize(32 * fScale * fScaleFactor);
    textAlign(Align::ALIGN_RIGHT | Align::ALIGN_TOP);

    fontFaceId(logo_font);
    text(width - 10 * fScale * fScaleFactor, 4 * fScale * fScaleFactor, "waive sampler", nullptr);
    closePath();
}

void WAIVESamplerUI::uiScaleFactorChanged(const double scaleFactor)
{
    fScaleFactor = scaleFactor;
}

void WAIVESamplerUI::idleCallback()
{
    std::queue<int> *updateQueue = &plugin->updateQueue;
    for (; !updateQueue->empty(); updateQueue->pop())
    {
        int msg = updateQueue->front();

        // std::cout << " - new msg: " << msg << std::endl;

        switch (msg)
        {
        case kSourceLoading:
            break;
        case kSourceLoaded:
            source_display->setWaveformLength(plugin->fSourceLength);
            source_display->waveformNew();
            save_sample_btn->setEnabled(true);
            break;
        case kSourceUpdated:
            source_display->setWaveformLength(plugin->fSourceLength);
            source_display->waveformUpdated();
            break;
        case kSampleLoading:
            break;
        case kSampleLoaded:
            sample_display->setWaveformLength(plugin->fCurrentSample->sampleLength);
            if (plugin->fCurrentSample->saved)
                save_sample_btn->setLabel("update");
            else
                save_sample_btn->setLabel("add");
            sample_display->waveformNew();
            play_btn->setEnabled(true);
            break;
        case kSampleUpdated:
            if (plugin->fCurrentSample != nullptr)
            {
                sample_display->setWaveformLength(plugin->fCurrentSample->sampleLength);
                sample_display->waveformUpdated();
                if (plugin->fCurrentSample->saved)
                    save_sample_btn->setLabel("update");
                else
                    save_sample_btn->setLabel("add");
            }
            play_btn->setEnabled(true);
            break;
        case kSampleAdded:
            sample_map->repaint();
            break;
        case kParametersChanged:
            if (plugin->fCurrentSample != nullptr)
            {
                setSampleEditorVisible(true);
                pitch->setValue(plugin->fCurrentSample->pitch, false);
                percussionBoost->setValue(plugin->fCurrentSample->percussiveBoost, false);
                volume->setValue(plugin->fCurrentSample->volume, false);
                sustainLength->setValue(plugin->fCurrentSample->sustainLength, false);
                filterCutoff->setValue(plugin->fCurrentSample->filterCutoff, false);
                filterResonance->setValue(plugin->fCurrentSample->filterResonance, false);
                filterType->setItem(plugin->fCurrentSample->filterType, false);
                ampAttack->setValue(plugin->ampEnvGen.getAttack(), false);
                ampDecay->setValue(plugin->ampEnvGen.getDecay(), false);
                ampSustain->setValue(plugin->ampEnvGen.getSustain(), false);
                ampRelease->setValue(plugin->ampEnvGen.getRelease(), false);

                source_display->setSelection(plugin->fCurrentSample->sourceStart, false);
                if (plugin->fCurrentSample->waive)
                {
                    save_sample_btn->setVisible(true);
                    if (plugin->fCurrentSample->saved)
                        save_sample_btn->setLabel("update");
                    else
                        save_sample_btn->setLabel("add");
                }
                else
                    save_sample_btn->setVisible(false);
                sample_name->setText(plugin->fCurrentSample->name.c_str(), false);
                new_sample_btn->setLabel("duplicate");
            }
            else
            {
                setSampleEditorVisible(false);
                new_sample_btn->setLabel("new");
            }

            sample_map->repaint();

            break;
        case kSlotLoaded:
            for (int i = 0; i < sampleSlots.size(); i++)
            {
                sampleSlots[i]->repaint();
            }
            break;
        default:
            std::cout << "Unknown update: " << msg << std::endl;
            break;
        }
    }
}

void WAIVESamplerUI::onTaskStarted(Poco::TaskStartedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    std::cout << "WAIVESamplerUI::onTaskStarted: " << pTask->name() << std::endl;
    if (pTask->name().compare("ImporterTask") == 0)
    {
        import_spinner->setLoading(true);
    }
    pTask->release();
}

void WAIVESamplerUI::onTaskFinished(Poco::TaskFinishedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    std::cout << "WAIVESamplerUI::onTaskFinished: " << pTask->name() << std::endl;
    if (pTask->name().compare("ImporterTask") == 0)
    {
        import_spinner->setLoading(false);
    }
    pTask->release();
}

void WAIVESamplerUI::setSampleEditorVisible(bool visible)
{
    source_display->setVisible(visible);
    sample_display->setVisible(visible);
    ampADSRKnobs->setVisible(visible);
    filterKnobs->setVisible(visible);
    shapeKnobs->setVisible(visible);
    open_source_btn->setVisible(visible);
    browser_sources_btn->setVisible(visible);
    save_sample_btn->setVisible(visible);
    play_btn->setVisible(visible);
    sample_name->setVisible(visible);
}

// Helper functions to set up UI
Knob3D *createWAIVEKnob(
    WAIVESamplerUI *parent,
    Parameters param,
    std::string label,
    float min,
    float max,
    float value,
    UI::FontId font)
{
    Knob3D *knob = new Knob3D(parent);
    knob->setId(param);
    knob->label = label;
    knob->setRadius(25.f);
    knob->min = min;
    knob->max = max;
    knob->setValue(value);
    knob->gauge_width = 6.0f;
    knob->background_color = Color(190, 190, 190);
    knob->foreground_color = Color(0, 160, 245);
    knob->setCallback(parent);
    knob->font = font;

    return knob;
}

void WAIVESamplerUI::createSampleSlots()
{
    int n = 8;
    float width = slots_container->getWidth();
    for (int i = 0; i < n; i++)
    {
        DropDown *midi_number = new DropDown(this);
        Button *trigger_btn = new Button(this);
        SampleSlot *slot = new SampleSlot(this, midi_number, trigger_btn);
        slot->setSize(width, 300.f / n - 5.f);
        slot->setSamplePlayer(&plugin->samplePlayers.at(i));

        float hue = ((float)8 - i) / (n + 2);
        slot->highlight_color = Color::fromHSL(hue, 0.8f, 0.7f);

        slots_container->addWidget(slot);
        sampleSlots.push_back(slot);

        midi_number->setItem(60 + i, false);
        midi_number->setId(kSlot1MidiNumber + i);
        midi_number->setCallback(this);

        trigger_btn->setCallback(this);

        sampleMidiDropdowns.push_back(midi_number);
        sampleTriggerButtons.push_back(trigger_btn);

        addIdleCallback(slot);
    }
}

END_NAMESPACE_DISTRHO