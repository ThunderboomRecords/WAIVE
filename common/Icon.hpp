#ifndef ICON_HPP_INCLUDED
#define ICON_HPP_INCLUDED

#include "WAIVEWidget.hpp"
#include "WAIVEImage.hpp"

START_NAMESPACE_DISTRHO

class Icon : public WAIVEWidget
{
public:
    Icon(Widget *widget);

    void setImageData(const uchar *data, uint dataSize, int width, int height, ImageFlags imageFlags);

    bool hoverable;

protected:
    void onNanoDisplay() override;
    bool onMotion(const MotionEvent &ev) override;
    bool onMouse(const MouseEvent &ev) override;

private:
    bool hovered;
    WAIVEImage *imageData;

    DISTRHO_LEAK_DETECTOR(Icon);
};

END_NAMESPACE_DISTRHO

#endif