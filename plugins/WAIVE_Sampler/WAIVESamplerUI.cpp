#include "WAIVESamplerUI.hpp"
START_NAMESPACE_DISTRHO

WAIVESamplerUI::WAIVESamplerUI() : UI(UI_W, UI_H),
                                   fScaleFactor(getScaleFactor()),
                                   fScale(1.0f),
                                   filebrowserOpen(false),
                                   errorMessage(false),
                                   loadingTaskCount(0)
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

    dragDropManager = new DragDropManager(&getWindow());
    std::cout << "WAIVESamplerUI::WAIVESamplerUI() dragDropManager->isDragging() " << dragDropManager->isDragging() << std::endl;

    // 1 ----- Source Browser Panel
    {
        sourceBrowserPanel = new Panel(this);
        sourceBrowserPanel->setSize(col1Width, 254.f);
        sourceBrowserPanel->setAbsolutePos(padding, padding);
        sourceBrowserPanel->setFont("VG5000", VG5000, VG5000_len);
        sourceBrowserPanel->label = "1";
        sourceBrowserPanel->title = "Source";
        sourceBrowserPanel->expandable = true;

        DGL::Rectangle<float> sourceTitleBounds;
        sourceBrowserPanel->getTitlAbsoluteBounds(sourceTitleBounds);

        sourceList = new SourceList(this);
        sourceList->setSize(539.f - 24.f - 24.f, 150.f);
        sourceList->onTop(sourceBrowserPanel, CENTER, START, 52.f * fScaleFactor);
        sourceList->source_info = &(plugin->sd.sourcesList);
        sourceList->source_info_mtx = &(plugin->sd.sourceListMutex);
        sourceList->scrollHandle = WaiveColors::light2;
        sourceList->scrollGutter = WaiveColors::light1;
        sourceList->background_color = WaiveColors::grey2;
        sourceList->accent_color = sourceBrowserPanel->background_color;
        sourceList->padding = 0.0f;
        sourceList->margin = 0.0f;
        sourceList->setFontSize(16.f);
        sourceList->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        sourceList->computeColumnWidths();
        sourceList->setCallback(this);
        sourceList->description = "Select sound source.";
        sourceBrowserPanel->addChildWidget(sourceList);

        importSource = new Button(this);
        importSource->setLabel("Import");
        importSource->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        importSource->setFontSize(14.f);
        importSource->resizeToFit();
        importSource->setRight(sourceList->getRight());
        importSource->setCenterY(sourceTitleBounds.getY() + sourceTitleBounds.getHeight() * 0.5f);
        importSource->setCallback(this);
        importSource->description = "Import source audio file.";
        sourceBrowserPanel->addChildWidget(importSource);

        previewPlaybackBtn = new Button(this);
        previewPlaybackBtn->setLabel("Stop");
        previewPlaybackBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        previewPlaybackBtn->setFontSize(14.f);
        previewPlaybackBtn->resizeToFit();
        previewPlaybackBtn->leftOf(importSource, END, padding);
        previewPlaybackBtn->setCallback(this);
        previewPlaybackBtn->description = "Stop source preview.";
        previewPlaybackBtn->setVisible(false);
        sourceBrowserPanel->addChildWidget(previewPlaybackBtn);

        openFilterPanelBtn = new Button(this);
        openFilterPanelBtn->isToggle = true;
        openFilterPanelBtn->setLabel("Filter");
        openFilterPanelBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        openFilterPanelBtn->setFontSize(14.f);
        openFilterPanelBtn->resizeToFit();
        float spacing = Layout::measureVertical(sourceList, Widget_Align::END, sourceBrowserPanel, Widget_Align::END);
        spacing = (spacing - openFilterPanelBtn->getHeight()) / 2.f;
        openFilterPanelBtn->below(sourceList, START, spacing);
        openFilterPanelBtn->setCallback(this);
        openFilterPanelBtn->description = "Show/Hide additional filter options for searching sources.";
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
        sourceSearch->foreground_color = WaiveColors::grey2;
        sourceSearch->accent_color = WaiveColors::grey2;
        sourceSearch->background_color = searchBox->background_color;
        sourceSearch->setSize(searchBox->getWidth() - padding * 4.f - sourceSearch->getFontSize(), sourceSearch->getFontSize(), true);
        sourceSearch->onTop(searchBox, START, CENTER, padding * 2.f);
        sourceSearch->setCallback(this);
        sourceSearch->description = "Search archives by keywords.";
        sourceBrowserPanel->addChildWidget(sourceSearch);

        searchIcon = new Icon(this);
        searchIcon->setImageData(search, search_len, 85, 85, IMAGE_GENERATE_MIPMAPS);
        searchIcon->setSize(sourceSearch->getFontSize() - 4.f, sourceSearch->getFontSize() - 4.f, true);
        searchIcon->onTop(searchBox, END, CENTER, searchBox->radius);
        sourceBrowserPanel->addChildWidget(searchIcon);

        archiveList = new RadioButtons(this);
        archiveList->setFontSize(18.0f);
        archiveList->radioSize = 4.f * fScaleFactor;
        archiveList->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        archiveList->addItem("All");
        archiveList->setItem(0, false);
        archiveList->calculateWidth();
        archiveList->calculateHeight();
        archiveList->setCallback(this);

        databaseLoading = new Spinner(this);
        databaseLoading->setSize(sourceSearch->getHeight(), sourceSearch->getHeight(), true);
        databaseLoading->setLeft(sourceTitleBounds.getX() + sourceTitleBounds.getWidth() + padding);
        databaseLoading->setCenterY(sourceTitleBounds.getY() + sourceTitleBounds.getHeight() * 0.5f);
        sourceBrowserPanel->addChildWidget(databaseLoading);

        databaseProgress = new Label(this, "");
        databaseProgress->rightOf(databaseLoading, START);
        databaseProgress->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        sourceBrowserPanel->addChildWidget(databaseProgress);

        randomSourceBtn = new Button(this);
        randomSourceBtn->setLabel("Random");
        randomSourceBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        randomSourceBtn->setFontSize(14.f);
        randomSourceBtn->resizeToFit();
        randomSourceBtn->below(sourceList, Widget_Align::END, spacing);
        randomSourceBtn->setCallback(this);
        randomSourceBtn->description = "Randomly select a source file.";
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
        presetLabel->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        presetLabel->resizeToFit();
        sampleEditorPanel->addChildWidget(presetLabel);

        knobsLabel = new Label(this, "Adjust");
        knobsLabel->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        knobsLabel->resizeToFit();
        sampleEditorPanel->addChildWidget(knobsLabel);

        makeKick = new Button(this);
        makeKick->setLabel("Kick");
        makeKick->isToggle = true;
        makeKick->setFontSize(16.f);
        makeKick->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        makeKick->setSize(100.f, 32.f);
        makeKick->description = "Find a Kick sound.";
        makeKick->setCallback(this);

        makeSnare = new Button(this);
        makeSnare->setLabel("Snare");
        makeSnare->isToggle = true;
        makeSnare->setFontSize(16.f);
        makeSnare->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        makeSnare->setSize(100.f, 32.f);
        makeSnare->description = "Find a Snare sound.";
        makeSnare->setCallback(this);

        makeHihat = new Button(this);
        makeHihat->setLabel("Hi-Hat");
        makeHihat->isToggle = true;
        makeHihat->setFontSize(16.f);
        makeHihat->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        makeHihat->setSize(100.f, 32.f);
        makeHihat->description = "Find a Hi-Hat sound.";
        makeHihat->setCallback(this);

        makeClap = new Button(this);
        makeClap->setLabel("Clap");
        makeClap->isToggle = true;
        makeClap->setFontSize(16.f);
        makeClap->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        makeClap->setSize(100.f, 32.f);
        makeClap->description = "Find a Clap sound.";
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
        sampleEditorPanel->addChildWidget(presetButtons);

        sustainLength = createWAIVEKnob(kSustainLength, "Length", 0.0f, 5000.0f, 200.f);
        sustainLength->description = "Set sustain length of sample.";
        sustainLength->format = "{:.0f}ms";
        sustainLength->vertical = false;
        sustainLength->resizeToFit();
        sustainLength->onTop(sampleEditorPanel, Widget_Align::END, Widget_Align::START, 24.f, 16.f);
        sampleEditorPanel->addChildWidget(sustainLength);
        allKnobs.push_back(sustainLength);

        pitch = createWAIVEKnob(kSamplePitch, "Pitch", 0.25f, 4.f, 1.0f);
        pitch->description = "Set sample playback speed.";
        volume = createWAIVEKnob(kSampleVolume, "Volume", 0.0f, 2.0f, 1.0f);
        volume->description = "Set volume of sample.";
        percussionBoost = createWAIVEKnob(kPercussiveBoost, "Hit", 0.0f, 2.0f, 0.5f);
        percussionBoost->description = "Set amount of percussive hit.";

        ampAttack = createWAIVEKnob(kAmpAttack, "Attack", 0.0f, 500.0f, 10.0f);
        ampAttack->format = "{:.0f}ms";
        ampAttack->description = "Set attack time.";
        ampDecay = createWAIVEKnob(kAmpDecay, "Decay", 0.0f, 500.0f, 50.0f);
        ampDecay->format = "{:.0f}ms";
        ampDecay->description = "Set decay time.";
        ampSustain = createWAIVEKnob(kAmpSustain, "Sustain", 0.0f, 1.0f, 0.7f);
        ampSustain->description = "Set sustain level.";
        ampRelease = createWAIVEKnob(kAmpRelease, "Release", 0.0f, 500.0f, 100.0f);
        ampRelease->format = "{:.0f}ms";
        ampRelease->description = "Set release time.";

        filterCutoff = createWAIVEKnob(kFilterCutoff, "Cutoff", 0.0, 0.999, 0.999);
        filterCutoff->description = "Set filter cutoff point.";
        filterResonance = createWAIVEKnob(kFilterResonance, "Res.", 0.0, 1.0, 0.0);
        filterResonance->description = "Set filter resonance.";

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
        filterType->description = "Set filter type.";
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
        editorKnobs->addWidget(sustainLength);
        sampleEditorPanel->addChildWidget(editorKnobs);

        editorKnobs->setVisible(false);

        instructions = new Label(this, "Load a source or import your own audio to extract samples from.");
        instructions->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        instructions->resizeToFit();
        instructions->below(sampleWaveformDisplay, CENTER, Layout::measureVertical(sampleWaveformDisplay, END, sampleEditorPanel, END) / 2.f);
        sampleEditorPanel->addChildWidget(instructions);

        saveSampleBtn = new Button(this);
        saveSampleBtn->setLabel("Add to player");
        saveSampleBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        saveSampleBtn->setFontSize(14.f);
        saveSampleBtn->resizeToFit();
        saveSampleBtn->setCallback(this);
        saveSampleBtn->setEnabled(false);
        saveSampleBtn->description = "Save sample and add to Sample Player.";
        sampleEditorPanel->addChildWidget(saveSampleBtn);

        playSampleBtn = new Button(this);
        playSampleBtn->setLabel(" ▶");
        playSampleBtn->setFontSize(14.f);
        playSampleBtn->background_color = WaiveColors::light2;
        playSampleBtn->text_color = WaiveColors::dark;
        playSampleBtn->resizeToFit();
        playSampleBtn->description = "Preview sample.";
        playSampleBtn->setCallback(this);
        sampleEditorPanel->addChildWidget(playSampleBtn);

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
        // newSampleBtn = new Button(this);
    }

    // 3 ----- Sample Player Panel
    {
        samplePlayerPanel = new Panel(this);
        samplePlayerPanel->setSize(col2Width * fScaleFactor, Layout::measureVertical(sourceBrowserPanel, Widget_Align::START, sampleEditorPanel, Widget_Align::END), true);
        samplePlayerPanel->setAbsolutePos(width - padding - col2Width * fScaleFactor, padding);
        samplePlayerPanel->setFont("VG5000", VG5000, VG5000_len);
        samplePlayerPanel->label = "3";
        samplePlayerPanel->title = "Player";
        samplePlayerPanel->expandable = true;
        samplePlayerPanel->expand_right = false;
        samplePlayerPanel->expand_h = width - padding - padding;
        samplePlayerPanel->expand_v = samplePlayerPanel->getHeight();

        DGL::Rectangle<float> samplePlayerTitleBounds;
        samplePlayerPanel->getTitlAbsoluteBounds(samplePlayerTitleBounds);

        openMapBtn = new Button(this);
        openMapBtn->setLabel("Sample Map");
        openMapBtn->isToggle = true;
        openMapBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        openMapBtn->setFontSize(14.f);
        openMapBtn->resizeToFit();
        openMapBtn->setCallback(this);
        openMapBtn->description = "Show/Hide Sample Map.";
        samplePlayerPanel->addChildWidget(openMapBtn);

        browseFilesBtn = new Button(this);
        browseFilesBtn->setLabel("View Folder");
        browseFilesBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        browseFilesBtn->setFontSize(14.f);
        browseFilesBtn->resizeToFit();
        browseFilesBtn->setCallback(this);
        browseFilesBtn->description = "Open samples folder in your system's file explorer.";
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
            SampleSlot *slot = new SampleSlot(this, dragDropManager);
            slot->setSamplePlayer(&plugin->samplePlayers[i]);
            slot->accent_color = WaiveColors::light2;
            slot->setCallback(this);
            slot->slotId = i;
            slot->setSize(sampleSlotsContainer->getWidth(), sampleSlotsContainer->getHeight() / NUM_SLOTS - 4, true);
            slot->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
            slot->setFontSize(12.f);
            slot->currentSample = plugin->fCurrentSample;

            addIdleCallback(slot);

            sampleSlotsContainer->addWidget(slot);
            sampleSlots.push_back(slot);
        }

        sampleSlotsContainer->positionWidgets();
        for (int i = 0; i < sampleSlots.size(); i++)
            sampleSlots[i]->repositionWidgets();

        oscControlsBtn = new Button(this);
        oscControlsBtn->setLabel("OSC");
        oscControlsBtn->isToggle = true;
        oscControlsBtn->description = "Edit OSC options";
        oscControlsBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        oscControlsBtn->setFontSize(14.f);
        oscControlsBtn->resizeToFit();
        oscControlsBtn->setRight(sampleSlotsContainer->getRight());
        oscControlsBtn->setCenterY(samplePlayerTitleBounds.getY() + samplePlayerTitleBounds.getHeight() * 0.5f);
        oscControlsBtn->setCallback(this);
        samplePlayerPanel->addChildWidget(oscControlsBtn);
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
        sampleBrowser = new SampleBrowser(samplePlayerPanel, &plugin->sd, dragDropManager);
        sampleBrowser->setSize(sourceList->getWidth(), tagBrowser->getBottom() - sourceList->getTop());
        sampleBrowser->setAbsolutePos(sourceList->getLeft(), sourceList->getTop());
        sampleBrowser->repositionWidgets();
        sampleBrowser->setCallback(this);
        samplePlayerPanel->hiddenWidgets.addChildWidget(sampleBrowser);
        samplePlayerPanel->hiddenWidgets.hide();
    }

    // 6 ----- OSC Options

    {
        oscControlsPanel = new Panel(this);
        oscControlsPanel->setSize(sampleSlotsContainer->getWidth(), sampleSlotsContainer->getHeight(), true);
        oscControlsPanel->setAbsolutePos(sampleSlotsContainer->getAbsoluteX(), sampleSlotsContainer->getAbsoluteY());
        oscControlsPanel->setFont("VG5000", VG5000, VG5000_len);
        oscControlsPanel->title = "OSC Options";

        oscControlsDescription = new Label(this, "Send an OSC message every time a sample is triggered.\n\nMessage format:\n/WAIVE_Sampler/Sample  [name, tags, midi]");
        oscControlsDescription->setWidth(oscControlsPanel->getWidth() - 2 * padding);
        oscControlsDescription->setFontSize(14.f);
        oscControlsDescription->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        oscControlsDescription->calculateHeight();
        oscControlsDescription->onTop(oscControlsPanel, START, START, padding);
        oscControlsPanel->addChildWidget(oscControlsDescription);

        float hostW = oscControlsDescription->getWidth() * 0.6f;
        float portW = oscControlsDescription->getWidth() - hostW - padding;
        oscHostInput = new TextInput(this);
        oscHostInput->setFontSize(16.0f);
        oscHostInput->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
        oscHostInput->foreground_color = WaiveColors::light1;
        oscHostInput->accent_color = WaiveColors::text;
        oscHostInput->setText(plugin->oscHost.c_str());
        oscHostInput->setSize(hostW, oscHostInput->getFontSize() + 6, true);
        oscHostInput->setCallback(this);
        oscHostInput->description = "Set OSC host address.";
        oscHostInput->below(oscControlsDescription, START, padding);
        oscControlsPanel->addChildWidget(oscHostInput);

        oscPortInput = new TextInput(this);
        oscPortInput->textType = TextInput::TextType::INTEGER;
        oscPortInput->setFontSize(16.0f);
        oscPortInput->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
        oscPortInput->foreground_color = WaiveColors::light1;
        oscPortInput->accent_color = WaiveColors::text;
        oscPortInput->setText(fmt::format("{:d}", plugin->oscPort).c_str());
        oscPortInput->setSize(portW, oscPortInput->getFontSize() + 6, true);
        oscPortInput->setCallback(this);
        oscPortInput->description = "Set OSC port number.";
        oscPortInput->rightOf(oscHostInput, START, padding);
        oscControlsPanel->addChildWidget(oscPortInput);

        oscEnableBtn = new Button(this);
        oscEnableBtn->setLabel("Enable");
        oscEnableBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
        oscEnableBtn->setFontSize(14.f);
        oscEnableBtn->resizeToFit();
        oscEnableBtn->description = "Enable sending OSC messages";
        oscEnableBtn->isToggle = true;
        oscEnableBtn->setToggled(plugin->getSendOSC());
        oscEnableBtn->below(oscPortInput, CENTER, padding);
        oscEnableBtn->setCenterX(oscControlsPanel->getCenterX());
        oscEnableBtn->setCallback(this);
        oscControlsPanel->addChildWidget(oscEnableBtn);

        oscControlsPanel->setVisible(false);
    }

    // ----------

    toolTip = new Label(this);
    toolTip->setFontSize(18.f);
    toolTip->text_color = WaiveColors::light2;
    toolTip->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
    toolTip->setHeight(toolTip->getFontSize());
    toolTip->setWidth((col1Width + col2Width) * fScaleFactor);
    toolTip->setLeft(sampleEditorPanel->getLeft());
    toolTip->setTop(sampleEditorPanel->getBottom() + 10.f);

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

    std::cout << " - WAIVESamplerUI::~WAIVESamplerUI() DONE" << std::endl;
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
        sampleSlots[slot]->setMidiNumber(value + 1, false);
        break;
    default:
        break;
    }

    repaint();
}

void WAIVESamplerUI::stateChanged(const char *key, const char *value)
{
    std::cout << "WAIVESamplerUI::stateChanged: " << key << " -> " << value << std::endl;

    if (std::strcmp(key, "osc-address") == 0)
    {
    }

    repaint();
}

void WAIVESamplerUI::knobDragStarted(Knob *knob) {}

void WAIVESamplerUI::knobDragFinished(Knob *knob, float value)
{
    plugin->triggerPreview();
}

void WAIVESamplerUI::knobValueChanged(Knob *knob, float value)
{
    setParameterValue(knob->getId(), value);
}

std::string getDescription(std::string text, DGL::SubWidget *widget, const DGL::Widget::MotionEvent &ev)
{
    if (auto waiveWidget = dynamic_cast<WAIVEWidget *>(widget))
    {
        if (waiveWidget->isVisible() && waiveWidget->getAbsoluteArea().contains(ev.pos) && waiveWidget->description.length() > 0)
            text = waiveWidget->description;
    }

    std::list<DGL::SubWidget *> subWidgets = widget->getChildren();
    for (const auto subWidget : subWidgets)
        text = getDescription(text, subWidget, ev);

    return text;
};

bool WAIVESamplerUI::onMotion(const MotionEvent &ev)
{
    std::list<DGL::SubWidget *> children = getChildren();
    std::string text = "";
    for (const auto &widget : children)
        text = getDescription(text, widget, ev);
    toolTip->setLabel(text);

    if (dragDropManager->isDragging())
        getWindow().setCursor(kMouseCursorHand);

    return UI::onMotion(ev);
}

bool WAIVESamplerUI::onMouse(const MouseEvent &ev)
{
    bool result = UI::onMouse(ev);

    if (!ev.press)
    {
        dragDropManager->clearEvent();

        // bypass other widgets consuption of mouseUp event for all knobs
        for (const auto &knob : allKnobs)
            knob->onMouse(ev);
    }

    return result;
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
        plugin->stopSourcePreview();
    // else if (button == newSampleBtn)
    //     plugin->newSample();
    else if (button == makeKick)
    {
        makeKick->setToggled(true);
        makeSnare->setToggled(false);
        makeClap->setToggled(false);
        makeHihat->setToggled(false);

        if (plugin->fCurrentSample == nullptr)
            return;
        plugin->newSample();

        Source *sourceInfo = &plugin->fCurrentSample->sourceInfo;

        // 0. Check if a source is loaded
        if (!sourceInfo->sourceLoaded)
            return;

        // 1. Select random candidate area of source
        int nCandidates = sourceInfo->sourceFeatures.size();
        if (nCandidates == 0)
            return; // or pick a random spot?

        sampleWaveformDisplay->text_color = WaiveColors::accent2;
        int i = random.next() % nCandidates;
        int startIndex = sourceInfo->sourceFeatures[i].start;
        plugin->selectWaveform(&sourceInfo->buffer, startIndex);
        plugin->fCurrentSample->sourceStart = startIndex;

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::KickPreset);

        // 3. Set sample name
        // sampleName->setText(plugin->sd.getNewSampleName("kick.wav").c_str(), true);
        plugin->generateCurrentSampleName("kick");

        // 4. Trigger preview
        plugin->triggerPreview();
    }
    else if (button == makeSnare)
    {
        makeKick->setToggled(false);
        makeSnare->setToggled(true);
        makeClap->setToggled(false);
        makeHihat->setToggled(false);

        // 0. Check if a source is loaded
        if (plugin->fCurrentSample == nullptr)
            return;

        plugin->newSample();
        Source *sourceInfo = &plugin->fCurrentSample->sourceInfo;

        // 0. Check if a source is loaded
        if (!sourceInfo->sourceLoaded)
            return;

        // 1. pick random start
        std::vector<long> starts;

        for (const auto &m : sourceInfo->sourceMeasurements)
        {
            if (m.rms > 0.1 && m.specFlat > 0.9f)
                starts.push_back(m.frame);
        }

        if (starts.size() == 0)
            return;

        sampleWaveformDisplay->text_color = WaiveColors::accent1;

        int i = random.next() % starts.size();
        plugin->selectWaveform(&sourceInfo->buffer, starts[i]);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::SnarePreset);

        // 3. Set sample name
        // std::cout << plugin->fSourceTagString << std::endl;
        // sampleName->setText(plugin->sd.getNewSampleName("snare.wav").c_str(), true);
        plugin->generateCurrentSampleName("snare");

        // 4. Trigger preview
        plugin->triggerPreview();
    }
    else if (button == makeHihat)
    {
        makeKick->setToggled(false);
        makeSnare->setToggled(false);
        makeClap->setToggled(false);
        makeHihat->setToggled(true);

        if (plugin->fCurrentSample == nullptr)
            return;
        plugin->newSample();
        Source *sourceInfo = &plugin->fCurrentSample->sourceInfo;

        // 0. Check if a source is loaded
        if (!sourceInfo->sourceLoaded)
            return;

        // 1. pick random start
        std::vector<long> starts;

        for (const auto &m : sourceInfo->sourceMeasurements)
        {
            if (m.rms > 0.1 && m.specFlat > 0.9f)
                starts.push_back(m.frame);
        }

        if (starts.size() == 0)
            return;

        sampleWaveformDisplay->text_color = WaiveColors::accent3;

        int i = random.next() % starts.size();
        plugin->selectWaveform(&sourceInfo->buffer, starts[i]);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::HiHat);

        // 3. Set sample name
        // sampleName->setText(plugin->sd.getNewSampleName("hihat.wav").c_str(), true);
        plugin->generateCurrentSampleName("hihat");

        // 4. Trigger preview
        plugin->triggerPreview();
    }
    else if (button == makeClap)
    {
        makeKick->setToggled(false);
        makeSnare->setToggled(false);
        makeClap->setToggled(true);
        makeHihat->setToggled(false);

        if (plugin->fCurrentSample == nullptr)
            return;
        plugin->newSample();
        Source *sourceInfo = &plugin->fCurrentSample->sourceInfo;

        // 0. Check if a source is loaded
        if (!sourceInfo->sourceLoaded)
            return;

        // 1. pick random start
        std::vector<long> starts;

        for (const auto &m : sourceInfo->sourceMeasurements)
        {
            if (m.rms > 0.1 && m.specFlat > 0.9f)
                starts.push_back(m.frame);
        }

        if (starts.size() == 0)
            return;

        sampleWaveformDisplay->text_color = WaiveColors::accent4;

        int i = random.next() % starts.size();
        plugin->selectWaveform(&sourceInfo->buffer, starts[i]);

        // 2. Load preset parameter values
        plugin->loadPreset(Presets::Clap);

        // 3. Set sample name
        plugin->generateCurrentSampleName("clap");

        // 4. Trigger preview
        plugin->triggerPreview();
    }
    else if (button == browseFilesBtn)
        SystemOpenDirectory(plugin->sd.getSampleFolder());
    else if (button == openMapBtn)
    {
        if (button->getToggled())
            samplePlayerPanel->expand();
        else
            samplePlayerPanel->collapse();
        oscControlsBtn->setToggled(oscControlsBtn->getToggled(), true);
    }
    else if (button == openFilterPanelBtn)
    {
        if (button->getToggled())
            sourceBrowserPanel->expand();
        else
            sourceBrowserPanel->collapse();
    }
    else if (button == oscControlsBtn)
    {
        oscControlsPanel->toFront();
        oscControlsPanel->setVisible(oscControlsBtn->getToggled());
    }
    else if (button == oscEnableBtn)
        plugin->setSendOSC(oscEnableBtn->getToggled());
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
    // plugin->selectWaveform(&plugin->fSourceWaveform, static_cast<int>(selectionStart));
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

void WAIVESamplerUI::mapSampleDelete(int id)
{
    plugin->deleteSample(id);
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
    else if (textInput == oscHostInput)
    {
        if (text.length() == 0)
        {
            textInput->undo();
            return;
        }

        if (!plugin->setOSCAddress(text, plugin->oscPort))
            textInput->undo();

        return;
    }
    else if (textInput == oscPortInput)
    {
        if (text.length() == 0)
        {
            textInput->undo();
            return;
        }

        if (!plugin->setOSCAddress(plugin->oscHost, std::stoi(text)))
            textInput->undo();

        return;
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
};

void WAIVESamplerUI::sampleSlotCleared(SampleSlot *slot, int slotId)
{
    plugin->clearSamplePlayer(*sampleSlots[slotId]->getSamplePlayer());
    plugin->clear();
    sourceList->selected = -1;
};

void WAIVESamplerUI::sampleSlotLoadSample(SampleSlot *slot, int slotId, int sampleId)
{
    plugin->loadSlot(slotId, sampleId);
}

void WAIVESamplerUI::sourceDownload(int index)
{
    plugin->sd.downloadSourceFile(index);
}

void WAIVESamplerUI::sourceLoad(int index)
{
    plugin->loadSource(index);
}

void WAIVESamplerUI::sourcePreview(int index, bool start)
{
    if (start)
        plugin->sd.playTempSourceFile(index);
    else
        plugin->stopSourcePreview();
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
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_CENTER);
    fillColor(WaiveColors::grey2);
    fontSize(18.f * fScaleFactor);
    fontFaceId(fontTitle);
    text(width / 2.f, middle, "waive sampler", nullptr);
    closePath();

    beginPath();
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_RIGHT);
    fontFaceId(fontMain);
    fontSize(12.f * fScaleFactor);
    text(width - 10.f, middle, fmt::format("v{:d}.{:d}.{:d}", V_MAJ, V_MIN, V_PAT).c_str(), nullptr);
    closePath();
}

void WAIVESamplerUI::uiScaleFactorChanged(const double scaleFactor)
{
    fScaleFactor = scaleFactor;
}

void WAIVESamplerUI::updateWidgets()
{
    std::cout << "WAIVESamplerUI::updateWidgets()\n";
    bool sourceAvailable = (plugin->fCurrentSample != nullptr) && (plugin->fCurrentSample->sourceInfo.sourceLoaded);
    std::cout << "sourceAvaliable: " << sourceAvailable << std::endl;
    saveSampleBtn->setEnabled(sourceAvailable);
    sourceLoading->setLoading(false);
    presetButtons->setVisible(sourceAvailable);
    editorKnobs->setVisible(sourceAvailable);
    instructions->setVisible(!sourceAvailable);

    if (plugin->fCurrentSample != nullptr)
    {
        sampleWaveformDisplay->setWaveformLength(plugin->fCurrentSample->sampleLength);
        sampleWaveformDisplay->waveformUpdated();
        if (plugin->fCurrentSample->saved)
            saveSampleBtn->setLabel("Update");
        else
            saveSampleBtn->setLabel("Add to player");
        saveSampleBtn->setEnabled(true);
    }
    else
    {
        saveSampleBtn->setLabel("Add to player");
    }
}

void WAIVESamplerUI::onPluginUpdated(const void *pSender, const WAIVESampler::PluginUpdate &arg)
{
    std::cout << "WAIVESamplerUI::onPluginUpdated " << plugin->pluginUpdateToString(arg) << std::endl;
    bool sourceAvailable;
    switch (arg)
    {
    case WAIVESampler::PluginUpdate::kSourceLoading:
        break;
    case WAIVESampler::PluginUpdate::kSourceLoaded:
    case WAIVESampler::PluginUpdate::kSourceUpdated:
        sourceAvailable = (plugin->fCurrentSample != nullptr) && (plugin->fCurrentSample->sourceInfo.sourceLoaded);
        std::cout << "sourceAvaliable: " << sourceAvailable << std::endl;
        saveSampleBtn->setEnabled(sourceAvailable);
        sourceLoading->setLoading(false);
        presetButtons->setVisible(sourceAvailable);
        editorKnobs->setVisible(sourceAvailable);
        instructions->setVisible(!sourceAvailable);
        updatePresetButtons();
        break;
    case WAIVESampler::PluginUpdate::kSourcePreviewPlay:
        previewPlaybackBtn->show();
        break;
    case WAIVESampler::PluginUpdate::kSourcePreviewStop:
        previewPlaybackBtn->hide();
        sourceList->previewPlaying = -1;
        break;
    case WAIVESampler::PluginUpdate::kSampleLoading:
        break;
    case WAIVESampler::PluginUpdate::kSampleLoaded:
        sourceList->repaint();
        for (int i = 0; i < sampleSlots.size(); i++)
        {
            sampleSlots[i]->currentSample = plugin->fCurrentSample;
            sampleSlots[i]->repaint();
        }

        if (plugin->fCurrentSample == nullptr)
        {
            updatePresetButtons();
            sampleWaveformDisplay->setWaveform(nullptr);
            sampleWaveformDisplay->setWaveformLength(0);
            sampleWaveformDisplay->waveformNew();
            playSampleBtn->setEnabled(false);
            sourceLoading->setLoading(false);
            presetButtons->setVisible(false);
            editorKnobs->setVisible(false);
            instructions->setVisible(true);
            saveSampleBtn->setLabel("Add to player");
        }
        else
        {
            updatePresetButtons();
            sampleWaveformDisplay->setWaveformLength(plugin->fCurrentSample->sampleLength);
            if (plugin->fCurrentSample->saved)
                saveSampleBtn->setLabel("Update");
            else
                saveSampleBtn->setLabel("Add to player");
            saveSampleBtn->setEnabled(true);
            sampleWaveformDisplay->waveformNew();
            playSampleBtn->setEnabled(true);
            presetButtons->setVisible(true);
            editorKnobs->setVisible(true);
            instructions->setVisible(false);
        }
        break;
    case WAIVESampler::PluginUpdate::kSampleUpdated:
        if (plugin->fCurrentSample != nullptr)
        {
            sampleWaveformDisplay->setWaveform(plugin->editorPreviewWaveform);
            sampleWaveformDisplay->setWaveformLength(plugin->fCurrentSample->sampleLength);
            sampleWaveformDisplay->waveformUpdated();
            if (plugin->fCurrentSample->saved)
                saveSampleBtn->setLabel("Update");
            else
                saveSampleBtn->setLabel("Add to player");
            playSampleBtn->setEnabled(true);
        }
        else
        {
            std::cout << "plugin->fCurrentSample == nullptr" << std::endl;
            sampleWaveformDisplay->setWaveformLength(0);
            sampleWaveformDisplay->waveformNew();
            saveSampleBtn->setLabel("Add to player");
            saveSampleBtn->setEnabled(false);

            playSampleBtn->setEnabled(false);
        }
        break;
    case WAIVESampler::PluginUpdate::kSampleAdded:
        break;
    case WAIVESampler::PluginUpdate::kParametersChanged:
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
                saveSampleBtn->setLabel("Add to player");
            // sampleName->setText(plugin->fCurrentSample->name.c_str(), false);
        }
        else
        {
            saveSampleBtn->setLabel("Add to player");
        }

        break;
    case WAIVESampler::PluginUpdate::kSlotLoaded:
        for (int i = 0; i < sampleSlots.size(); i++)
        {
            sampleSlots[i]->currentSample = plugin->fCurrentSample;
            sampleSlots[i]->repaint();
        }
        break;
    case WAIVESampler::PluginUpdate::kSampleCleared:
        sampleWaveformDisplay->setWaveform(nullptr);
        sampleWaveformDisplay->setWaveformLength(0);
        sampleWaveformDisplay->waveformNew();
        saveSampleBtn->setEnabled(false);
        saveSampleBtn->setLabel("Add to player");
        sourceLoading->setLoading(false);
        presetButtons->setVisible(false);
        editorKnobs->setVisible(false);
        instructions->setVisible(true);
    default:
        std::cout << "Unknown update: " << arg << std::endl;
        break;
    }

    repaint();
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
    else if (taskName == "loadSourceBuffer")
    {
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
        progress->setLabel(fmt::format("Analysing...[{:d}%]", static_cast<int>(pTask->progress() * 100.f)));
        progress->resizeToFit();
        progress->setVisible(true);
        // sourceWaveformDisplay->repaint();
    }
    else if (taskName == "WaveformLoaderTask")
    {
        progress->setLabel(fmt::format("Importing...[{:d}%]", static_cast<int>(pTask->progress() * 100.f)));
        progress->resizeToFit();
        progress->setVisible(true);
        // sourceWaveformDisplay->repaint();
    }
    else if (taskName == "ParseSourceList")
    {
        databaseProgress->setLabel(fmt::format("Importing sources [{:d}%]", static_cast<int>(pTask->progress() * 100.f)));
        databaseProgress->resizeToFit();
        databaseProgress->setVisible(true);
    }
    else if (taskName == "ParseTagsList")
    {
        databaseProgress->setLabel(fmt::format("Importing tags [{:d}%]", static_cast<int>(pTask->progress() * 100.f)));
        databaseProgress->resizeToFit();
        databaseProgress->setVisible(true);
    }
    pNf->release();
}

void WAIVESamplerUI::onTaskCancelled(Poco::TaskCancelledNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    if (!pTask)
        return;
    std::cout << "WAIVESamplerUI::onTaskCancelled " << pTask->name() << std::endl;
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
    std::cout << "WAIVESamplerUI::onTaskFinished " << taskName << std::endl;

    if (taskName == "ImporterTask")
        sampleBrowser->loading->setLoading(false);
    else if (taskName == "loadSourceBuffer")
    {
        sourceLoading->setLoading(false);
        progress->setVisible(false);
        sourceLoading->repaint();
        progress->repaint();
    }
    else if (taskName == "FeatureExtractorTask")
    {
        sourceLoading->setLoading(false);
        progress->setVisible(false);
        sourceLoading->repaint();
        progress->repaint();
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
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_CHECKING_UPDATE:
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
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_CHECKED_UPDATE:
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
    {
        loadingTaskCount = 0;
        databaseLoading->setLoading(false);
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
        databaseProgress->setLabel("Error downloading data.");
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_PREVIEW_READY:
        // previewPlaybackBtn->setVisible(true);
        // break;
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
    const std::string &label,
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

void WAIVESamplerUI::updatePresetButtons()
{
    makeKick->setToggled(false);
    makeSnare->setToggled(false);
    makeClap->setToggled(false);
    makeHihat->setToggled(false);

    if (plugin->fCurrentSample == nullptr)
    {
        sampleWaveformDisplay->text_color = WaiveColors::text;
        return;
    }

    if (plugin->fCurrentSample->preset == "Kick")
    {
        makeKick->setToggled(true);
        sampleWaveformDisplay->text_color = WaiveColors::accent2;
    }
    else if (plugin->fCurrentSample->preset == "Snare")
    {
        makeSnare->setToggled(true);
        sampleWaveformDisplay->text_color = WaiveColors::accent1;
    }
    else if (plugin->fCurrentSample->preset == "Clap")
    {
        makeClap->setToggled(true);
        sampleWaveformDisplay->text_color = WaiveColors::accent4;
    }
    else if (plugin->fCurrentSample->preset == "HiHat")
    {
        makeHihat->setToggled(true);
        sampleWaveformDisplay->text_color = WaiveColors::accent3;
    }
    else
        sampleWaveformDisplay->text_color = WaiveColors::text;
}

END_NAMESPACE_DISTRHO