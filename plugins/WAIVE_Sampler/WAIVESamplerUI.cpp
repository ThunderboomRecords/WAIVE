#include "WAIVESamplerUI.hpp"
START_NAMESPACE_DISTRHO

WAIVESamplerUI::WAIVESamplerUI() : UI(UI_W, UI_H),
                                   fScaleFactor(getScaleFactor()),
                                   fScale(1.0f),
                                   filebrowserOpen(false)
{
    plugin = static_cast<WAIVESampler *>(getPluginInstancePointer());

    random.seed();

    float width = UI_W * fScaleFactor;
    float height = UI_H * fScaleFactor;
    float padding = 4.f * fScaleFactor;

    fontTitle = createFontFromMemory("VG5000", VG5000, VG5000_len, false);
    fontMain = createFontFromMemory("Poppins-Light", Poppins_Light, Poppins_Light_len, false);

    float panelWidths = width - 3.f * padding;
    float panelHeights = height - 3.f * padding;

    float col1Width = panelWidths * 2.f / 3.f;
    float col2Width = panelWidths - col1Width;

    // 1 ----- Source Browser Panel
    sourceBrowserPanel = new Panel(this);
    sourceBrowserPanel->setSize(col1Width, panelHeights * 0.5f, true);
    sourceBrowserPanel->setAbsolutePos(padding, padding);
    sourceBrowserPanel->setFont("VG5000", VG5000, VG5000_len);
    sourceBrowserPanel->label = "1";
    sourceBrowserPanel->title = "Source";

    sourceList = new SourceList(this);
    sourceList->setSize(sourceBrowserPanel->getWidth() - 4.f * padding, sourceBrowserPanel->getHeight() * 0.6f, true);
    sourceList->onTop(sourceBrowserPanel, CENTER, START, sourceBrowserPanel->getFontSize() * 2.f);
    sourceList->source_info = &(plugin->sd.sourcesList);
    sourceList->source_info_mtx = &(plugin->sd.sourceListMutex);
    sourceList->scrollHandle = WaiveColors::light2;
    sourceList->scrollGutter = WaiveColors::light1;
    sourceList->background_color = WaiveColors::grey2;
    sourceList->accent_color = sourceBrowserPanel->background_color;
    sourceList->padding = 0.0f;
    sourceList->margin = 0.0f;
    sourceList->setFontSize(16.f);
    sourceList->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    sourceList->setCallback(this);

    filterSources = new Button(this);
    filterSources->setLabel("Tags");
    filterSources->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    filterSources->resizeToFit();
    filterSources->onTop(sourceBrowserPanel, START, END, padding * 2.f);
    filterSources->setCallback(this);

    searchBox = new Panel(this);
    searchBox->setSize(300, filterSources->getHeight(), true);
    searchBox->radius = searchBox->getHeight() / 2;
    searchBox->background_color = WaiveColors::grey2;
    searchBox->rightOf(filterSources);

    sourceSearch = new TextInput(this);
    sourceSearch->placeholder = "Search...";
    sourceSearch->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    sourceSearch->foreground_color = WaiveColors::light1;
    sourceSearch->background_color = searchBox->background_color;
    sourceSearch->setSize(searchBox->getWidth() - padding * 4.f - sourceSearch->getFontSize(), sourceSearch->getFontSize(), true);
    sourceSearch->onTop(searchBox, START, CENTER, padding * 2.f);
    sourceSearch->setCallback(this);

    searchIcon = new Icon(this);
    searchIcon->setImageData(search, search_len, 85, 85, IMAGE_GENERATE_MIPMAPS);
    searchIcon->setSize(sourceSearch->getHeight() - padding, sourceSearch->getHeight() - padding, true);
    searchIcon->onTop(searchBox, END, CENTER, padding * 2.f);

    sourceList->setHeight(Layout::measureVertical(sourceList, START, searchBox, START) - 2.f * padding);

    databaseLoading = new Spinner(this);
    databaseLoading->setSize(sourceSearch->getHeight(), sourceSearch->getHeight(), true);
    databaseLoading->rightOf(searchBox, CENTER, padding * 2.f);

    databaseProgress = new Label(this, "");
    databaseProgress->rightOf(databaseLoading, START);
    databaseProgress->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);

    previewPlayback = new Button(this);
    previewPlayback->setLabel("Stop");
    previewPlayback->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    previewPlayback->resizeToFit();
    previewPlayback->onTop(sourceBrowserPanel, END, END, padding * 2.f);
    previewPlayback->setCallback(this);
    previewPlayback->setVisible(false);

    sourceBrowserPanel->addChildWidget(sourceList);
    sourceBrowserPanel->addChildWidget(filterSources);
    sourceBrowserPanel->addChildWidget(searchBox);
    sourceBrowserPanel->addChildWidget(sourceSearch);

    // 2 ----- Sample Editor Panel
    sampleEditorPanel = new Panel(this);
    sampleEditorPanel->setSize(col1Width, panelHeights * 0.5f, true);
    sampleEditorPanel->below(sourceBrowserPanel, START, padding);
    sampleEditorPanel->setFont("VG5000", VG5000, VG5000_len);
    sampleEditorPanel->label = "2";
    sampleEditorPanel->title = "Sample Editor";

    importSource = new Button(this);
    importSource->setLabel("Import source");
    importSource->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    importSource->resizeToFit();
    importSource->onTop(sampleEditorPanel, END, START, padding);
    importSource->setCallback(this);

    sourceWaveformDisplay = new Waveform(this);
    sourceWaveformDisplay->setSize(sampleEditorPanel->getWidth() - 4.f * padding, sampleEditorPanel->getHeight() * 0.4f, true);
    sourceWaveformDisplay->below(importSource, END, padding);
    sourceWaveformDisplay->selectable = true;
    sourceWaveformDisplay->setCallback(this);
    sourceWaveformDisplay->setWaveform(&plugin->fSourceWaveform);
    sourceWaveformDisplay->setWaveformFeatures(&plugin->fSourceFeatures);
    sourceWaveformDisplay->background_color = WaiveColors::dark;
    sampleEditorPanel->addChildWidget(sourceWaveformDisplay);

    sourcePreviewBtn = new Button(this);
    sourcePreviewBtn->setLabel("▶");
    sourcePreviewBtn->drawBackground = false;
    sourcePreviewBtn->resizeToFit();
    sourcePreviewBtn->onTop(sourceWaveformDisplay, END, START, padding);
    sourcePreviewBtn->setCallback(this);

    sourceLoading = new Spinner(this);
    sourceLoading->setSize(16, 16);
    sourceLoading->onTop(sourceWaveformDisplay, START, START, padding * 2);
    sourceLoading->setLoading(false);

    progress = new Label(this, "");
    progress->setFontSize(12.f);
    progress->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    progress->resizeToFit();
    progress->rightOf(sourceLoading, START, padding);

    presetLabel = new Label(this, "Detect:");
    presetLabel->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    presetLabel->resizeToFit();

    knobsLabel = new Label(this, "Adjust");
    knobsLabel->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    knobsLabel->resizeToFit();

    makeKick = new Button(this);
    makeKick->setLabel("Kick");
    makeKick->setFontSize(16.f);
    makeKick->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    makeKick->setSize(100.f, 32.f);
    makeKick->setCallback(this);

    makeSnare = new Button(this);
    makeSnare->setLabel("Snare");
    makeSnare->setFontSize(16.f);
    makeSnare->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    makeSnare->setSize(100.f, 32.f);
    makeSnare->setCallback(this);

    makeHihat = new Button(this);
    makeHihat->setLabel("Hi-Hat");
    makeHihat->setFontSize(16.f);
    makeHihat->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    makeHihat->setSize(100.f, 32.f);
    makeHihat->setCallback(this);

    makeClap = new Button(this);
    makeClap->setLabel("Clap");
    makeClap->setFontSize(16.f);
    makeClap->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    makeClap->setSize(100.f, 32.f);
    makeClap->setCallback(this);

    presetButtons = new HBox(this);
    presetButtons->addWidget(presetLabel);
    presetButtons->setWidgetJustify_Content(0, HBox::Justify_Content::left);
    presetButtons->addWidget(makeKick);
    presetButtons->addWidget(makeSnare);
    presetButtons->addWidget(makeHihat);
    presetButtons->addWidget(makeClap);
    presetButtons->below(sourceWaveformDisplay, START, 2.f * padding);
    presetButtons->setWidth(sourceWaveformDisplay->getWidth());
    presetButtons->setHeight(makeKick->getHeight());
    presetButtons->justify_content = HBox::Justify_Content::space_evenly;
    presetButtons->positionWidgets();

    presetButtons->setVisible(false);

    pitch = createWAIVEKnob(kSamplePitch, "pitch", 0.25f, 4.f, 1.0f);
    volume = createWAIVEKnob(kSampleVolume, "volume", 0.0f, 2.0f, 1.0f);
    percussionBoost = createWAIVEKnob(kPercussiveBoost, "perc.", 0.0f, 2.0f, 0.5f);

    ampAttack = createWAIVEKnob(kAmpAttack, "attack", 0.0f, 500.0f, 10.0f);
    ampAttack->format = "{:.0f}ms";
    ampDecay = createWAIVEKnob(kAmpDecay, "decay", 0.0f, 500.0f, 50.0f);
    ampDecay->format = "{:.0f}ms";
    ampSustain = createWAIVEKnob(kAmpSustain, "sustain", 0.0f, 1.0f, 0.7f);
    ampRelease = createWAIVEKnob(kAmpRelease, "release", 0.0f, 500.0f, 100.0f);
    ampRelease->format = "{:.0f}ms";
    sustainLength = createWAIVEKnob(kSustainLength, "length", 0.0f, 1000.0f, 100.f);
    sustainLength->format = "{:.0f}ms";

    filterCutoff = createWAIVEKnob(kFilterCutoff, "cutoff", 0.0, 0.999, 0.999);
    filterResonance = createWAIVEKnob(kFilterResonance, "res.", 0.0, 1.0, 0.0);

    filterType = new DropDown(this);
    filterType->setFontSize(16.0f);
    filterType->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    filterType->menu->setFontSize(16.0f);
    filterType->menu->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    filterType->addItem("LP");
    filterType->addItem("HP");
    filterType->addItem("BP");
    filterType->setId(kFilterType);
    filterType->setDisplayNumber(3);
    filterType->setSize(35, 20);
    filterType->setItem(0, false);
    filterType->setCallback(this);

    editorKnobs = new HBox(this);
    editorKnobs->addWidget(knobsLabel);
    editorKnobs->addWidget(pitch);
    editorKnobs->addWidget(volume);
    editorKnobs->addWidget(percussionBoost);
    editorKnobs->addWidget(ampAttack);
    editorKnobs->addWidget(ampDecay);
    editorKnobs->addWidget(ampSustain);
    editorKnobs->addWidget(ampRelease);
    editorKnobs->addWidget(sustainLength);
    editorKnobs->addWidget(filterCutoff);
    editorKnobs->addWidget(filterResonance);
    editorKnobs->addWidget(filterType);
    editorKnobs->below(presetButtons, START, padding);
    editorKnobs->setWidth(presetButtons->getWidth());
    editorKnobs->justify_content = HBox::Justify_Content::space_between;
    editorKnobs->positionWidgets();

    editorKnobs->setVisible(false);

    instructions = new Label(this, "Load a source or import your own audio to extract samples from.");
    instructions->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    instructions->resizeToFit();
    instructions->below(sourceWaveformDisplay, CENTER, Layout::measureVertical(sourceWaveformDisplay, END, sampleEditorPanel, END) / 2.f);

    valueIndicator = new ValueIndicator(this);
    valueIndicator->setSize(70, 20);
    valueIndicator->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    valueIndicator->hide();
    valueIndicator->background_color = WaiveColors::grey2;

    // 3 ----- Sample Viewer Panel
    samplePanel = new Panel(this);
    samplePanel->setSize(col2Width, panelHeights / 3.f, true);
    samplePanel->rightOf(sourceBrowserPanel, START, padding);
    samplePanel->setFont("VG5000", VG5000, VG5000_len);
    samplePanel->label = "3";
    samplePanel->title = "Sample";

    sampleWaveformDisplay = new Waveform(this);
    sampleWaveformDisplay->setSize(samplePanel->getWidth() - 4.f * padding, samplePanel->getHeight() * 0.4f, true);
    sampleWaveformDisplay->onTop(samplePanel, CENTER, START, samplePanel->getFontSize() * 2.f);
    sampleWaveformDisplay->setWaveform(plugin->editorPreviewWaveform);
    sampleWaveformDisplay->background_color = WaiveColors::accent1;
    samplePanel->addChildWidget(sampleWaveformDisplay);

    playSampleBtn = new Button(this);
    playSampleBtn->setLabel("▶");
    playSampleBtn->drawBackground = false;
    playSampleBtn->resizeToFit();
    playSampleBtn->onTop(sampleWaveformDisplay, END, START, padding);
    playSampleBtn->setCallback(this);

    sampleName = new TextInput(this);
    sampleName->setSize(samplePanel->getWidth() * 0.8f, 20, true);
    sampleName->below(sampleWaveformDisplay, CENTER, padding);
    sampleName->setCallback(this);
    sampleName->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    sampleName->align = Align::ALIGN_CENTER;
    sampleName->foreground_color = WaiveColors::light1;

    saveSampleBtn = new Button(this);
    saveSampleBtn->setLabel("Add to pack");
    saveSampleBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    saveSampleBtn->resizeToFit();
    saveSampleBtn->setCallback(this);
    saveSampleBtn->setEnabled(false);

    newSampleBtn = new Button(this);
    newSampleBtn->setLabel("New sample");
    newSampleBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    newSampleBtn->resizeToFit();
    newSampleBtn->setCallback(this);

    HBox alignSampleButtons(this);
    alignSampleButtons.addWidget(newSampleBtn);
    alignSampleButtons.addWidget(saveSampleBtn);
    alignSampleButtons.justify_content = HBox::Justify_Content::center;
    alignSampleButtons.padding = 2.f * padding;
    alignSampleButtons.resizeToFit();
    alignSampleButtons.setWidth(samplePanel->getWidth() - 4.f * padding);
    alignSampleButtons.onTop(samplePanel, CENTER, END, padding * 2.f);
    alignSampleButtons.positionWidgets();

    // 4 ----- Sample Player Panel
    samplePlayerPanel = new Panel(this);
    samplePlayerPanel->setSize(col2Width, panelHeights * 2.f / 3.f, true);
    samplePlayerPanel->rightOf(sampleEditorPanel, END, padding);
    samplePlayerPanel->setFont("VG5000", VG5000, VG5000_len);
    samplePlayerPanel->label = "4";
    samplePlayerPanel->title = "Sample Player";

    openMapBtn = new Button(this);
    openMapBtn->setLabel("Sample Map");
    openMapBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    openMapBtn->resizeToFit();
    openMapBtn->setCallback(this);

    browseFilesBtn = new Button(this);
    browseFilesBtn->setLabel("Open Folder");
    browseFilesBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    browseFilesBtn->resizeToFit();
    browseFilesBtn->setCallback(this);

    HBox alignPlayerButtons(this);
    alignPlayerButtons.addWidget(openMapBtn);
    alignPlayerButtons.addWidget(browseFilesBtn);
    alignPlayerButtons.justify_content = HBox::Justify_Content::center;
    alignPlayerButtons.padding = 2.f * padding;
    alignPlayerButtons.resizeToFit();
    alignPlayerButtons.setWidth(samplePlayerPanel->getWidth() - 4.f * padding);
    alignPlayerButtons.onTop(samplePlayerPanel, CENTER, END, padding * 2.f);
    alignPlayerButtons.positionWidgets();

    sampleSlotsContainer = new VBox(this);
    sampleSlotsContainer->justify_content = VBox::Justify_Content::space_evenly;
    sampleSlotsContainer->setWidth(samplePlayerPanel->getWidth() - 4.f * padding);
    sampleSlotsContainer->onTop(samplePlayerPanel, CENTER, START, samplePlayerPanel->getFontSize() * 2.f);
    sampleSlotsContainer->setHeight(Layout::measureVertical(sampleSlotsContainer, START, openMapBtn, START) - padding);
    samplePlayerPanel->addChildWidget(sampleSlotsContainer);

    for (int i = 0; i < 8; i++)
    {
        SampleSlot *slot = new SampleSlot(this);
        slot->setSamplePlayer(&plugin->samplePlayers[i]);
        slot->setCallback(this);
        slot->slotId = i;
        slot->setSize(sampleSlotsContainer->getWidth(), 35, true);
        slot->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);

        sampleSlotsContainer->addWidget(slot);
        sampleSlots.push_back(slot);
    }

    sampleSlotsContainer->positionWidgets();
    for (int i = 0; i < sampleSlots.size(); i++)
        sampleSlots[i]->repositionWidgets();

    // 5 ----- Sample Map
    sampleBrowserRoot = new SampleBrowserRoot(getApp(), UI_W, UI_H - 40);
    sampleBrowserRoot->setTitle("Browse samples");

    sampleBrowser = new SampleBrowser(*sampleBrowserRoot, &plugin->sd);
    sampleBrowser->setCallback(this);

    // 6 ----- Sample Map
    tagRoot = new SampleBrowserRoot(getApp(), 300 * fScaleFactor, 300 * fScaleFactor);
    tagRoot->setTitle("Browse tags");

    tagBrowser = new TagBrowser(*tagRoot, &plugin->sd);

    setGeometryConstraints(width, height, false, false);

    if (fScaleFactor != 1.0)
        setSize(width, height);

    // register notifications
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskStartedNotification>(*this, &WAIVESamplerUI::onTaskStarted));
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFinishedNotification>(*this, &WAIVESamplerUI::onTaskFinished));
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskProgressNotification>(*this, &WAIVESamplerUI::onTaskProgress));
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskCancelledNotification>(*this, &WAIVESamplerUI::onTaskCancelled));
    plugin->taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFailedNotification>(*this, &WAIVESamplerUI::onTaskFailed));

    plugin->sd.taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskStartedNotification>(*this, &WAIVESamplerUI::onTaskStarted));
    plugin->sd.taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFinishedNotification>(*this, &WAIVESamplerUI::onTaskFinished));
    plugin->sd.taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskProgressNotification>(*this, &WAIVESamplerUI::onTaskProgress));
    plugin->sd.taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskCancelledNotification>(*this, &WAIVESamplerUI::onTaskCancelled));
    plugin->sd.taskManager.addObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFailedNotification>(*this, &WAIVESamplerUI::onTaskFailed));

    plugin->sd.databaseUpdate += Poco::delegate(this, &WAIVESamplerUI::onDatabaseChanged);
    plugin->pluginUpdate += Poco::delegate(this, &WAIVESamplerUI::onPluginUpdated);

    printf("WAIVESamplerUI initialised: (%.0f, %.0f)\n", width, height);

    // should be done by plugin itself?
    plugin->sd.checkLatestRemoteVersion();
}

WAIVESamplerUI::~WAIVESamplerUI()
{
    plugin->sd.databaseUpdate -= Poco::delegate(this, &WAIVESamplerUI::onDatabaseChanged);
    sampleBrowserRoot->close();
    tagRoot->close();

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
        sampleSlots[slot]->setMidiNumber(value, false);
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
    valueIndicator->setAbsoluteX(knob->getAbsoluteX());
    valueIndicator->setWidth(knob->getWidth());
    valueIndicator->setAbsoluteY(knob->getAbsoluteY() + knob->getHeight());
    valueIndicator->setFormatString(knob->getFormat());
    valueIndicator->setValue(knob->getValue());
    valueIndicator->toFront();
    valueIndicator->show();
}

void WAIVESamplerUI::knobDragFinished(Knob *knob, float value)
{
    valueIndicator->hide();
    repaint();

    plugin->triggerPreview();
}

void WAIVESamplerUI::knobValueChanged(Knob *knob, float value)
{
    setParameterValue(knob->getId(), value);
    valueIndicator->setValue(knob->getValue());
}

void WAIVESamplerUI::buttonClicked(Button *button)
{
    if (button == importSource)
        beginOpenFileBrowser("filename", false);
    else if (button == saveSampleBtn)
        plugin->addCurrentSampleToLibrary();
    else if (button == playSampleBtn)
        plugin->triggerPreview();
    else if (button == filterSources)
        tagRoot->show();
    else if (button == previewPlayback)
    {
        plugin->stopSourcePreview();
        previewPlayback->setVisible(false);
    }
    else if (button == sourcePreviewBtn)
        plugin->playSourcePreview();
    else if (button == newSampleBtn)
        plugin->newSample();
    else if (button == makeKick)
    {
        // 0. Check if a source is loaded
        if (!plugin->fSourceLoaded)
            return;

        // 1. Select random candidate area of source
        int nCandidates = plugin->fSourceFeatures.size();
        if (nCandidates == 0)
        {

            return; // or pick a random spot?
        }
        else
        {
            int i = random.next() % nCandidates;
            int startIndex = plugin->fSourceFeatures[i].start;
            plugin->selectWaveform(&plugin->fSourceWaveform, startIndex);
        }

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::KickPreset);

        // 3. Set sample name
        sampleName->setText(plugin->sd.getNewSampleName("kick.wav").c_str(), true);

        // 4. Add to library
        // plugin->addCurrentSampleToLibrary();
    }
    else if (button == makeSnare)
    {
        // 0. Check if a source is loaded
        if (!plugin->fSourceLoaded)
            return;

        // 1. pick random start
        std::vector<long> starts;

        for (auto &m : plugin->fSourceMeasurements)
        {
            if (m.rms > 0.1 && m.specFlat > 0.9f)
            {
                starts.push_back(m.frame);
            }
        }

        if (starts.size() == 0)
        {
            return;
        }

        int i = random.next() % starts.size();
        plugin->selectWaveform(&plugin->fSourceWaveform, starts[i]);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::SnarePreset);

        // 3. Set sample name
        sampleName->setText(plugin->sd.getNewSampleName("snare.wav").c_str(), true);
    }
    else if (button == makeHihat)
    {
        // 0. Check if a source is loaded
        if (!plugin->fSourceLoaded)
            return;

        // 1. pick random start
        std::vector<long> starts;

        for (auto &m : plugin->fSourceMeasurements)
        {
            if (m.rms > 0.1 && m.specFlat > 0.9f)
            {
                starts.push_back(m.frame);
            }
        }

        if (starts.size() == 0)
        {
            return;
        }

        int i = random.next() % starts.size();
        plugin->selectWaveform(&plugin->fSourceWaveform, starts[i]);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::HiHat);

        // 3. Set sample name
        sampleName->setText(plugin->sd.getNewSampleName("hihat.wav").c_str(), true);
    }
    else if (button == makeClap)
    {
        // 0. Check if a source is loaded
        if (!plugin->fSourceLoaded)
            return;

        // 1. pick random start
        std::vector<long> starts;

        for (auto &m : plugin->fSourceMeasurements)
        {
            if (m.rms > 0.1 && m.specFlat > 0.9f)
            {
                starts.push_back(m.frame);
            }
        }

        if (starts.size() == 0)
        {
            return;
        }

        int i = random.next() % starts.size();
        plugin->selectWaveform(&plugin->fSourceWaveform, starts[i]);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::Clap);

        // 3. Set sample name
        sampleName->setText(plugin->sd.getNewSampleName("clap.wav").c_str(), true);
    }
    else if (button == browseFilesBtn)
        SystemOpenDirectory(plugin->sd.getSampleFolder());
    else if (button == openMapBtn)
        sampleBrowserRoot->show();

    repaint();
}

void WAIVESamplerUI::mapSampleImport()
{
    beginOpenFileBrowser("import", true);
}

void WAIVESamplerUI::beginOpenFileBrowser(const std::string &state, bool multiple)
{
    if (filebrowserOpen.load(std::memory_order_acquire))
        return;

    if (open_dialog.joinable())
        open_dialog.join();

    open_dialog = std::thread(
        [this, state, multiple]()
        {
            {
                std::lock_guard<std::mutex> lock(fileBrowserOpenMtx);
                filebrowserOpen.store(true, std::memory_order_release);
            }

            // filebrowserOpen = true;
            char const *filename = nullptr;
            char const *filterPatterns[2] = {"*.mp3", "*.wav"};
            filename = tinyfd_openFileDialog(
                "Open audio file...",
                "./",
                2,
                filterPatterns,
                "Audio files",
                multiple);

            {
                std::lock_guard<std::mutex> lock(fileBrowserOpenMtx);
                filebrowserOpen.store(false, std::memory_order_release);
            }
            fileBrowserCV.notify_one();

            if (filename)
            {
                std::cout << filename << std::endl;
                setState(state.c_str(), filename);
            }
        });

    std::unique_lock<std::mutex> lock(fileBrowserOpenMtx);
    fileBrowserCV.wait(lock, [this]
                       { return !filebrowserOpen.load(std::memory_order_acquire); });
}

void WAIVESamplerUI::waveformSelection(Waveform *waveform, uint selectionStart)
{
    plugin->selectWaveform(&plugin->fSourceWaveform, (int)selectionStart);
}

void WAIVESamplerUI::mapSampleHovered(int id)
{
    plugin->loadSamplePreview(id);
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
    if (textInput == sampleName)
    {
        if (text.length() == 0)
        {
            textInput->undo();
            return;
        }

        if (plugin->fCurrentSample != nullptr)
            plugin->sd.renameSample(plugin->fCurrentSample, text);
    }
    else if (textInput == sourceSearch)
    {
        std::string search = "";
        search.reserve(text.size());

        for (int i = 0; i < text.size(); i++)
        {
            if (text[i] != '"')
                search += text[i];
        }

        if (text.compare(plugin->sd.filterConditions.searchString) != 0)
        {
            plugin->sd.filterConditions.searchString.assign(search);
            plugin->sd.filterSources();
        }
    }
}

void WAIVESamplerUI::textInputChanged(TextInput *textInput, std::string text)
{
    if (textInput == sourceSearch)
    {
        std::string search = "";
        search.reserve(text.size());

        for (int i = 0; i < text.size(); i++)
        {
            if (text[i] != '"')
                search += text[i];
        }

        if (text.compare(plugin->sd.filterConditions.searchString) != 0)
        {
            plugin->sd.filterConditions.searchString.assign(search);
            plugin->sd.filterSources();
        }
    }
}

void WAIVESamplerUI::dropdownSelection(DropDown *widget, int item)
{
    if (widget == filterType)
    {
        setParameterValue(widget->getId(), item);
    }
}

void WAIVESamplerUI::sampleSelected(SampleSlot *slot, int slotId)
{
    plugin->loadSample(sampleSlots[slotId]->getSamplePlayer()->sampleInfo);
    return;
};

void WAIVESamplerUI::sampleSlotCleared(SampleSlot *slot, int slotId)
{
    plugin->clearSamplePlayer(*sampleSlots[slotId]->getSamplePlayer());
    return;
};

void WAIVESamplerUI::sourceDownload(int index)
{
    plugin->sd.downloadSourceFile(index);
}

void WAIVESamplerUI::sourceLoad(int index)
{
    std::string fp = plugin->sd.getFullSourcePath(plugin->sd.sourcesList.at(index));
    plugin->fSourceTagString = makeTagString(plugin->sd.sourcesList.at(index).tags);
    plugin->loadSource(fp.c_str());
    sourceLoading->setLoading(true);
}

void WAIVESamplerUI::sourcePreview(int index)
{
    plugin->sd.playTempSourceFile(index);
}

void WAIVESamplerUI::onNanoDisplay()
{
    float width = getWidth();
    float height = getHeight();

    beginPath();
    fillColor(WaiveColors::dark);
    rect(0.0f, 0.0f, width, height);
    fill();
    closePath();
}

void WAIVESamplerUI::uiScaleFactorChanged(const double scaleFactor)
{
    fScaleFactor = scaleFactor;
}

void WAIVESamplerUI::onPluginUpdated(const void *pSender, const WAIVESampler::PluginUpdate &arg)
{
    bool sourceAvailable;
    switch (arg)
    {
    case WAIVESampler::kSourceLoading:
        break;
    case WAIVESampler::kSourceLoaded:
    case WAIVESampler::kSourceUpdated:
        sourceWaveformDisplay->setWaveform(&plugin->fSourceWaveform);
        sourceWaveformDisplay->setWaveformLength(plugin->fSourceLength);
        sourceWaveformDisplay->waveformNew();
        sourceAvailable = plugin->fSourceLength > 0;
        saveSampleBtn->setEnabled(sourceAvailable);
        sourceLoading->setLoading(false);
        presetButtons->setVisible(sourceAvailable);
        editorKnobs->setVisible(sourceAvailable);
        instructions->setVisible(!sourceAvailable);
        break;
    case WAIVESampler::kSampleLoading:
        break;
    case WAIVESampler::kSampleLoaded:
        sampleWaveformDisplay->setWaveformLength(plugin->fCurrentSample->sampleLength);
        if (plugin->fCurrentSample->saved)
            saveSampleBtn->setLabel("update");
        else
            saveSampleBtn->setLabel("save");
        sampleWaveformDisplay->waveformNew();
        playSampleBtn->setEnabled(true);
        break;
    case WAIVESampler::kSampleUpdated:
        if (plugin->fCurrentSample != nullptr)
        {
            sampleWaveformDisplay->setWaveformLength(plugin->fCurrentSample->sampleLength);
            sampleWaveformDisplay->waveformUpdated();
            if (plugin->fCurrentSample->saved)
                saveSampleBtn->setLabel("update");
            else
                saveSampleBtn->setLabel("save");
        }
        playSampleBtn->setEnabled(true);
        break;
    case WAIVESampler::kSampleAdded:
        break;
    case WAIVESampler::kParametersChanged:
        if (plugin->fCurrentSample != nullptr)
        {
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

            sourceWaveformDisplay->setSelection(plugin->fCurrentSample->sourceStart, false);
            if (plugin->fCurrentSample->waive)
            {
                saveSampleBtn->setVisible(true);
                if (plugin->fCurrentSample->saved)
                    saveSampleBtn->setLabel("update");
                else
                    saveSampleBtn->setLabel("save");
            }
            else
                saveSampleBtn->setVisible(false);
            sampleName->setText(plugin->fCurrentSample->name.c_str(), false);
        }

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
    if (pTask->name().compare("ImporterTask") == 0)
    {
        sampleBrowser->loading->setLoading(true);
    }
    else if (pTask->name().compare("WaveformLoaderTask") == 0)
    {
        sourceLoading->setLoading(true);
        progress->setLabel("Importing...");
        progress->resizeToFit();
        progress->setVisible(true);
    }
    else if (pTask->name().compare("FeatureExtractorTask") == 0)
    {
        sourceLoading->setLoading(true);
        progress->setLabel("Analysing...");
        progress->resizeToFit();
        progress->setVisible(true);
    }
    pNf->release();
}

void WAIVESamplerUI::onTaskProgress(Poco::TaskProgressNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    // printf("WAIVESamplerUI::onTaskProgress: %s progress %.4f\n", pTask->name().c_str(), pTask->progress());

    if (pTask->name().compare("FeatureExtractorTask") == 0)
    {
        progress->setLabel(fmt::format("Analysing...[{:d}%]", (int)(pTask->progress() * 100.f)));
        progress->resizeToFit();
        progress->setVisible(true);
        sourceWaveformDisplay->repaint();
    }
    else if (pTask->name().compare("WaveformLoaderTask") == 0)
    {
        progress->setLabel(fmt::format("Importing...[{:d}%]", (int)(pTask->progress() * 100.f)));
        progress->resizeToFit();
        progress->setVisible(true);
        sourceWaveformDisplay->repaint();
    }
    else if (pTask->name().compare("ParseSourceList") == 0)
    {
        databaseProgress->setLabel(fmt::format("Importing sources [{:d}%]", (int)(pTask->progress() * 100.f)));
        databaseProgress->resizeToFit();
        databaseProgress->setVisible(true);
    }
    else if (pTask->name().compare("ParseTagsList") == 0)
    {
        databaseProgress->setLabel(fmt::format("Importing tags [{:d}%]", (int)(pTask->progress() * 100.f)));
        databaseProgress->resizeToFit();
        databaseProgress->setVisible(true);
    }
    pNf->release();
}

void WAIVESamplerUI::onTaskCancelled(Poco::TaskCancelledNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    // std::cout << "WAIVESamplerUI::onTaskCancelled: " << pTask->name() << std::endl;
    pNf->release();
}

void WAIVESamplerUI::onTaskFailed(Poco::TaskFailedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    std::cerr << "WAIVESamplerUI::onTaskFailed: " << pTask->name() << std::endl;
    std::cerr << "Reason: " << pNf->reason().displayText() << std::endl;
    pNf->release();
}

void WAIVESamplerUI::onTaskFinished(Poco::TaskFinishedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    if (!pTask)
    {
        std::cerr << "Error: pTask is null" << std::endl;
        return;
    }

    const std::string &taskName = pTask->name();

    if (taskName == "ImporterTask")
        sampleBrowser->loading->setLoading(false);
    else if (taskName == "WaveformLoaderTask")
    {
        sourceLoading->setLoading(false);
        progress->setVisible(false);
    }
    else if (taskName == "FeatureExtractorTask")
    {
        sourceLoading->setLoading(false);
        progress->setVisible(false);
    }
    else if (taskName == "ParseSourceList")
    {
        databaseProgress->hide();
    }
    else if (taskName == "ParseTagsList")
    {
        databaseProgress->hide();
    }

    pNf->release();
}

void WAIVESamplerUI::onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg)
{
    // std::cout << "WAIVESamplerUI::onDatabaseChanged " << arg << std::endl;
    switch (arg)
    {
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADING:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_FILTER_START:
    case SampleDatabase::DatabaseUpdate::BUILDING_TAG_LIST:
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOADING:
        databaseLoading->setLoading(true);
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR:
    case SampleDatabase::DatabaseUpdate::TAG_LIST_DOWNLOAD_ERROR:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADED:
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOAD_FAILED:
        databaseLoading->setLoading(false);
        break;
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOADED:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_UPDATED:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_FILTER_END:
        databaseLoading->setLoading(false);
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_READY:
    case SampleDatabase::DatabaseUpdate::SOURCE_PREVIEW_READY:
        break;
    default:
        break;
    }

    switch (arg)
    {
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADING:
        databaseProgress->setLabel("Downloading file list...");
        break;
    case SampleDatabase::DatabaseUpdate::BUILDING_TAG_LIST:
        databaseProgress->setLabel("Building database...");
        break;
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOADING:
        databaseProgress->setLabel("Downloading file...");
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADED:
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOADED:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_UPDATED:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_READY:
        databaseProgress->setLabel("");
        break;
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOAD_FAILED:
    case SampleDatabase::DatabaseUpdate::TAG_LIST_DOWNLOAD_ERROR:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR:
        databaseProgress->setLabel("Error downloading.");
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_PREVIEW_READY:
        previewPlayback->setVisible(true);
        break;
    case SampleDatabase::DatabaseUpdate::SAMPLE_ADDED:
    case SampleDatabase::DatabaseUpdate::SAMPLE_DELETED:
    case SampleDatabase::DatabaseUpdate::SAMPLE_UPDATED:
        sampleBrowser->repaint();
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_FILTER_START:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_FILTER_END:
        break;
    default:
        break;
    }
    databaseProgress->resizeToFit();

    repaint();
}

Knob *WAIVESamplerUI::createWAIVEKnob(
    Parameters param,
    std::string label,
    float min,
    float max,
    float value)
{
    Knob *knob = new Knob(this);
    knob->setId(param);
    knob->label = label;
    knob->min = min;
    knob->max = max;
    knob->setValue(value);
    knob->gauge_width = 3.0f * fScaleFactor;
    knob->setCallback(this);
    knob->setFontSize(14.f);
    knob->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    knob->setRadius(16.f);
    knob->resizeToFit();

    return knob;
}

END_NAMESPACE_DISTRHO