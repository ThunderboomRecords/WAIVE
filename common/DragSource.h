#pragma once
#include <string>

class DragSource
{
public:
    static void startDraggingFile(const std::string &filePath, void *nativeView);
};
