#include "WAIVESamplerUI.hpp"

START_NAMESPACE_DISTRHO


WAIVESamplerUI::WAIVESamplerUI() : UI(UI_W, UI_H),
                                   fScaleFactor(getScaleFactor()),
                                   fScale(1.0f)
{
    plugin = static_cast<WAIVESampler *>(getPluginInstancePointer());

    logo_font = createFontFromMemory("VG5000", VG5000, VG5000_len, false);


    open_button = new Button(this);
    open_button->setLabel("Open");
    open_button->setFontScale(fScaleFactor);
    open_button->setBackgroundColor(Color(220, 220, 220));
    open_button->setLabelColor(Color(10, 10, 10));
    open_button->setSize(100, 50);
    open_button->setAbsolutePos(10, 10);
    open_button->setCallback(this);

    waveform_display = new Waveform(this);
    waveform_display->setSize(UI_W - 20, 80);
    waveform_display->setAbsolutePos(10, 70);

    addIdleCallback(this);

    setGeometryConstraints(UI_W*fScaleFactor, UI_H*fScaleFactor, false, false);

    if(fScaleFactor != 1.0)
    {
        setSize(UI_W*fScaleFactor, UI_H*fScaleFactor);
    }
}


WAIVESamplerUI::~WAIVESamplerUI() { }


void WAIVESamplerUI::parameterChanged(uint32_t index, float value)
{   
    switch(index)
    {
        default:
            break;
    }

    repaint();
}


void WAIVESamplerUI::stateChanged(const char *key, const char *value)
{
    std::cout << "WAIVESamplerUI::stateChanged: " << key << " -> " << value << std::endl;

    if(strcmp(key, "filename") == 0){ }

    repaint();
}


void WAIVESamplerUI::buttonClicked(Button *button)
{
    std::cout << "button clicked" << std::endl;
    
    if(button == open_button)
    {
        requestStateFile("filename");
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
    fontSize(32*fScale*fScaleFactor);
    textAlign(Align::ALIGN_RIGHT | Align::ALIGN_TOP);
    fontFaceId(logo_font);
    text(width-10*fScale*fScaleFactor, 4*fScale*fScaleFactor, "waive sampler", nullptr);
    closePath();

}


void WAIVESamplerUI::uiScaleFactorChanged(const double scaleFactor)
{
    fScaleFactor = scaleFactor;
}


void WAIVESamplerUI::idleCallback()
{
    std::queue<int> *updateQueue = &plugin->updateQueue;
    for(; !updateQueue->empty(); updateQueue->pop())
    {
        int msg = updateQueue->front();
        // std::cout << "msg: " << msg << std::endl;

        switch(msg)
        {
            case kSampleLoading:
                break;
            case kSampleLoaded:
                waveform_display->calculateWaveform(&plugin->fWaveform);
                break;
            default:
                std::cout << "Unknown update: " << msg << std::endl;
                break;
        }
    }
}

END_NAMESPACE_DISTRHO