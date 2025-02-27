#ifndef WAIVE_UTILS_HPP_INCLUDED
#define WAIVE_UTILS_HPP_INCLUDED

#include <cstdlib>
#include <string>
#include <iostream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#include <ShellAPI.h>
#elif __APPLE__ || __linux__
#include <unistd.h>
#endif

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
    system(command.c_str());
}

static void SystemOpenDirectory(const std::string &directory)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    // std::wstring wDirectory(directory.begin(), directory.end());
    ShellExecuteA(NULL, "open", directory.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__ || __linux__
    pid_t pid = fork();
    if (pid == 0)
    {
#if __APPLE__
        execlp("open", "open", directory.c_str(), (char *)NULL);
#elif __linux__
        execlp("xdg-open", "xdg-open", directory.c_str(), (char *)NULL);
#endif
        _exit(EXIT_FAILURE);
    }
#endif
}

static void SystemCopyToClipboard(const std::string &text)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    const size_t len = text.length() + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), text.c_str(), len);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
#elif __APPLE__
    std::string command = "echo " + text + " | pbcopy";
    system(command.c_str());
#elif __linux__
    std::string command = "echo " + text + " | xclip -selection clipboard";
    system(command.c_str());
#endif
}

#endif