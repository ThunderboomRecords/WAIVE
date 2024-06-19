#include "WAIVESamplerUI.hpp"
START_NAMESPACE_DISTRHO

WAIVESamplerUI::WAIVESamplerUI() : UI(UI_W, UI_H),
                                   fScaleFactor(getScaleFactor()),
                                   fScale(1.0f),
                                   filebrowserOpen(false),
                                   map_full(false),
                                   simple_controls(true)
{
    plugin = static_cast<WAIVESampler *>(getPluginInstancePointer());

    random.seed();

    waive_link = new Link(this);
    waive_link->url = "https://waive.studio";
    waive_link->setAbsolutePos(UI_W - 240, 4);
    waive_link->setSize(230, 32);

    // register notifications
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskStartedNotification>(*this, &WAIVESamplerUI::onTaskStarted));
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFinishedNotification>(*this, &WAIVESamplerUI::onTaskFinished));
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskProgressNotification>(*this, &WAIVESamplerUI::onTaskProgress));
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskCancelledNotification>(*this, &WAIVESamplerUI::onTaskCancelled));
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFailedNotification>(*this, &WAIVESamplerUI::onTaskFailed));

    plugin->sd.databaseUpdate += Poco::delegate(this, &WAIVESamplerUI::onDatabaseChanged);
    plugin->pluginUpdate += Poco::delegate(this, &WAIVESamplerUI::onPluginUpdated);

    logo_font = createFontFromMemory("VG5000", VG5000, VG5000_len, false);

    // ------ Sample Map

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
    expand_map_btn->background_color = Color(40, 40, 40);
    expand_map_btn->text_color = Color(100, 100, 100);
    expand_map_btn->setCallback(this);
    expand_map_btn->onTop(sample_map, Widget_Align::END, Widget_Align::END, 5);

    map_label = new Label(this, "sample map");
    map_label->setFont("VG5000", VG5000, VG5000_len);
    map_label->font_size = 16.0f;
    map_label->resizeToFit();
    map_label->above(sample_map, Widget_Align::START, 5);

    import_sample_btn = new Button(this);
    import_sample_btn->setLabel("import sample");
    import_sample_btn->setSize(100, 20);
    import_sample_btn->rightOf(map_label, Widget_Align::CENTER, 20);
    import_sample_btn->setCallback(this);

    import_spinner = new Spinner(this);
    import_spinner->setSize(20, 20);
    import_spinner->rightOf(import_sample_btn, Widget_Align::CENTER, 5);

    sample_map_menu = new Menu(this);
    for (int i = 1; i < 9; i++)
        sample_map_menu->addItem(fmt::format("Add to slot {:d}", i));

    sample_map_menu->setWidth(100);
    sample_map_menu->setFont("VG5000", VG5000, VG5000_len);
    sample_map_menu->setDisplayNumber(8);
    sample_map_menu->hide();
    sample_map_menu->setCallback(sample_map);

    sample_map->menu = sample_map_menu;

    // ------ Sample Player

    slots_container = new VBox(this);
    slots_container->setSize(UI_W - sample_map->getWidth() - 30, sample_map->getHeight());
    slots_container->padding = 2;
    slots_container->justify_content = VBox::Justify_Content::space_evenly;
    slots_container->rightOf(sample_map, Widget_Align::START, 10);
    createSampleSlots();
    slots_container->positionWidgets();

    // ------ Sample Creator

    // Advanced
    sample_controls_label = new Label(this, "sample creator");
    sample_controls_label->setFont("VG5000", VG5000, VG5000_len);
    sample_controls_label->font_size = 16.0f;
    sample_controls_label->resizeToFit();
    sample_controls_label->below(sample_map, Widget_Align::START, 5);

    new_sample_btn = new Button(this);
    new_sample_btn->setLabel("new");
    new_sample_btn->setSize(70, 20);
    new_sample_btn->rightOf(sample_controls_label, Widget_Align::CENTER, 20);
    new_sample_btn->setCallback(this);

    sample_name = new TextInput(this);
    sample_name->setSize(200, 20);
    sample_name->rightOf(new_sample_btn, Widget_Align::CENTER, 20);
    sample_name->setCallback(this);

    source_display = new Waveform(this);
    source_display->setSize(UI_W - 20, 80);
    source_display->below(sample_controls_label, Widget_Align::START, 5);
    source_display->selectable = true;
    source_display->setCallback(this);
    source_display->lineColor = Color(255, 255, 255);
    source_display->setWaveform(&plugin->fSourceWaveform);
    source_display->setWaveformFeatures(&plugin->fSourceFeatures);

    open_source_btn = new Button(this);
    open_source_btn->setLabel("import source");
    open_source_btn->background_color = Color(40, 40, 40);
    open_source_btn->text_color = Color(100, 100, 100);
    open_source_btn->setSize(100, 20);
    open_source_btn->onTop(source_display, Widget_Align::END, Widget_Align::START, 2);
    open_source_btn->setCallback(this);

    browser_sources_btn = new Button(this);
    browser_sources_btn->setLabel("browse...");
    browser_sources_btn->background_color = Color(40, 40, 40);
    browser_sources_btn->text_color = Color(100, 100, 100);
    browser_sources_btn->setSize(100, 20);
    browser_sources_btn->leftOf(open_source_btn, Widget_Align::CENTER, 5);
    browser_sources_btn->setCallback(this);

    controls_toggle = new Button(this);
    controls_toggle->setCallback(this);
    controls_toggle->setLabel("advanced");
    controls_toggle->setSize(100, 20);
    controls_toggle->below(source_display, Widget_Align::START, 10);

    sample_display = new Waveform(this);
    sample_display->setSize(180, 80);
    sample_display->below(source_display, Widget_Align::END, 10);
    sample_display->setWaveform(plugin->editorPreviewWaveform);

    save_sample_btn = new Button(this);
    save_sample_btn->setLabel("add");
    save_sample_btn->setSize(70, 20);
    save_sample_btn->below(sample_display, Widget_Align::END, 10);
    save_sample_btn->setCallback(this);
    save_sample_btn->setEnabled(false);

    play_btn = new Button(this);
    play_btn->setLabel("preview");
    play_btn->setSize(70, 20);
    play_btn->below(sample_display, Widget_Align::START, 10);
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

    pitch = createWAIVEKnob(this, kSamplePitch, "pitch", 0.25f, 4.f, 1.0f, logo_font);
    volume = createWAIVEKnob(this, kSampleVolume, "volume", 0.0f, 2.0f, 1.0f, logo_font);
    percussionBoost = createWAIVEKnob(this, kPercussiveBoost, "perc.", 0.0f, 1.0f, 1.0f, logo_font);

    ampAttack = createWAIVEKnob(this, kAmpAttack, "attack", 0.0f, 500.0f, 10.0f, logo_font);
    ampAttack->format = "{:.0f}ms";

    ampDecay = createWAIVEKnob(this, kAmpDecay, "decay", 0.0f, 500.0f, 50.0f, logo_font);
    ampDecay->format = "{:.0f}ms";

    ampSustain = createWAIVEKnob(this, kAmpSustain, "sustain", 0.0f, 1.0f, 0.7f, logo_font);

    ampRelease = createWAIVEKnob(this, kAmpRelease, "release", 0.0f, 500.0f, 100.0f, logo_font);
    ampRelease->format = "{:.0f}ms";

    sustainLength = createWAIVEKnob(this, kSustainLength, "length", 0.0f, 1000.0f, 100.f, logo_font);
    sustainLength->format = "{:.0f}ms";

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
    shapeKnobs->leftOf(sample_display, Widget_Align::CENTER, 10);
    shapeKnobs->positionWidgets();

    ampADSRKnobs->addWidget(ampAttack);
    ampADSRKnobs->addWidget(ampDecay);
    ampADSRKnobs->addWidget(ampSustain);
    ampADSRKnobs->addWidget(ampRelease);
    ampADSRKnobs->addWidget(sustainLength);
    ampADSRKnobs->resizeToFit();
    ampADSRKnobs->leftOf(shapeKnobs, Widget_Align::CENTER, 10);
    ampADSRKnobs->positionWidgets();

    filterKnobs->addWidget(filterType);
    filterKnobs->addWidget(filterCutoff);
    filterKnobs->addWidget(filterResonance);
    filterKnobs->resizeToFit();
    filterKnobs->leftOf(ampADSRKnobs, Widget_Align::CENTER, 10);
    filterKnobs->positionWidgets();

    sample_editor_controls_advanced = new WidgetGroup(this);
    sample_editor_controls_advanced->addChildWidget(filterKnobs);
    sample_editor_controls_advanced->addChildWidget(ampADSRKnobs);
    sample_editor_controls_advanced->addChildWidget(shapeKnobs);
    sample_editor_controls_advanced->computeSize();
    sample_editor_controls_advanced->render = false;

    // Simple controls
    make_kick = new Button(this);
    make_kick->setLabel("kick");
    make_kick->setSize(100, 20);
    make_kick->setCallback(this);

    make_snare = new Button(this);
    make_snare->setLabel("snare");
    make_snare->setSize(100, 20);
    make_snare->setCallback(this);

    make_crash = new Button(this);
    make_crash->setLabel("crash");
    make_crash->setSize(100, 20);
    make_crash->setCallback(this);

    simple_buttons = new HBox(this);
    simple_buttons->padding = 10;
    simple_buttons->addWidget(make_kick);
    simple_buttons->addWidget(make_snare);
    simple_buttons->addWidget(make_crash);
    simple_buttons->resizeToFit();

    instructions = new Label(this, "First load a Source above, then click to create different drum hits");
    instructions->resizeToFit();
    sample_editor_controls_simple = new WidgetGroup(this);
    sample_editor_controls_simple->render = false;
    sample_editor_controls_simple->addChildWidget(simple_buttons,
                                                  {simple_buttons,
                                                   sample_editor_controls_advanced,
                                                   Position::ON_TOP,
                                                   Widget_Align::CENTER,
                                                   Widget_Align::END, 0});
    // sample_editor_controls_simple->addChildWidget(instructions, {instructions, simple_buttons, Position::ON_TOP, Widget_Align::CENTER, Widget_Align::CENTER, 5});
    sample_editor_controls_simple->repositionWidgets();
    instructions->below(simple_buttons, Widget_Align::CENTER, 5);

    simple_buttons->positionWidgets();

    // ----------- Floating components
    dropdown_menu = new Menu(this);
    dropdown_menu->hide();
    dropdown_menu->setFont("VG5000", VG5000, VG5000_len);

    for (int i = 0; i < sampleSlots.size(); i++)
    {
        sampleMidiDropdowns[i]->menu = dropdown_menu;
        sampleSlots[i]->repositionWidgets();
    }

    filterType->menu = dropdown_menu;

    value_indicator = new ValueIndicator(this);
    value_indicator->setSize(70, 20);
    value_indicator->fontId = logo_font;
    value_indicator->hide();

    setSampleEditorVisible(false);

    source_browser_root = new SourceBrowserRoot(getApp(), UI_W, UI_H - 40);
    source_browser_root->setTitle("Browse archives...");
    source_browser_root->setCallback(this);

    source_browser = new SourceBrowser(*source_browser_root, &plugin->sd);
    source_browser->setCallback(this);

    setGeometryConstraints(UI_W * fScaleFactor, UI_H * fScaleFactor, false, false);

    if (fScaleFactor != 1.0)
        setSize(UI_W * fScaleFactor, UI_H * fScaleFactor);

    std::cout << "WAIVESamplerUI initialised" << std::endl;
}

WAIVESamplerUI::~WAIVESamplerUI()
{
    plugin->sd.databaseUpdate -= Poco::delegate(this, &WAIVESamplerUI::onDatabaseChanged);
    source_browser_root->getWindow().close();

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
    {
        source_browser_root->show();
        source_browser->updateSourceDatabase();
    }
    else if (button == expand_map_btn)
    {
        map_full = !map_full;
        if (!map_full)
        {
            sample_map->toFront();
            sample_map->setSize(520, 300);

            expand_map_btn->setLabel("↘");
            expand_map_btn->toFront();
        }
        else
        {
            sample_map->toFront();
            sample_map->setSize(UI_W - 20, UI_H - 50);

            expand_map_btn->setLabel("↖");
            expand_map_btn->toFront();
        }
        expand_map_btn->onTop(sample_map, Widget_Align::END, Widget_Align::END, 5);
    }
    else if (button == controls_toggle)
    {
        simple_controls = !simple_controls;
        if (simple_controls)
        {
            sample_editor_controls_advanced->setVisible(false);
            sample_editor_controls_simple->setVisible(true);
            controls_toggle->setLabel("advanced");
        }
        else
        {
            sample_editor_controls_advanced->setVisible(true);
            sample_editor_controls_simple->setVisible(false);
            controls_toggle->setLabel("simple");
        }
    }
    else if (button == make_kick)
    {
        std::cout << "Make kick...\n";
        // 0. Check if a source is loaded
        if (!plugin->fSourceLoaded)
            return;

        // 0.5 Create new sample?
        plugin->newSample();

        // 1. Select random area of source
        int source_length = plugin->fSourceLength;
        int startIndex = 0.9f * random.nextFloat() * source_length;
        plugin->selectWaveform(&plugin->fSourceWaveform, startIndex);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::KickPreset);

        // 3. Set sample name
        std::string sampleName = plugin->sd.getNewSampleName("kick.wav");
        sample_name->setText(sampleName.c_str(), true);

        // 4. Add to library
        plugin->addCurrentSampleToLibrary();
    }
    else if (button == make_snare)
    {
        std::cout << "Make snare...\n";
    }
    else if (button == make_crash)
    {
        std::cout << "Make crash...\n";
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
        if (text.length() == 0)
        {
            textInput->undo();
            return;
        }

        if (plugin->fCurrentSample != nullptr)
            plugin->sd.renameSample(plugin->fCurrentSample, text);
    }
}

void WAIVESamplerUI::textInputChanged(TextInput *textInput, std::string text) {}

void WAIVESamplerUI::dropdownSelection(DropDown *widget, int item)
{
    if (widget == filterType)
    {
        setParameterValue(widget->getId(), item);
    }
}

void WAIVESamplerUI::sampleSelected(SampleSlot *slot)
{
    for (int i = 0; i < sampleSlots.size(); i++)
    {
        if (sampleSlots[i] != slot)
            continue;

        plugin->loadSample(sampleSlots[i]->getSamplePlayer()->sampleInfo);
        return;
    }
};

void WAIVESamplerUI::sampleSlotCleared(SampleSlot *slot)
{
    for (int i = 0; i < sampleSlots.size(); i++)
    {
        if (sampleSlots[i] != slot)
            continue;

        plugin->clearSamplePlayer(*sampleSlots[i]->getSamplePlayer());
        return;
    }
};

void WAIVESamplerUI::sourceBrowserClosed()
{
    plugin->stopSourcePreview();
}

void WAIVESamplerUI::browserStopPreview()
{
    plugin->stopSourcePreview();
}

void WAIVESamplerUI::browserLoadSource(const std::string &fp)
{
    plugin->loadSource(fp.c_str());
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

void WAIVESamplerUI::onPluginUpdated(const void *pSender, const WAIVESampler::PluginUpdate &arg)
{
    switch (arg)
    {
    case WAIVESampler::kSourceLoading:
        break;
    case WAIVESampler::kSourceLoaded:
        source_display->setWaveformLength(plugin->fSourceLength);
        source_display->waveformNew();
        save_sample_btn->setEnabled(true);
        break;
    case WAIVESampler::kSourceUpdated:
        source_display->setWaveformLength(plugin->fSourceLength);
        source_display->waveformUpdated();
        break;
    case WAIVESampler::kSampleLoading:
        break;
    case WAIVESampler::kSampleLoaded:
        sample_display->setWaveformLength(plugin->fCurrentSample->sampleLength);
        if (plugin->fCurrentSample->saved)
            save_sample_btn->setLabel("update");
        else
            save_sample_btn->setLabel("add");
        sample_display->waveformNew();
        play_btn->setEnabled(true);
        break;
    case WAIVESampler::kSampleUpdated:
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
    case WAIVESampler::kSampleAdded:
        sample_map->repaint();
        break;
    case WAIVESampler::kParametersChanged:
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
    case WAIVESampler::kSlotLoaded:
        for (int i = 0; i < sampleSlots.size(); i++)
        {
            sampleSlots[i]->repaint();
        }
        break;
    default:
        std::cout << "Unknown update: " << arg << std::endl;
        break;
    }
}

void WAIVESamplerUI::onTaskStarted(Poco::TaskStartedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    // std::cout << "WAIVESamplerUI::onTaskStarted: " << pTask->name() << std::endl;
    if (pTask->name().compare("ImporterTask") == 0)
    {
        import_spinner->setLoading(true);
    }
    pTask->release();
}

void WAIVESamplerUI::onTaskProgress(Poco::TaskProgressNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    // std::cout << "WAIVESamplerUI::onTaskProgress: " << pTask->name() << " " << pTask->progress() << std::endl;
    if (pTask->name().compare("FeatureExtractorTask") == 0)
        source_display->repaint();
    pTask->release();
}

void WAIVESamplerUI::onTaskCancelled(Poco::TaskCancelledNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    // std::cout << "WAIVESamplerUI::onTaskCancelled: " << pTask->name() << std::endl;
    pTask->release();
}

void WAIVESamplerUI::onTaskFailed(Poco::TaskFailedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    // std::cout << "WAIVESamplerUI::onTaskFailed: " << pTask->name() << std::endl;
    pTask->release();
}

void WAIVESamplerUI::onTaskFinished(Poco::TaskFinishedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    // std::cout << "WAIVESamplerUI::onTaskFinished: " << pTask->name() << std::endl;
    if (pTask->name().compare("ImporterTask") == 0)
    {
        import_spinner->setLoading(false);
    }
    pTask->release();
}

void WAIVESamplerUI::onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg)
{
    // std::cout << "WAIVESamplerUI::onDatabaseChanged " << arg << std::endl;
}

void WAIVESamplerUI::setSampleEditorVisible(bool visible)
{
    if (simple_controls)
    {
        sample_editor_controls_advanced->setVisible(false);
        sample_editor_controls_simple->setVisible(visible);
    }
    else
    {
        sample_editor_controls_advanced->setVisible(visible);
        sample_editor_controls_simple->setVisible(false);
        // ampADSRKnobs->setVisible(visible);
        // filterKnobs->setVisible(visible);
        // shapeKnobs->setVisible(visible);
    }
    source_display->setVisible(visible);
    controls_toggle->setVisible(visible);
    sample_display->setVisible(visible);
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

        trigger_btn->fontSize(16.0f);
        trigger_btn->setLabel("▶");
        trigger_btn->setSize(20, 20);

        for (int i = 1; i < 128; i++)
            midi_number->addItem(fmt::format("{:d}", i).c_str());

        midi_number->setDisplayNumber(16);
        midi_number->font_size = 16.0f;
        midi_number->setSize(45, 20);

        SampleSlot *slot = new SampleSlot(this);
        slot->setCallback(this);
        slot->addChildWidget(trigger_btn, {trigger_btn, slot, Position::ON_TOP, Widget_Align::START, Widget_Align::CENTER, 5});
        slot->addChildWidget(midi_number, {midi_number, slot, Position::ON_TOP, Widget_Align::END, Widget_Align::CENTER, 5});

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