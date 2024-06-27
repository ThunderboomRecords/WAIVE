#include "Link.hpp"

START_NAMESPACE_DISTRHO

Link::Link(Widget *widget) noexcept : WAIVEWidget(widget)
{
    setSkipDrawing(true);
}

void Link::onNanoDisplay() {}

bool Link::onMouse(const MouseEvent &ev)
{
    if (!ev.press || ev.button != kMouseButtonLeft || url.length() == 0 || !contains(ev.pos))
        return false;

    SystemOpenURL(url);
    return true;
}

// void Link::openURL()
// {
//     std::string cmd = fmt::format("{} {}", OPEN_CMD, url);
//     std::cout << cmd << std::endl;
//     system(cmd.c_str());
// }

END_NAMESPACE_DISTRHO
