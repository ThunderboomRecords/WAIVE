#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include "NanoVG.hpp"
#include <iostream>

START_NAMESPACE_DISTRHO

class WAIVEImage : public NanoVG
{
public:
    WAIVEImage(NanoSubWidget *parent, const uchar *data, uint dataSize, int width, int height, ImageFlags imageFlags);

    int getWidth() const;
    int getHeight() const;

    void drawAt(int x, int y, int w = -1, int h = -1, float alpha = 1.f);

    int align;

private:
    int width, height;
    NanoImage *image;
    Paint imagePaint;
};

END_NAMESPACE_DISTRHO

#endif