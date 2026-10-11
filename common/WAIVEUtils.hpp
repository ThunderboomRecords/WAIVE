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
#include <sys/wait.h>
#endif

// Opens a URL or path with the system's default handler
static void SystemOpen(const std::string &target)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    ShellExecuteA(NULL, "open", target.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__ || __linux__
    // Double fork so the opener is reparented to init and reaped there,
    // rather than becoming a zombie of the host process
    pid_t pid = fork();
    if (pid == 0)
    {
        if (fork() == 0)
        {
#if __APPLE__
            execlp("open", "open", target.c_str(), (char *)NULL);
#elif __linux__
            execlp("xdg-open", "xdg-open", target.c_str(), (char *)NULL);
#endif
        }
        _exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        waitpid(pid, NULL, 0);
    }
#else
#error "Unknown compiler"
#endif
}

static void SystemOpenURL(const std::string &url)
{
    SystemOpen(url);
}

static void SystemOpenDirectory(const std::string &directory)
{
    SystemOpen(directory);
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