#include "WAIVEMidiUI.hpp"


START_NAMESPACE_DISTRHO

WAIVEMidiUI::WAIVEMidiUI() : UI(UI_W, UI_H)
{
    plugin = static_cast<WAIVEMidi *>(getPluginInstancePointer());

    logo_font = createFontFromMemory("VG5000", VG5000, VG5000_len, false);

    hbox_controls = new HBox(this);

    fThreshold = new VSlider(hbox_controls);
    // fThreshold->setId(kThreshold);
    // fThreshold->setSize(Size<uint>(20, 160));
    // fThreshold->setCallback(this);
    // fThreshold->gauge_width = 12.0f;
    // fThreshold->max = 1.0f;
    // fThreshold->foreground_color = Color(51, 51, 51);
    // fThreshold->background_color = Color(255, 255, 255);
    // fThreshold->marker_color = Color(0, 0, 0);

    vbox_container = new VBox(hbox_controls);

    score_grid = new ScoreGrid(this);
    score_grid->setSize(Size<uint>(350, 250));
    score_grid->fScore = &plugin->fScore;
    score_grid->ui = this;

    groove_graph = new GrooveGraph(this);
    groove_graph->setSize(Size<uint>(350, 50));
    groove_graph->fGroove = &plugin->fGroove;
    groove_graph->callback = this;

    vbox_container->setSize(Size<uint>(350, 320));
    vbox_container->padding = 10;
    vbox_container->justify_content = VBox::Justify_Content::space_between;
    vbox_container->addWidget(score_grid);
    vbox_container->addWidget(groove_graph);

    drum_pattern = new DrumPattern(this);
    drum_pattern->setSize(Size<uint>(350, 250));
    drum_pattern->notes = &plugin->notes;

    hbox_controls->setAbsolutePos(100, 40);
    hbox_controls->setWidth(UI_W - 10);
    hbox_controls->padding = 10;
    hbox_controls->justify_content = HBox::Justify_Content::left;
    hbox_controls->align_items = HBox::Align_Items::top;
    hbox_controls->addWidget(vbox_container);
    hbox_controls->addWidget(drum_pattern);
    hbox_controls->positionWidgets();

    vbox_container->positionWidgets();

    drum_playhead = new Playhead(this);
    drum_playhead->setAbsolutePos(drum_pattern->getAbsolutePos());
    drum_playhead->setSize(drum_pattern->getSize());
    drum_playhead->progress = &plugin->progress;
    addIdleCallback(drum_playhead);

    setGeometryConstraints(UI_W, UI_H, true, true);
}

WAIVEMidiUI::~WAIVEMidiUI() {}

void WAIVEMidiUI::parameterChanged(uint32_t index, float value)
{   
    switch(index)
    {
        case kThreshold:
            fThreshold->setValue(value);
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
    fillColor(Color(240, 240, 240));
    rect(0.0f, 0.0f, width, height);
    fill();
    closePath();

    beginPath();
    fillColor(Color(40, 40, 40));
    fontSize(32);
    textAlign(Align::ALIGN_RIGHT | Align::ALIGN_TOP);
    fontFaceId(logo_font);
    text(width-10, 4, "waive", nullptr);
    closePath();

    beginPath();
    fillColor(Color(60, 60, 60));
    fontSize(16);
    textAlign(Align::ALIGN_LEFT | Align::ALIGN_BOTTOM);
    fontFaceId(logo_font);
    text(score_grid->getAbsoluteX(), score_grid->getAbsoluteY(), "score:", nullptr);
    text(groove_graph->getAbsoluteX(), groove_graph->getAbsoluteY(), "groove:", nullptr);
    text(drum_pattern->getAbsoluteX(), drum_pattern->getAbsoluteY(), "result:", nullptr);
    closePath();

    int row_height = score_grid->getHeight() / 9;
    beginPath();
    fillColor(Color(60, 60, 60));
    fontSize(14);
    textAlign(Align::ALIGN_RIGHT | Align::ALIGN_CENTER);
    fontFaceId(logo_font);
    int x, y;
    x = score_grid->getAbsoluteX() - 2;
    y = score_grid->getAbsoluteY() + score_grid->getHeight() - row_height/2;
    for(int i = 0; i < 9; i++)
    {
        text(x, y, labels[i], nullptr);
        y -= row_height; 
    }
    closePath();

}

void WAIVEMidiUI::sliderDragStarted(Slider *slider){}

void WAIVEMidiUI::sliderDragFinished(Slider *slider, float value){}

void WAIVEMidiUI::sliderValueChanged(Slider *slider, float value)
{
    if(slider == fThreshold){
        setParameterValue(kThreshold, value);
    }
}

void WAIVEMidiUI::grooveClicked(GrooveGraph *graph)
{
    plugin->encodeGroove();
    repaint();
}

END_NAMESPACE_DISTRHO