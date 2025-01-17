#include "WAIVESamplerUI.hpp"
START_NAMESPACE_DISTRHO

WAIVESamplerUI::WAIVESamplerUI() : UI(UI_W, UI_H),
                                   fScaleFactor(getScaleFactor()),
                                   fScale(1.0f),
                                   filebrowserOpen(false),
                                   errorMessage(false)
{
    plugin = static_cast<WAIVESampler *>(getPluginInstancePointer());

    random.seed();

    float width = UI_W * fScaleFactor;
    float height = UI_H * fScaleFactor;
    float padding = 10.f * fScaleFactor;

    std::cout << "fScaleFactor: " << fScaleFactor << std::endl;
    std::cout << "UI_W: " << UI_W << ", UI_H: " << UI_H << std::endl;
    std::cout << "width: " << width << ", height: " << height << std::endl;

    fontTitle = createFontFromMemory("VG5000", VG5000, VG5000_len, false);
    fontMain = createFontFromMemory("Poppins-Light", Poppins_Light, Poppins_Light_len, false);

    float col1Width = 539.f;
    float col2Width = 298.f;

    // 1 ----- Source Browser Panel
    {
        sourceBrowserPanel = new Panel(this);
        sourceBrowserPanel->setSize(col1Width, 254.f);
        sourceBrowserPanel->setAbsolutePos(padding, padding);
        sourceBrowserPanel->setFont("VG5000", VG5000, VG5000_len);
        sourceBrowserPanel->label = "1";
        sourceBrowserPanel->title = "Source";
        sourceBrowserPanel->expandable = true;

        importSource = new Button(this);
        importSource->setLabel("Import");
        importSource->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        importSource->setFontSize(14.f);
        importSource->resizeToFit();
        importSource->onTop(sourceBrowserPanel, END, START, 14);
        importSource->setCallback(this);
        sourceBrowserPanel->addChildWidget(importSource);

        sourceList = new SourceList(this);
        sourceList->setSize(539.f - 24.f - 24.f, 150.f);
        sourceList->onTop(sourceBrowserPanel, CENTER, START, 52.f);
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
        sourceBrowserPanel->addChildWidget(sourceList);

        openFilterPanelBtn = new Button(this);
        openFilterPanelBtn->isToggle = true;
        openFilterPanelBtn->setLabel("Filter");
        openFilterPanelBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        openFilterPanelBtn->setFontSize(14.f);
        openFilterPanelBtn->resizeToFit();
        float spacing = Layout::measureVertical(sourceList, Widget_Align::END, sourceBrowserPanel, Widget_Align::END);
        spacing = (spacing - openFilterPanelBtn->getHeight()) / 2.f;
        openFilterPanelBtn->below(sourceList, START, spacing);
        openFilterPanelBtn->setCallback(this);
        sourceBrowserPanel->addChildWidget(openFilterPanelBtn);

        searchBox = new Box(this);
        searchBox->setSize(260 * fScaleFactor, openFilterPanelBtn->getHeight(), true);
        searchBox->radius = searchBox->getHeight() / 2;
        searchBox->background_color = WaiveColors::grey2;
        searchBox->rightOf(openFilterPanelBtn);
        sourceBrowserPanel->addChildWidget(searchBox);

        sourceSearch = new TextInput(this);
        sourceSearch->placeholder = "Search...";
        sourceSearch->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        sourceSearch->setFontSize(14.f);
        sourceSearch->foreground_color = WaiveColors::light1;
        sourceSearch->background_color = searchBox->background_color;
        sourceSearch->setSize(searchBox->getWidth() - padding * 4.f - sourceSearch->getFontSize(), sourceSearch->getFontSize(), true);
        sourceSearch->onTop(searchBox, START, CENTER, padding * 2.f);
        sourceSearch->setCallback(this);
        sourceBrowserPanel->addChildWidget(sourceSearch);

        searchIcon = new Icon(this);
        searchIcon->setImageData(search, search_len, 85, 85, IMAGE_GENERATE_MIPMAPS);
        searchIcon->setSize(sourceSearch->getFontSize() - 4.f, sourceSearch->getFontSize() - 4.f, true);
        searchIcon->onTop(searchBox, END, CENTER, searchBox->radius);
        sourceBrowserPanel->addChildWidget(searchIcon);

        archiveList = new RadioButtons(this);
        archiveList->setFontSize(18.0f);
        archiveList->radioSize = 4.f * fScaleFactor;
        archiveList->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        archiveList->addItem("All");
        archiveList->setItem(0, false);
        archiveList->calculateWidth();
        archiveList->calculateHeight();
        archiveList->setCallback(this);

        databaseLoading = new Spinner(this);
        databaseLoading->setSize(sourceSearch->getHeight(), sourceSearch->getHeight(), true);
        databaseLoading->rightOf(searchBox, CENTER, padding * 2.f);
        sourceBrowserPanel->addChildWidget(databaseLoading);

        databaseProgress = new Label(this, "");
        databaseProgress->rightOf(databaseLoading, START);
        databaseProgress->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        sourceBrowserPanel->addChildWidget(databaseProgress);

        previewPlaybackBtn = new Button(this);
        previewPlaybackBtn->setLabel("Stop");
        previewPlaybackBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        previewPlaybackBtn->resizeToFit();
        previewPlaybackBtn->onTop(sourceBrowserPanel, END, END, padding * 2.f);
        previewPlaybackBtn->setCallback(this);
        previewPlaybackBtn->setVisible(false);

        randomSourceBtn = new Button(this);
        randomSourceBtn->setLabel("Random");
        randomSourceBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        randomSourceBtn->setFontSize(14.f);
        randomSourceBtn->resizeToFit();
        randomSourceBtn->below(sourceList, Widget_Align::END, spacing);
        randomSourceBtn->setCallback(this);
        sourceBrowserPanel->addChildWidget(randomSourceBtn);
    }

    // 2 ----- Sample Editor Panel
    {
        sampleEditorPanel = new Panel(this);
        sampleEditorPanel->setSize(col1Width, 356.f);
        sampleEditorPanel->below(sourceBrowserPanel, START, 5.f);
        sampleEditorPanel->setFont("VG5000", VG5000, VG5000_len);
        sampleEditorPanel->label = "2";
        sampleEditorPanel->title = "Sample";

        sampleWaveformDisplay = new Waveform(this);
        sampleWaveformDisplay->setSize(sampleEditorPanel->getWidth() - 2.f * 24.f, 100.f * fScaleFactor, true);
        sampleWaveformDisplay->onTop(sampleEditorPanel, CENTER, START, (103.f - 50.f));
        sampleWaveformDisplay->background_color = sampleEditorPanel->background_color;
        sampleWaveformDisplay->setWaveform(plugin->editorPreviewWaveform);
        sampleEditorPanel->addChildWidget(sampleWaveformDisplay);

        sourceLoading = new Spinner(this);
        sourceLoading->setSize(16, 16);
        sourceLoading->onTop(sampleWaveformDisplay, START, START, padding * 2);
        sourceLoading->setLoading(false);

        progress = new Label(this, "");
        progress->setFontSize(12.f);
        progress->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        progress->resizeToFit();
        progress->rightOf(sourceLoading, START, padding);

        presetLabel = new Label(this, "Detect");
        presetLabel->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        presetLabel->resizeToFit();

        knobsLabel = new Label(this, "Adjust");
        knobsLabel->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        knobsLabel->resizeToFit();

        makeKick = new Button(this);
        makeKick->setLabel("Kick");
        makeKick->isToggle = true;
        makeKick->setFontSize(16.f);
        makeKick->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        makeKick->setSize(100.f, 32.f);
        makeKick->setCallback(this);

        makeSnare = new Button(this);
        makeSnare->setLabel("Snare");
        makeSnare->isToggle = true;
        makeSnare->setFontSize(16.f);
        makeSnare->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        makeSnare->setSize(100.f, 32.f);
        makeSnare->setCallback(this);

        makeHihat = new Button(this);
        makeHihat->setLabel("Hi-Hat");
        makeHihat->isToggle = true;
        makeHihat->setFontSize(16.f);
        makeHihat->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        makeHihat->setSize(100.f, 32.f);
        makeHihat->setCallback(this);

        makeClap = new Button(this);
        makeClap->setLabel("Clap");
        makeClap->isToggle = true;
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
        presetButtons->below(sampleWaveformDisplay, START, 2.f * padding);
        presetButtons->setWidth(sampleWaveformDisplay->getWidth());
        presetButtons->setHeight(makeKick->getHeight());
        presetButtons->justify_content = HBox::Justify_Content::space_evenly;
        presetButtons->positionWidgets();

        presetButtons->setVisible(false);

        sustainLength = createWAIVEKnob(kSustainLength, "Length", 0.0f, 5000.0f, 200.f);
        sustainLength->format = "{:.0f}ms";
        sustainLength->vertical = false;
        sustainLength->resizeToFit();
        sustainLength->onTop(sampleEditorPanel, Widget_Align::END, Widget_Align::START, 24.f, 16.f);

        pitch = createWAIVEKnob(kSamplePitch, "Pitch", 0.25f, 4.f, 1.0f);
        volume = createWAIVEKnob(kSampleVolume, "Volume", 0.0f, 2.0f, 1.0f);
        percussionBoost = createWAIVEKnob(kPercussiveBoost, "Hit", 0.0f, 2.0f, 0.5f);

        ampAttack = createWAIVEKnob(kAmpAttack, "Attack", 0.0f, 500.0f, 10.0f);
        ampAttack->format = "{:.0f}ms";
        ampDecay = createWAIVEKnob(kAmpDecay, "Decay", 0.0f, 500.0f, 50.0f);
        ampDecay->format = "{:.0f}ms";
        ampSustain = createWAIVEKnob(kAmpSustain, "Sustain", 0.0f, 1.0f, 0.7f);
        ampRelease = createWAIVEKnob(kAmpRelease, "Release", 0.0f, 500.0f, 100.0f);
        ampRelease->format = "{:.0f}ms";

        filterCutoff = createWAIVEKnob(kFilterCutoff, "Cutoff", 0.0, 0.999, 0.999);
        filterResonance = createWAIVEKnob(kFilterResonance, "Res.", 0.0, 1.0, 0.0);

        filterType = new DropDown(this);
        filterType->setFontSize(16.0f);
        filterType->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        filterType->menu->setFontSize(16.0f);
        filterType->menu->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        filterType->addItem("Low");
        filterType->addItem("High");
        filterType->addItem("Band");
        filterType->setId(kFilterType);
        filterType->setDisplayNumber(3);
        filterType->resizeToFit();
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
        instructions->below(sampleWaveformDisplay, CENTER, Layout::measureVertical(sampleWaveformDisplay, END, sampleEditorPanel, END) / 2.f);

        saveSampleBtn = new Button(this);
        saveSampleBtn->setLabel("Add To Player");
        saveSampleBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        saveSampleBtn->resizeToFit();
        saveSampleBtn->setCallback(this);
        saveSampleBtn->setEnabled(false);
        // saveSampleBtn->onTop(sampleEditorPanel, Widget_Align::CENTER, Widget_Align::END, padding, 24.f);

        playSampleBtn = new Button(this);
        playSampleBtn->setLabel(" ▶");
        playSampleBtn->background_color = WaiveColors::light2;
        playSampleBtn->text_color = WaiveColors::dark;
        playSampleBtn->resizeToFit();
        // playSampleBtn->leftOf(saveSampleBtn, CENTER, 10.f);
        playSampleBtn->setCallback(this);

        HBox sampleBtns(this);
        sampleBtns.addWidget(playSampleBtn);
        sampleBtns.addWidget(saveSampleBtn);
        sampleBtns.justify_content = HBox::Justify_Content::center;
        sampleBtns.padding = 2.f * padding;
        sampleBtns.resizeToFit();
        sampleBtns.setWidth(sampleEditorPanel->getWidth() - 4.f * padding);
        sampleBtns.onTop(sampleEditorPanel, Widget_Align::CENTER, Widget_Align::END, padding, 24.f);
        sampleBtns.positionWidgets();

        // TODO: add this button somewhere?
        newSampleBtn = new Button(this);
    }

    // 3 ----- Sample Viewer Panel
    {
        // samplePanel = new Panel(this);
        // samplePanel->setSize(col2Width, panelHeights / 3.f, true);
        // samplePanel->rightOf(sourceBrowserPanel, START, padding);
        // samplePanel->setFont("VG5000", VG5000, VG5000_len);
        // samplePanel->label = "3";
        // samplePanel->title = "Sample";

        // sampleName = new TextInput(this);
        // sampleName->setSize(samplePanel->getWidth() * 0.8f, 20, true);
        // sampleName->below(sampleWaveformDisplay, CENTER, padding);
        // sampleName->setCallback(this);
        // sampleName->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        // sampleName->align = Align::ALIGN_CENTER;
        // sampleName->foreground_color = WaiveColors::light1;

        // newSampleBtn->setLabel("New sample");
        // newSampleBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        // newSampleBtn->resizeToFit();
        // newSampleBtn->setCallback(this);

        // HBox alignSampleButtons(this);
        // alignSampleButtons.addWidget(newSampleBtn);
        // alignSampleButtons.addWidget(saveSampleBtn);
        // alignSampleButtons.justify_content = HBox::Justify_Content::center;
        // alignSampleButtons.padding = 2.f * padding;
        // alignSampleButtons.resizeToFit();
        // alignSampleButtons.setWidth(samplePanel->getWidth() - 4.f * padding);
        // alignSampleButtons.onTop(samplePanel, CENTER, END, padding * 2.f);
        // alignSampleButtons.positionWidgets();
    }

    // 3 ----- Sample Player Panel
    {
        samplePlayerPanel = new Panel(this);
        samplePlayerPanel->setSize(col2Width, Layout::measureVertical(sourceBrowserPanel, Widget_Align::START, sampleEditorPanel, Widget_Align::END), true);
        samplePlayerPanel->setAbsolutePos(width - padding - col2Width, padding);
        samplePlayerPanel->setFont("VG5000", VG5000, VG5000_len);
        samplePlayerPanel->label = "3";
        samplePlayerPanel->title = "Player";
        samplePlayerPanel->expandable = true;
        samplePlayerPanel->expand_right = false;
        samplePlayerPanel->expand_h = width - padding - padding;
        std::cout << "samplePlayerPanel->expand_h = " << samplePlayerPanel->expand_h << std::endl;
        samplePlayerPanel->expand_v = samplePlayerPanel->getHeight();

        openMapBtn = new Button(this);
        openMapBtn->setLabel("Sample Map");
        openMapBtn->isToggle = true;
        openMapBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        openMapBtn->resizeToFit();
        openMapBtn->setCallback(this);
        samplePlayerPanel->addChildWidget(openMapBtn);

        browseFilesBtn = new Button(this);
        browseFilesBtn->setLabel("View Folder");
        browseFilesBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
        browseFilesBtn->resizeToFit();
        browseFilesBtn->setCallback(this);
        samplePlayerPanel->addChildWidget(browseFilesBtn);

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
        sampleSlotsContainer->onTop(samplePlayerPanel, CENTER, START, 52.f);
        sampleSlotsContainer->setHeight(Layout::measureVertical(sampleSlotsContainer, START, openMapBtn, START) - 10.f);
        samplePlayerPanel->addChildWidget(sampleSlotsContainer);

        for (int i = 0; i < NUM_SLOTS; i++)
        {
            SampleSlot *slot = new SampleSlot(this);
            slot->setSamplePlayer(&plugin->samplePlayers[i]);
            slot->setCallback(this);
            slot->slotId = i;
            slot->setSize(sampleSlotsContainer->getWidth(), sampleSlotsContainer->getHeight() / NUM_SLOTS - 4, true);
            slot->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
            slot->setFontSize(12.f);
            slot->currentSample = plugin->fCurrentSample;

            addIdleCallback(slot);

            sampleSlotsContainer->addWidget(slot);
            sampleSlots.push_back(slot);
        }

        sampleSlotsContainer->positionWidgets();
        for (int i = 0; i < sampleSlots.size(); i++)
            sampleSlots[i]->repositionWidgets();
    }

    // 4 ----- Filter Map
    {
        archiveListHeading = new Label(sourceBrowserPanel, "Archive Collection");
        archiveListHeading->setFontSize(14.f);
        archiveListHeading->setFont("VG5000", VG5000, VG5000_len);
        archiveListHeading->resizeToFit();
        archiveListHeading->setAbsolutePos(sourceList->getLeft(), sampleEditorPanel->getTop());

        browseTagsHeading = new Label(sourceBrowserPanel, "Browse Tags");
        browseTagsHeading->setFontSize(14.f);
        browseTagsHeading->setFont("VG5000", VG5000, VG5000_len);
        browseTagsHeading->resizeToFit();
        browseTagsHeading->rightOf(archiveListHeading, CENTER, 52.f);

        sourceBrowserPanel->expand_v = sampleEditorPanel->getBottom() - sourceBrowserPanel->getTop();
        sourceBrowserPanel->expand_h = sourceBrowserPanel->getWidth();
        sourceBrowserPanel->expand();

        sourceBrowserPanel->hiddenWidgets.addChildWidget(archiveListHeading);
        sourceBrowserPanel->hiddenWidgets.addChildWidget(browseTagsHeading);

        archiveList->below(archiveListHeading, START);
        sourceBrowserPanel->hiddenWidgets.addChildWidget(archiveList);

        tagBrowser = new TagBrowser(sourceBrowserPanel, &plugin->sd);
        tagBrowser->setSize(
            sourceList->getRight() - browseTagsHeading->getLeft(),
            sourceBrowserPanel->getBottom() - archiveList->getTop() - 24.f * fScaleFactor,
            true);
        tagBrowser->onTop(sourceBrowserPanel, Widget_Align::END, Widget_Align::END, 24.f * fScaleFactor);
        tagBrowser->repositionWidgets();
        sourceBrowserPanel->hiddenWidgets.addChildWidget(tagBrowser);

        sourceBrowserPanel->collapse();
    }

    // 5 ----- Sample Map
    {
        // sampleBrowserRoot = new Popup(
        //     this,
        //     sourceBrowserPanel->getAbsoluteX(),
        //     sourceBrowserPanel->getAbsoluteY(),
        //     sourceBrowserPanel->getWidth(),
        //     Layout::measureVertical(sourceBrowserPanel, Widget_Align::START, sampleEditorPanel, Widget_Align::END),
        //     true);
        // sampleBrowserRoot->title = "Browse samples";
        // sampleBrowserRoot->setFont("VG5000", VG5000, VG5000_len);
        // sampleBrowserRoot->close();
        // sampleBrowserRoot->setCallback(this);

        sampleBrowser = new SampleBrowser(samplePlayerPanel, &plugin->sd);
        sampleBrowser->setSize(sourceList->getWidth(), tagBrowser->getBottom() - sourceList->getTop());
        sampleBrowser->setAbsolutePos(sourceList->getLeft(), sourceList->getTop());
        sampleBrowser->repositionWidgets();
        sampleBrowser->setCallback(this);
        samplePlayerPanel->hiddenWidgets.addChildWidget(sampleBrowser);
        samplePlayerPanel->hiddenWidgets.hide();
    }

    setGeometryConstraints(width, height, false, false);

    if (fScaleFactor != 1.0)
        setSize(width, height);

    // register notifications
    {
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
    }

    updateWidgets();

    plugin->sd.checkLatestRemoteVersion();

    printf("** WAIVESamplerUI initialised: (%.0f, %.0f)\n", width, height);
}

WAIVESamplerUI::~WAIVESamplerUI()
{
    // std::cout << "WAIVESamplerUI::~WAIVESamplerUI destructor" << std::endl;
    plugin->taskManager.removeObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskStartedNotification>(*this, &WAIVESamplerUI::onTaskStarted));
    plugin->taskManager.removeObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFinishedNotification>(*this, &WAIVESamplerUI::onTaskFinished));
    plugin->taskManager.removeObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskProgressNotification>(*this, &WAIVESamplerUI::onTaskProgress));
    plugin->taskManager.removeObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskCancelledNotification>(*this, &WAIVESamplerUI::onTaskCancelled));
    plugin->taskManager.removeObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFailedNotification>(*this, &WAIVESamplerUI::onTaskFailed));

    plugin->sd.taskManager.removeObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskStartedNotification>(*this, &WAIVESamplerUI::onTaskStarted));
    plugin->sd.taskManager.removeObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFinishedNotification>(*this, &WAIVESamplerUI::onTaskFinished));
    plugin->sd.taskManager.removeObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskProgressNotification>(*this, &WAIVESamplerUI::onTaskProgress));
    plugin->sd.taskManager.removeObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskCancelledNotification>(*this, &WAIVESamplerUI::onTaskCancelled));
    plugin->sd.taskManager.removeObserver(Poco::Observer<WAIVESamplerUI, Poco::TaskFailedNotification>(*this, &WAIVESamplerUI::onTaskFailed));

    plugin->sd.databaseUpdate -= Poco::delegate(this, &WAIVESamplerUI::onDatabaseChanged);
    plugin->pluginUpdate -= Poco::delegate(this, &WAIVESamplerUI::onPluginUpdated);

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
    case kSlot9MidiNumber:
    case kSlot10MidiNumber:
    case kSlot11MidiNumber:
    case kSlot12MidiNumber:
    case kSlot13MidiNumber:
    case kSlot14MidiNumber:
    case kSlot15MidiNumber:
    case kSlot16MidiNumber:
    case kSlot17MidiNumber:
    case kSlot18MidiNumber:
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

    repaint();
}

void WAIVESamplerUI::knobDragStarted(Knob *knob)
{
    // valueIndicator->setAbsoluteX(knob->getAbsoluteX());
    // valueIndicator->setWidth(knob->getWidth());
    // valueIndicator->setAbsoluteY(knob->getAbsoluteY() + knob->getHeight());
    // valueIndicator->setFormatString(knob->getFormat());
    // valueIndicator->setValue(knob->getValue());
    // valueIndicator->toFront();
    // valueIndicator->show();
}

void WAIVESamplerUI::knobDragFinished(Knob *knob, float value)
{
    // valueIndicator->hide();
    // repaint();

    plugin->triggerPreview();
}

void WAIVESamplerUI::knobValueChanged(Knob *knob, float value)
{
    setParameterValue(knob->getId(), value);
    // valueIndicator->setValue(knob->getValue());
}

void WAIVESamplerUI::buttonClicked(Button *button)
{
    if (button == importSource)
        beginOpenFileBrowser("importSource", true);
    else if (button == saveSampleBtn)
        plugin->addCurrentSampleToLibrary();
    else if (button == playSampleBtn)
        plugin->triggerPreview();
    else if (button == previewPlaybackBtn)
    {
        plugin->stopSourcePreview();
        previewPlaybackBtn->setVisible(false);
    }
    // else if (button == sourcePreviewBtn)
    //     plugin->playSourcePreview();
    else if (button == newSampleBtn)
        plugin->newSample();
    else if (button == makeKick)
    {
        makeKick->setToggled(true);
        makeSnare->setToggled(false);
        makeClap->setToggled(false);
        makeHihat->setToggled(false);
        // 0. Check if a source is loaded
        if (!plugin->fSourceLoaded)
            return;

        // 1. Select random candidate area of source
        int nCandidates = plugin->fSourceFeatures.size();
        if (nCandidates == 0)
            return; // or pick a random spot?

        sampleWaveformDisplay->text_color = WaiveColors::accent2;
        int i = random.next() % nCandidates;
        int startIndex = plugin->fSourceFeatures[i].start;
        plugin->selectWaveform(&plugin->fSourceWaveform, startIndex);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::KickPreset);

        // 3. Set sample name
        // sampleName->setText(plugin->sd.getNewSampleName("kick.wav").c_str(), true);
    }
    else if (button == makeSnare)
    {
        makeKick->setToggled(false);
        makeSnare->setToggled(true);
        makeClap->setToggled(false);
        makeHihat->setToggled(false);
        // 0. Check if a source is loaded
        if (!plugin->fSourceLoaded)
            return;

        // 1. pick random start
        std::vector<long> starts;

        for (auto &m : plugin->fSourceMeasurements)
        {
            if (m.rms > 0.1 && m.specFlat > 0.9f)
                starts.push_back(m.frame);
        }

        if (starts.size() == 0)
            return;

        sampleWaveformDisplay->text_color = WaiveColors::accent1;

        int i = random.next() % starts.size();
        plugin->selectWaveform(&plugin->fSourceWaveform, starts[i]);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::SnarePreset);

        // 3. Set sample name
        std::cout << plugin->fSourceTagString << std::endl;
        // sampleName->setText(plugin->sd.getNewSampleName("snare.wav").c_str(), true);
    }
    else if (button == makeHihat)
    {
        makeKick->setToggled(false);
        makeSnare->setToggled(false);
        makeClap->setToggled(false);
        makeHihat->setToggled(true);
        // 0. Check if a source is loaded
        if (!plugin->fSourceLoaded)
            return;

        // 1. pick random start
        std::vector<long> starts;

        for (auto &m : plugin->fSourceMeasurements)
        {
            if (m.rms > 0.1 && m.specFlat > 0.9f)
                starts.push_back(m.frame);
        }

        if (starts.size() == 0)
            return;

        sampleWaveformDisplay->text_color = WaiveColors::accent3;

        int i = random.next() % starts.size();
        plugin->selectWaveform(&plugin->fSourceWaveform, starts[i]);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::HiHat);

        // 3. Set sample name
        // sampleName->setText(plugin->sd.getNewSampleName("hihat.wav").c_str(), true);
    }
    else if (button == makeClap)
    {
        makeKick->setToggled(false);
        makeSnare->setToggled(false);
        makeClap->setToggled(true);
        makeHihat->setToggled(false);

        // 0. Check if a source is loaded
        if (!plugin->fSourceLoaded)
            return;

        // 1. pick random start
        std::vector<long> starts;

        for (auto &m : plugin->fSourceMeasurements)
        {
            if (m.rms > 0.1 && m.specFlat > 0.9f)
                starts.push_back(m.frame);
        }

        if (starts.size() == 0)
            return;

        sampleWaveformDisplay->text_color = WaiveColors::accent4;

        int i = random.next() % starts.size();
        plugin->selectWaveform(&plugin->fSourceWaveform, starts[i]);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::Clap);

        // 3. Set sample name
        // sampleName->setText(plugin->sd.getNewSampleName("clap.wav").c_str(), true);
    }
    else if (button == browseFilesBtn)
        SystemOpenDirectory(plugin->sd.getSampleFolder());
    else if (button == openMapBtn)
    {
        if (button->getToggled())
            samplePlayerPanel->expand();
        else
            samplePlayerPanel->collapse();
    }
    else if (button == openFilterPanelBtn)
    {
        if (button->getToggled())
            sourceBrowserPanel->expand();
        else
            sourceBrowserPanel->collapse();
    }
    else if (button == randomSourceBtn)
        sourceList->selectRandom();

    repaint();
}

void WAIVESamplerUI::mapSampleImport()
{
    beginOpenFileBrowser("importSample", true);
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
    sourceList->selected = -1; // TODO: select correct source!
}

void WAIVESamplerUI::mapSampleLoadSlot(int id, int slot)
{
    plugin->loadSlot(slot, id);
}

void WAIVESamplerUI::textEntered(TextInput *textInput, std::string text)
{
    // if (textInput == sampleName)
    // {
    //     if (text.length() == 0)
    //     {
    //         textInput->undo();
    //         return;
    //     }

    //     if (plugin->fCurrentSample != nullptr)
    //         plugin->sd.renameSample(plugin->fCurrentSample, text);
    // }
    // else if (textInput == sourceSearch)
    // {
    //     std::string search = "";
    //     search.reserve(text.size());

    //     for (int i = 0; i < text.size(); i++)
    //     {
    //         if (text[i] != '"')
    //             search += text[i];
    //     }

    //     if (text.compare(plugin->sd.filterConditions.searchString) != 0)
    //     {
    //         plugin->sd.filterConditions.searchString.assign(search);
    //         plugin->sd.filterSources();
    //     }
    // }
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

void WAIVESamplerUI::onMenuItemSelection(Menu *menu, int item, const std::string &value)
{
    if (menu == archiveList)
    {
        if (item)
        {
            plugin->sd.filterConditions.archiveIs.assign(archiveList->getCurrentItem());
        }
        else
        {
            plugin->sd.filterConditions.archiveIs.assign("");
        }
        plugin->sd.filterSources();
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
    plugin->clear();
    sourceList->selected = -1;
    return;
};

void WAIVESamplerUI::sourceDownload(int index)
{
    plugin->sd.downloadSourceFile(index);
}

void WAIVESamplerUI::sourceLoad(int index)
{
    if (index < 0)
        return;
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

    float middle = sampleEditorPanel->getBottom() + 0.5f * (height - sampleEditorPanel->getBottom());
    beginPath();
    fillColor(WaiveColors::grey2);
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_CENTER);
    fontFaceId(fontTitle);
    fontSize(18.f);
    text(width / 2.f, middle, "waive sampler", nullptr);
    closePath();

    beginPath();
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_RIGHT);
    fontFaceId(fontMain);
    fontSize(12.f);
    text(width - 10.f, middle, fmt::format("v{:d}.{:d}.{:d}", V_MAJ, V_MIN, V_PAT).c_str(), nullptr);
    closePath();
}

void WAIVESamplerUI::uiScaleFactorChanged(const double scaleFactor)
{
    fScaleFactor = scaleFactor;
}

void WAIVESamplerUI::updateWidgets()
{
    bool sourceAvailable = plugin->fSourceLength > 0;
    saveSampleBtn->setEnabled(sourceAvailable);
    sourceLoading->setLoading(false);
    presetButtons->setVisible(sourceAvailable);
    editorKnobs->setVisible(sourceAvailable);
    instructions->setVisible(!sourceAvailable);

    if (plugin->fCurrentSample != nullptr)
    {
        sampleWaveformDisplay->setWaveformLength(plugin->fCurrentSample->sampleLength);
        sampleWaveformDisplay->waveformUpdated();
        // if (plugin->fCurrentSample->saved)
        //     saveSampleBtn->setLabel("Update");
        // else
        //     saveSampleBtn->setLabel("Save");
        playSampleBtn->setEnabled(true);
    }
}

void WAIVESamplerUI::onPluginUpdated(const void *pSender, const WAIVESampler::PluginUpdate &arg)
{
    std::cout << "WAIVESamplerUI::onPluginUpdated " << arg << std::endl;
    bool sourceAvailable;
    switch (arg)
    {
    case WAIVESampler::kSourceLoading:
        break;
    case WAIVESampler::kSourceLoaded:
    case WAIVESampler::kSourceUpdated:
        // sourceWaveformDisplay->setWaveform(&plugin->fSourceWaveform);
        // sourceWaveformDisplay->setWaveformLength(plugin->fSourceLength);
        // sourceWaveformDisplay->waveformNew();
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
        sourceList->repaint();
        for (int i = 0; i < sampleSlots.size(); i++)
        {
            sampleSlots[i]->currentSample = plugin->fCurrentSample;
            sampleSlots[i]->repaint();
        }

        if (plugin->fCurrentSample == nullptr)
        {
            sampleWaveformDisplay->setWaveform(nullptr);
            sampleWaveformDisplay->setWaveformLength(0);
            sampleWaveformDisplay->waveformNew();
            playSampleBtn->setEnabled(false);
        }
        else
        {
            sampleWaveformDisplay->setWaveformLength(plugin->fCurrentSample->sampleLength);
            if (plugin->fCurrentSample->saved)
                saveSampleBtn->setLabel("Update");
            else
                saveSampleBtn->setLabel("Save");
            sampleWaveformDisplay->waveformNew();
            playSampleBtn->setEnabled(true);
        }
        break;
    case WAIVESampler::kSampleUpdated:
        if (plugin->fCurrentSample != nullptr)
        {
            sampleWaveformDisplay->setWaveformLength(plugin->fCurrentSample->sampleLength);
            sampleWaveformDisplay->waveformUpdated();
            if (plugin->fCurrentSample->saved)
                saveSampleBtn->setLabel("Update");
            else
                saveSampleBtn->setLabel("Save");
            playSampleBtn->setEnabled(true);
        }
        else
        {
            sampleWaveformDisplay->setWaveformLength(0);
            sampleWaveformDisplay->waveformNew();
            playSampleBtn->setEnabled(false);
        }
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

            // sourceWaveformDisplay->setSelection(plugin->fCurrentSample->sourceStart, false);
            if (plugin->fCurrentSample->saved)
                saveSampleBtn->setLabel("Update");
            else
                saveSampleBtn->setLabel("Save");
            // sampleName->setText(plugin->fCurrentSample->name.c_str(), false);
        }

        break;
    case WAIVESampler::kSlotLoaded:
        for (int i = 0; i < sampleSlots.size(); i++)
        {
            sampleSlots[i]->currentSample = plugin->fCurrentSample;
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
    // std::cout << "WAIVESamplerUI::onTaskStarted ";
    Poco::Task *pTask = pNf->task();

    const std::string &taskName = pTask->name();
    // std::cout << taskName << std::endl;

    if (taskName == "ImporterTask")
    {
        sampleBrowser->loading->setLoading(true);
    }
    else if (taskName == "WaveformLoaderTask")
    {
        std::cout << "sourceLoading: " << sourceLoading << std::endl;
        sourceLoading->setLoading(true);
        progress->setLabel("Importing...");
        progress->resizeToFit();
        progress->setVisible(true);
    }
    else if (taskName == "FeatureExtractorTask")
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

    const std::string &taskName = pTask->name();

    if (taskName == "FeatureExtractorTask")
    {
        progress->setLabel(fmt::format("Analysing...[{:d}%]", (int)(pTask->progress() * 100.f)));
        progress->resizeToFit();
        progress->setVisible(true);
        // sourceWaveformDisplay->repaint();
    }
    else if (taskName == "WaveformLoaderTask")
    {
        progress->setLabel(fmt::format("Importing...[{:d}%]", (int)(pTask->progress() * 100.f)));
        progress->resizeToFit();
        progress->setVisible(true);
        // sourceWaveformDisplay->repaint();
    }
    else if (taskName == "ParseSourceList")
    {
        databaseProgress->setLabel(fmt::format("Importing sources [{:d}%]", (int)(pTask->progress() * 100.f)));
        databaseProgress->resizeToFit();
        databaseProgress->setVisible(true);
    }
    else if (taskName == "ParseTagsList")
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
    // std::cout << "WAIVESamplerUI::onTaskFinished ";
    Poco::Task *pTask = pNf->task();
    if (!pTask)
    {
        std::cerr << "Error: pTask is null" << std::endl;
        return;
    }

    const std::string &taskName = pTask->name();
    std::cout << taskName << std::endl;

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
        // databaseProgress->hide();
    }
    else if (taskName == "ParseTagsList")
    {
        // databaseProgress->hide();
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
        loadingTaskCount++;
        // databaseLoading->setLoading(true);
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR:
    case SampleDatabase::DatabaseUpdate::TAG_LIST_DOWNLOAD_ERROR:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADED:
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOAD_FAILED:
        loadingTaskCount--;
        // databaseLoading->setLoading(false);
        break;
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOADED:
        sourceLoad(plugin->sd.latestDownloadedIndex);
        loadingTaskCount--;
        // databaseLoading->setLoading(false);
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_UPDATED:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_FILTER_END:
        // databaseLoading->setLoading(false);
        loadingTaskCount--;
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_ANALYSED:
        for (size_t i = 0; i < plugin->sd.archives.size(); i++)
            archiveList->addItem(plugin->sd.archives.at(i).c_str());

        archiveList->calculateHeight();
        archiveList->calculateWidth();
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_READY:
    case SampleDatabase::DatabaseUpdate::SOURCE_PREVIEW_READY:
        break;
    default:
        break;
    }

    if (loadingTaskCount > 0)
        databaseLoading->setLoading(true);
    else
        databaseLoading->setLoading(false);

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
        errorMessage = false;
        databaseProgress->setLabel("");
        break;
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOADED:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_UPDATED:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_READY:
        if (!errorMessage)
            databaseProgress->setLabel("");
        break;
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOAD_FAILED:
    case SampleDatabase::DatabaseUpdate::TAG_LIST_DOWNLOAD_ERROR:
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR:
        errorMessage = true;
        databaseProgress->setLabel("Error downloading.");
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_PREVIEW_READY:
        previewPlaybackBtn->setVisible(true);
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
    knob->gauge_width = 1.0f * fScaleFactor;
    knob->accent_color = WaiveColors::text;
    knob->foreground_color = WaiveColors::light1;
    knob->setCallback(this);
    knob->setFontSize(14.f);
    knob->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    knob->setRadius(12.f);
    knob->resizeToFit();

    return knob;
}

END_NAMESPACE_DISTRHO