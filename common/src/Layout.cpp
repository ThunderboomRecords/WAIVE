#include "Layout.hpp"

START_NAMESPACE_DISTRHO

void Layout::position(Position p, NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align h_align, Widget_Align v_align, float padding)
{
    switch (p)
    {
    case Position::ON_TOP:
        Layout::onTop(w1, w2, h_align, v_align, padding, padding);
        break;
    case Position::ABOVE:
        Layout::above(w1, w2, h_align, padding);
        break;
    case Position::BELOW:
        Layout::below(w1, w2, h_align, padding);
        break;
    case Position::LEFT_OF:
        Layout::leftOf(w1, w2, v_align, padding);
        break;
    case Position::RIGHT_OF:
        Layout::rightOf(w1, w2, v_align, padding);
        break;
    default:
        break;
    }
}

void Layout::rightOf(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, float padding)
{
    float targetY;
    Rectangle r1 = w1->getAbsoluteArea();
    Rectangle r2 = w2->getAbsoluteArea();

    switch (align)
    {
    case Widget_Align::START:
        targetY = r2.getY();
        break;
    case Widget_Align::CENTER:
        targetY = r2.getY() + r2.getHeight() / 2.0f - r1.getHeight() / 2.0f;
        break;
    case Widget_Align::END:
        targetY = r2.getY() + r2.getHeight() - r1.getHeight();
    default:
        break;
    }

    w1->setAbsoluteX(r2.getX() + r2.getWidth() + padding);
    w1->setAbsoluteY(targetY);
}

void Layout::leftOf(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, float padding)
{
    float targetY;
    Rectangle r1 = w1->getAbsoluteArea();
    Rectangle r2 = w2->getAbsoluteArea();

    switch (align)
    {
    case Widget_Align::START:
        targetY = r2.getY();
        break;
    case Widget_Align::CENTER:
        targetY = r2.getY() + r2.getHeight() / 2.0f - r1.getHeight() / 2.0f;
        break;
    case Widget_Align::END:
        targetY = r2.getY() + r2.getHeight() - r1.getHeight();
    default:
        break;
    }

    w1->setAbsoluteX(r2.getX() - r1.getWidth() - padding);
    w1->setAbsoluteY(targetY);
}

void Layout::below(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, float padding)
{
    float targetX;
    Rectangle r1 = w1->getAbsoluteArea();
    Rectangle r2 = w2->getAbsoluteArea();

    switch (align)
    {
    case Widget_Align::START:
        targetX = r2.getX();
        break;
    case Widget_Align::CENTER:
        targetX = r2.getX() + r2.getWidth() / 2.0f - r1.getWidth() / 2.0f;
        break;
    case Widget_Align::END:
        targetX = r2.getX() + r2.getWidth() - r1.getWidth();
    default:
        break;
    }

    w1->setAbsoluteX(targetX);
    w1->setAbsoluteY(r2.getY() + r2.getHeight() + padding);
}

void Layout::above(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, float padding)
{
    float targetX;
    Rectangle r1 = w1->getAbsoluteArea();
    Rectangle r2 = w2->getAbsoluteArea();

    switch (align)
    {
    case Widget_Align::START:
        targetX = r2.getX();
        break;
    case Widget_Align::CENTER:
        targetX = r2.getX() + r2.getWidth() / 2.0f - r1.getWidth() / 2.0f;
        break;
    case Widget_Align::END:
        targetX = r2.getX() + r2.getWidth() - r1.getWidth();
    default:
        break;
    }

    w1->setAbsoluteX(targetX);
    w1->setAbsoluteY(r2.getY() - r1.getHeight() - padding);
}

void Layout::onTop(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align h_align, Widget_Align v_align, float h_padding, float v_padding)
{
    float targetX, targetY;
    Rectangle r1 = w1->getAbsoluteArea();
    Rectangle r2 = w2->getAbsoluteArea();

    switch (h_align)
    {
    case Widget_Align::START:
        targetX = r2.getX() + h_padding;
        break;
    case Widget_Align::CENTER:
        targetX = r2.getX() + r2.getWidth() / 2.0f - r1.getWidth() / 2.0f;
        break;
    case Widget_Align::END:
        targetX = r2.getX() + r2.getWidth() - r1.getWidth() - h_padding;
    default:
        break;
    }

    switch (v_align)
    {
    case Widget_Align::START:
        targetY = r2.getY() + v_padding;
        break;
    case Widget_Align::CENTER:
        targetY = r2.getY() + r2.getHeight() / 2.0f - r1.getHeight() / 2.0f;
        break;
    case Widget_Align::END:
        targetY = r2.getY() + r2.getHeight() - r1.getHeight() - v_padding;
    default:
        break;
    }

    w1->setAbsolutePos(targetX, targetY);
}

float Layout::measureVertical(NanoSubWidget *w1, Widget_Align a1, NanoSubWidget *w2, Widget_Align a2)
{
    float y1, y2;

    switch (a1)
    {
    case Widget_Align::START:
        y1 = w1->getAbsoluteY();
        break;
    case Widget_Align::CENTER:
        y1 = w1->getAbsoluteY() + 0.5f * w1->getHeight();
        break;
    case Widget_Align::END:
        y1 = w1->getAbsoluteY() + w1->getHeight();
        break;
    default:
        break;
    }

    switch (a2)
    {
    case Widget_Align::START:
        y2 = w2->getAbsoluteY();
        break;
    case Widget_Align::CENTER:
        y2 = w2->getAbsoluteY() + 0.5f * w2->getHeight();
        break;
    case Widget_Align::END:
        y2 = w2->getAbsoluteY() + w2->getHeight();
        break;
    default:
        break;
    }

    return y2 - y1;
}

float Layout::measureHorizontal(NanoSubWidget *w1, Widget_Align a1, NanoSubWidget *w2, Widget_Align a2)
{
    float x1, x2;

    switch (a1)
    {
    case Widget_Align::START:
        x1 = w1->getAbsoluteX();
        break;
    case Widget_Align::CENTER:
        x1 = w1->getAbsoluteX() + 0.5f * w1->getWidth();
        break;
    case Widget_Align::END:
        x1 = w1->getAbsoluteX() + w1->getWidth();
        break;
    default:
        break;
    }

    switch (a2)
    {
    case Widget_Align::START:
        x2 = w2->getAbsoluteX();
        break;
    case Widget_Align::CENTER:
        x2 = w2->getAbsoluteX() + 0.5f * w2->getWidth();
        break;
    case Widget_Align::END:
        x2 = w2->getAbsoluteX() + w2->getWidth();
        break;
    default:
        break;
    }

    return x2 - x1;
}

END_NAMESPACE_DISTRHO