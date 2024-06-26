#ifndef WAIVE_UTILS_HPP_INCLUDED
#define WAIVE_UTILS_HPP_INCLUDED

#include <cstdlib>
#include <string>
#include <iostream>

static void SystemOpenURL(const std::string &url)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    std::string command = "start " + url;
#elif __APPLE__
    std::string command = "open " + url;
#elif __linux__
    std::string command = "xdg-open " + url;
#else
#error "Unknown compiler"
    std::string command = "echo Cannot open " + url;
#endif
    // system("xdg-open"+ url);
    system(command.c_str());
}

#endif