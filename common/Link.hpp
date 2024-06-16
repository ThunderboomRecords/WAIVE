#ifndef LINK_HPP_INCLUDED
#define LINK_HPP_INCLUDED

#include "WAIVEWidget.hpp"

#include <fmt/core.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define OPEN_CMD "start"
#define SystemOpenURL(url) system("start " url);
#elif __APPLE__
#define OPEN_CMD "open"
#define SystemOpenURL(url) system("open " url);
#elif __linux__
#define OPEN_CMD "xdg-open"
#define SystemOpenURL(url) system("xdg-open" url);
#else
#error "Unknown compiler"
#define OPEN_CMD "echo cannot open"
#endif

START_NAMESPACE_DISTRHO

class Link : public WAIVEWidget
{
public:
    Link(Widget *widget) noexcept;

    void openURL();

    std::string url;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
};

END_NAMESPACE_DISTRHO

#endif