#pragma once
#include <string>
#include <iostream>

class DragSource
{
public:
    static void startDraggingFile(const std::string &filePath, void *nativeView);
};
