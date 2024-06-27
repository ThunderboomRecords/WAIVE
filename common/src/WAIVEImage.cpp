#include "WAIVEImage.hpp"

WAIVEImage::WAIVEImage(NanoSubWidget *parent, const uchar *data, uint dataSize, int width, int height, ImageFlags imageFlags)
    : NanoVG(parent->getContext()), width(width), height(height), align(Align::ALIGN_TOP | Align::ALIGN_LEFT)
{
    image = new NanoImage(createImageFromMemory(data, dataSize, imageFlags));
}

int WAIVEImage::getWidth() const
{
    return width;
}

int WAIVEImage::getHeight() const
{
    return height;
}

void WAIVEImage::drawAt(int x, int y, int w, int h, float alpha)
{
    if (w == -1 && h == -1)
    {
        w = width;
        h = height;
    }
    else if (w == -1 && h > 0)
    {
        w = ((float)h / height) * width;
    }
    else if (h == -1 && w > 0)
    {
        h = ((float)w / width) * height;
    }
    else if (w > 0 && h > 0)
    {
    }
    else
    {
        std::cout << "Invalid w or h (" << w << ", " << h << ")" << std::endl;
        return;
    }

    if (align & Align::ALIGN_RIGHT)
        x -= w;
    else if (align & Align::ALIGN_CENTER)
        x -= w * 0.5f;

    if (align & Align::ALIGN_BOTTOM || align & Align::ALIGN_BASELINE)
        y -= h;
    else if (align & Align::ALIGN_MIDDLE)
        y -= h * 0.5f;

    imagePaint = imagePattern(x, y, w, h, 0, *image, alpha);
    beginPath();
    fillPaint(imagePaint);
    rect(x, y, w, h);
    fill();
    closePath();
}