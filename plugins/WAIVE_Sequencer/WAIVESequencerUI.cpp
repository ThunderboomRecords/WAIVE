#include "WAIVESequencerUI.hpp"

START_NAMESPACE_DISTRHO

WAIVESequencerUI::WAIVESequencerUI() : UI(UI_W, UI_H),
                                       fScaleFactor(getScaleFactor())
{
    plugin = static_cast<WAIVESequencer *>(getPluginInstancePointer());

    float width = UI_W * fScaleFactor;
    float height = UI_H * fScaleFactor;
    float padding = 10.f * fScaleFactor;

    std::cout << "UI_W: " << UI_W << " UI_H: " << UI_H << std::endl;

    fontTitle = createFontFromMemory("VG5000", VG5000, VG5000_len, false);
    fontMain = createFontFromMemory("Poppins Regular", Poppins_Regular, Poppins_Regular_len, false);

    mainPanel = new Panel(this);
    mainPanel->setSize(width - 2 * padding, 531.f * fScaleFactor, true);
    mainPanel->setAbsolutePos(padding, padding);

    // 1 - Score controls
    scoreLabel = new Label(this, "1   Pattern");
    scoreLabel->setFont("VG5000", VG5000, VG5000_len);
    scoreLabel->text_color = WaiveColors::text;
    scoreLabel->resizeToFit();
    scoreLabel->onTop(mainPanel, START, START, 110.f * fScaleFactor, 35.f * fScaleFactor);

    scoreGenreBox = new Box(this);
    scoreGenreBox->background_color = WaiveColors::grey2;
    scoreGenreBox->description = "Select genre for main drum pattern.";

    scoreGenreDD = new DropDown(this);
    scoreGenreDD->setSize(80, scoreGenreDD->getFontSize(), true);
    for (int i = 0; i < NUM_SCORE_GENRES; i++)
        scoreGenreDD->addItem(score_genres[i]);
    scoreGenreDD->setDisplayNumber(8);
    scoreGenreDD->resizeToFit();
    scoreGenreDD->setItem(plugin->getParameterValue(kScoreGenre), false);
    scoreGenreDD->setFontSize(16.0f);
    scoreGenreDD->highlight_color = WaiveColors::grey2;
    scoreGenreDD->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
    scoreGenreDD->menu->setFontSize(16.0f);
    scoreGenreDD->menu->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
    scoreGenreDD->menu->background_color = WaiveColors::grey2;
    scoreGenreDD->setCallback(this);

    scoreGenreBox->setHeight(scoreGenreDD->getFontSize() * 2.f);
    scoreGenreBox->setWidth(scoreGenreDD->getWidth() + scoreGenreBox->getHeight() * 1.5f);
    scoreGenreBox->radius = scoreGenreBox->getHeight() / 2;
    scoreGenreBox->rightOf(scoreLabel, Widget_Align::CENTER, padding);

    scoreGenreDD->onTop(scoreGenreBox, START, CENTER, scoreGenreBox->radius);

    scoreDDIcon = new Icon(this);
    scoreDDIcon->setImageData(dropdown_icon, dropdown_icon_len, 85, 85, IMAGE_GENERATE_MIPMAPS);
    scoreDDIcon->setSize(9, 5);
    scoreDDIcon->onTop(scoreGenreBox, END, CENTER, scoreGenreBox->radius);

    newScoreBtn = new Button(this);
    newScoreBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
    newScoreBtn->setLabel("New");
    newScoreBtn->resizeToFit();
    newScoreBtn->setCallback(this);
    newScoreBtn->rightOf(scoreGenreBox, Widget_Align::CENTER, padding);
    newScoreBtn->description = "Generate a new pattern.";

    varScoreBtn = new Button(this);
    varScoreBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
    varScoreBtn->setLabel("Variation");
    varScoreBtn->resizeToFit();
    varScoreBtn->setCallback(this);
    varScoreBtn->rightOf(newScoreBtn, Widget_Align::CENTER, padding);
    varScoreBtn->description = "Generate small variation in pattern.";

    drumPattern = new DrumPattern(this);
    drumPattern->setSize(560, 260);
    drumPattern->notes = &plugin->notesOut;
    drumPattern->noteMtx = &plugin->noteMtx;
    drumPattern->below(scoreLabel, Widget_Align::START, padding * 2.f);
    drumPattern->setCallback(this);
    drumPattern->description = "Click to add new hit. Drag hit to adjust timing. Scroll to set velocity. Click hit to delete/disable. Drag pattern to DAW.";

    drumPlayhead = new Playhead(this);
    drumPlayhead->setAbsolutePos(drumPattern->getAbsolutePos());
    drumPlayhead->setSize(drumPattern->getSize(), true);
    drumPlayhead->progress = &plugin->progress;
    drumPlayhead->accent_color = WaiveColors::light2;
    addIdleCallback(drumPlayhead);

    VBox labels(this);
    labels.setHeight(drumPattern->getHeight());

    drumNames.reserve(9);
    for (int i = 8; i >= 0; i--)
    {
        auto l = std::make_shared<Label>(this, midiNoteLabels[i]);

        l->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
        l->text_color = WaiveColors::text;
        l->resizeToFit();

        drumNames.push_back(std::move(l));
        labels.addWidget(drumNames.back().get());
    }

    labels.align_items = VBox::Align_Items::right;
    labels.justify_content = VBox::Justify_Content::space_evenly;
    labels.leftOf(drumPattern, Widget_Align::START, padding);
    labels.positionWidgets();

    // 2 - Groove controls
    grooveLabel = new Label(this, "2   Groove");
    grooveLabel->setFont("VG5000", VG5000, VG5000_len);
    grooveLabel->text_color = WaiveColors::text;
    grooveLabel->resizeToFit();
    grooveLabel->below(drumPattern, Widget_Align::START, padding * 2);

    grooveGenreBox = new Box(this);
    grooveGenreBox->background_color = WaiveColors::grey2;
    grooveGenreBox->description = "Select genre for groove pattern.";

    grooveGenreDD = new DropDown(this);
    grooveGenreDD->setSize(80, grooveGenreDD->getFontSize(), true);
    for (int i = 0; i < NUM_GROOVE_GENRES; i++)
        grooveGenreDD->addItem(groove_genres[i]);
    grooveGenreDD->setDisplayNumber(8);
    grooveGenreDD->resizeToFit();
    grooveGenreDD->setItem(plugin->getParameterValue(kGrooveGenre), false);
    grooveGenreDD->setFontSize(16.0f);
    grooveGenreDD->highlight_color = WaiveColors::grey2;
    grooveGenreDD->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
    grooveGenreDD->menu->setFontSize(16.0f);
    grooveGenreDD->menu->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
    grooveGenreDD->menu->background_color = WaiveColors::grey2;
    grooveGenreDD->setCallback(this);

    grooveGenreBox->setHeight(grooveGenreDD->getFontSize() * 2.f);
    grooveGenreBox->setWidth(grooveGenreDD->getWidth() + grooveGenreBox->getHeight() * 1.5f);
    grooveGenreBox->radius = grooveGenreBox->getHeight() / 2;
    grooveGenreBox->rightOf(grooveLabel, Widget_Align::CENTER, padding);
    grooveGenreBox->setLeft(scoreGenreBox->getLeft());

    grooveGenreDD->onTop(grooveGenreBox, START, CENTER, grooveGenreBox->radius);

    grooveDDIcon = new Icon(this);
    grooveDDIcon->setImageData(dropdown_icon, dropdown_icon_len, 85, 85, IMAGE_GENERATE_MIPMAPS);
    grooveDDIcon->setSize(9, 5);
    grooveDDIcon->onTop(grooveGenreBox, END, CENTER, grooveGenreBox->radius);

    newGrooveBtn = new Button(this);
    newGrooveBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
    newGrooveBtn->setLabel("New");
    newGrooveBtn->resizeToFit();
    newGrooveBtn->setCallback(this);
    newGrooveBtn->rightOf(grooveGenreBox, Widget_Align::CENTER, padding);
    newGrooveBtn->description = "Generate new groove pattern.";

    varGrooveBtn = new Button(this);
    varGrooveBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
    varGrooveBtn->setLabel("Variation");
    varGrooveBtn->resizeToFit();
    varGrooveBtn->setCallback(this);
    varGrooveBtn->rightOf(newGrooveBtn, Widget_Align::CENTER, padding);
    varGrooveBtn->description = "Generate variation of current groove.";

    grooveGraph = new GrooveGraph(this);
    grooveGraph->setSize(drumPattern->getWidth(), 86.f * fScaleFactor, true);
    grooveGraph->fGroove = &plugin->fGroove;
    grooveGraph->callback = this;
    grooveGraph->below(grooveLabel, Widget_Align::START, padding * 2);
    grooveGraph->description = "Scroll on groove event to adjust velocity.";

    // 3 Drum channel controls
    VBox complexitiesContainer(this);
    complexitiesContainer.setHeight(drumPattern->getHeight());

    complexities.reserve(9);
    for (int i = 8; i >= 0; i--)
    {
        auto t = std::make_shared<Knob>(this);
        t->setId(kThreshold1 + i);
        t->setSize(35, 20);
        t->setRadius(12.f);
        t->gauge_width = 1.f * fScaleFactor;
        t->foreground_color = WaiveColors::light1;
        t->showValue = false;
        t->min = 0.0f;
        t->max = 1.0f;
        t->setValue(plugin->getParameterValue(kThreshold1 + i));
        t->setCallback(this);
        t->resizeToFit();
        t->description = "Adjust complexity of drum track.";

        complexities.push_back(std::move(t));
        complexitiesContainer.addWidget(complexities.back().get());
    }

    complexitiesContainer.align_items = VBox::Align_Items::left;
    complexitiesContainer.justify_content = VBox::Justify_Content::space_evenly;
    complexitiesContainer.rightOf(drumPattern, Widget_Align::CENTER, padding);
    complexitiesContainer.positionWidgets();

    complexity = new Knob(this);
    complexity->setId(kThreshold);
    complexity->setCallback(this);
    complexity->min = 0.0f;
    complexity->max = 1.0f;
    complexity->setRadius(12.f);
    complexity->foreground_color = WaiveColors::light1;
    complexity->gauge_width = 1.f * fScaleFactor;
    complexity->showValue = false;
    complexity->setValue(plugin->getParameterValue(kThreshold));
    complexity->resizeToFit();
    complexity->setCenterX(complexities[0].get()->getCenterX());
    complexity->setCenterY(varScoreBtn->getCenterY());
    complexity->description = "Adjust all drum pattern complexity levels.";

    complexityLabel = new Label(this, "Complexity");
    complexityLabel->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
    complexityLabel->setFontSize(12.f);
    complexityLabel->text_color = WaiveColors::text;
    complexityLabel->resizeToFit();
    complexityLabel->leftOf(complexity, Widget_Align::CENTER, padding);

    midiNotesEdit.reserve(9);
    for (int i = 8; i >= 0; i--)
    {
        auto midiNote = std::make_shared<TextInput>(this);
        midiNote->align = Align::ALIGN_CENTER;
        midiNote->textType = TextInput::TextType::INTEGER;
        midiNote->setText(fmt::format("{:d}", plugin->midiNotes[i] + 1).c_str(), false);
        midiNote->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
        midiNote->setFontSize(12.f);
        midiNote->setSize(35, 20);
        midiNote->foreground_color = WaiveColors::light1;
        midiNote->accent_color = Color(88, 88, 207);
        midiNote->setId(i);
        midiNote->setCallback(this);
        midiNote->rightOf(complexities[8 - i].get(), Widget_Align::CENTER, padding);
        midiNote->description = "Set MIDI note.";

        midiNotesEdit.push_back(std::move(midiNote));
    }

    midiLabel = new Label(this, "MIDI");
    midiLabel->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
    midiLabel->setFontSize(12.f);
    midiLabel->text_color = WaiveColors::text;
    midiLabel->resizeToFit();
    midiLabel->setCenterX(midiNotesEdit[0].get()->getCenterX());
    midiLabel->setCenterY(complexityLabel->getCenterY());

    quantizeBtn = new Button(this);
    quantizeBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
    quantizeBtn->setLabel("Quantize");
    quantizeBtn->resizeToFit();
    quantizeBtn->setRight(drumPattern->getRight());
    quantizeBtn->setCenterY(varGrooveBtn->getCenterY());
    quantizeBtn->isToggle = true;
    quantizeBtn->setToggled(false);
    quantizeBtn->setCallback(this);
    quantizeBtn->description = "Quantise all hits to 16th note grid.";

    exportBtn = new Button(this);
    exportBtn->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
    exportBtn->setLabel("Export Pattern");
    exportBtn->resizeToFit();
    exportBtn->below(grooveGraph, Widget_Align::CENTER, padding);
    exportBtn->setCallback(this);
    exportBtn->description = "Export pattern to MIDI file.";

    toolTip = new Label(this, "");
    toolTip->setFontSize(18.f);
    toolTip->text_color = WaiveColors::light2;
    toolTip->setFont("Poppins-Medium", Poppins_Medium, Poppins_Medium_len);
    toolTip->setHeight(toolTip->getFontSize());
    toolTip->setWidth(mainPanel->getWidth());
    toolTip->setLeft(mainPanel->getLeft());
    toolTip->setTop(mainPanel->getBottom() + 10.f);

    setGeometryConstraints(width, height, false, false);

    if (fScaleFactor != 1.0)
        setSize(width, height);
}

WAIVESequencerUI::~WAIVESequencerUI()
{
    std::cout << "Closing WAIVESequencerUI" << std::endl;
}

void WAIVESequencerUI::parameterChanged(uint32_t index, float value)
{
    switch (index)
    {
    case kThreshold:
        complexity->setValue(value);
        for (int i = 0; i < 9; i++)
            complexities[i].get()->setValue(value);
        break;
    case kScoreGenre:
        scoreGenreDD->setItem((int)value, false);
        break;
    case kGrooveGenre:
        grooveGenreDD->setItem((int)value, false);
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
        complexities[index - kThreshold1]->setValue(value);
        break;
    case kMidi1:
    case kMidi2:
    case kMidi3:
    case kMidi4:
    case kMidi5:
    case kMidi6:
    case kMidi7:
    case kMidi8:
    case kMidi9:
        midiNotesEdit[8 - (index - kMidi1)]->setText(fmt::format("{:d}", static_cast<uint8_t>(value + 1)).c_str(), false);
        break;
    default:
        break;
    }

    repaint();
}

void WAIVESequencerUI::stateChanged(const char *key, const char *value)
{
    // printf("WAIVESequencerUI::stateChanged()\n");

    repaint();
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

bool WAIVESequencerUI::onMotion(const MotionEvent &ev)
{
    std::list<DGL::SubWidget *> children = getChildren();
    std::string text = "";
    for (const auto widget : children)
        text = getDescription(text, widget, ev);
    toolTip->setLabel(text);

    return UI::onMotion(ev);
}

void WAIVESequencerUI::onNanoDisplay()
{
    float width = getWidth();
    float height = getHeight();

    beginPath();
    fillColor(WaiveColors::dark);
    rect(0.0f, 0.0f, width, height);
    fill();
    closePath();

    float middle = mainPanel->getBottom() + 0.5f * (height - mainPanel->getBottom());
    beginPath();
    fillColor(WaiveColors::grey2);
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_CENTER);
    fontFaceId(fontTitle);
    fontSize(18.f);
    text(width / 2.f, middle, "waive sequencer", nullptr);
    closePath();

    beginPath();
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_RIGHT);
    fontFaceId(fontMain);
    fontSize(12.f);
    text(width - 10.f, middle, fmt::format("v{:d}.{:d}.{:d}", V_MAJ, V_MIN, V_PAT).c_str(), nullptr);
    closePath();
}

void WAIVESequencerUI::uiScaleFactorChanged(const double scaleFactor)
{
    fScaleFactor = scaleFactor;
}

void WAIVESequencerUI::buttonClicked(Button *button)
{
    if (button == newScoreBtn)
    {
        plugin->generateScore();
        plugin->generateFullPattern();
    }
    else if (button == newGrooveBtn)
    {
        plugin->generateGroove();
        plugin->generateFullPattern();
    }
    else if (button == varScoreBtn)
    {
        plugin->variationScore();
        plugin->generateFullPattern();
    }
    else if (button == varGrooveBtn)
    {
        plugin->variationGroove();
        plugin->generateFullPattern();
    }
    else if (button == quantizeBtn)
    {
        plugin->quantize = button->getToggled();
        plugin->computeNotes();
    }
    else if (button == exportBtn)
    {
        // create suitable file name
        std::string saveName = score_genres[plugin->score_genre];
        if (std::strcmp(score_genres[plugin->score_genre], groove_genres[plugin->groove_genre]) != 0)
        {
            saveName += "_";
            saveName += groove_genres[plugin->groove_genre];
        }
        saveName += ".mid";

        char const *filename = nullptr;
        char const *filterPatterns[1] = {"*.mid"};
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

void WAIVESequencerUI::buttonDragged(Button *button)
{
    if (button == exportBtn)
    {
    }
}

void WAIVESequencerUI::grooveClicked(GrooveGraph *graph)
{
    plugin->encodeGroove();
    repaint();
}

void WAIVESequencerUI::knobDragStarted(Knob *knob) {};

void WAIVESequencerUI::knobDragFinished(Knob *knob, float value) {};

void WAIVESequencerUI::knobValueChanged(Knob *knob, float value)
{
    setParameterValue(knob->getId(), value);

    if (knob == complexity)
        for (int i = 0; i < 9; i++)
            complexities[i].get()->setValue(value);
};

void WAIVESequencerUI::dropdownSelection(DropDown *widget, int item)
{
    // std::cout << widget->getId() << " set to " << item << std::endl;

    if (widget == scoreGenreDD)
        setParameterValue(kScoreGenre, item);
    else if (widget == grooveGenreDD)
        setParameterValue(kGrooveGenre, item);
}

void WAIVESequencerUI::textEntered(TextInput *textInput, const std::string &text)
{
    if (text.length() == 0)
    {
        textInput->undo();
        return;
    }

    errno = 0;
    char *endptr;
    long val = std::strtol(text.c_str(), &endptr, 10);

    if (endptr == text.c_str())
        return;
    else if (*endptr != '\0')
        return;

    bool clamped = false;
    if (val <= 0)
    {
        val = 1;
        clamped = true;
    }
    else if (val > 128)
    {
        val = 128;
        clamped = true;
    }

    if (clamped)
        textInput->setText(fmt::format("{:d}", val).c_str(), false);

    // plugin->setMidiNote(textInput->getId(), (uint8_t)(val - 1));
    setParameterValue(kMidi1 + textInput->getId(), static_cast<float>(val - 1));
}

void WAIVESequencerUI::textInputChanged(TextInput *textInput, const std::string &text)
{
}

void WAIVESequencerUI::onDrumPatternClicked(DrumPattern *widget, int instrument, int sixteenth)
{
    // std::cout << "WAIVESequencerUI::onDrumPatternClicked instrument " << instrument << " sixteenth " << sixteenth << std::endl;
    plugin->addNote(instrument, sixteenth, 100);
}

void WAIVESequencerUI::onDrumPatternScrolled(DrumPattern *widget, std::shared_ptr<Note> note, float deltaY)
{
    uint8_t vel = note->trigger->velocity;
    vel += (deltaY < 0 ? std::floor(deltaY) : std::ceil(deltaY));
    vel = std::clamp(vel, (uint8_t)1, (uint8_t)127);
    note->trigger->velocity = vel;

    plugin->computeNotes();
}

void WAIVESequencerUI::onDrumPatternNoteMoved(DrumPattern *widget, std::shared_ptr<Note> note, uint32_t newTick)
{
    note->trigger->tick = newTick;
    plugin->computeNotes();
}

void WAIVESequencerUI::onDrumPatternDragStarted(DrumPattern *widget)
{
    std::string saveName = score_genres[plugin->score_genre];
    if (std::strcmp(score_genres[plugin->score_genre], groove_genres[plugin->groove_genre]) != 0)
    {
        saveName += "_";
        saveName += groove_genres[plugin->groove_genre];
    }
    saveName += ".mid";

    Poco::Path filepath = Poco::Path(Poco::Path::temp());
    filepath.setFileName(saveName);

    std::string filename = filepath.toString();

    setState("export", filename.c_str());

    std::cout << "Dragging " << filename << std::endl;
    DragSource::startDraggingFile(filename, (void *)getWindow().getNativeWindowHandle());
    std::cout << "finished" << std::endl;

    Poco::TemporaryFile::registerForDeletion(filename);
}

void WAIVESequencerUI::onNoteUpdated(DrumPattern *widget, std::shared_ptr<Note> note)
{
    plugin->updateNote(note);
}

END_NAMESPACE_DISTRHO
