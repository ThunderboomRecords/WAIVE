#ifndef LINK_HPP_INCLUDED
#define LINK_HPP_INCLUDED

#include "WAIVEWidget.hpp"
#include "WAIVEUtils.hpp"

#include <fmt/core.h>

START_NAMESPACE_DISTRHO

class Link : public WAIVEWidget
{
public:
    Link(Widget *widget) noexcept;

    // void openURL();

    std::string url;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
};

END_NAMESPACE_DISTRHO

#endif