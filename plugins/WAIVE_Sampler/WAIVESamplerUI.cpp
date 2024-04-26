#include "WAIVESamplerUI.hpp"

START_NAMESPACE_DISTRHO


WAIVESamplerUI::WAIVESamplerUI() : UI(UI_W, UI_H),
                                   fScaleFactor(getScaleFactor()),
                                   fScale(1.0f)
{
    plugin = static_cast<WAIVESampler *>(getPluginInstancePointer());

    logo_font = createFontFromMemory("VG5000", VG5000, VG5000_len, false);

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
    repaint();
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

END_NAMESPACE_DISTRHO