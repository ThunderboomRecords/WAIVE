#include "Layout.hpp"

START_NAMESPACE_DISTRHO

void Layout::position(Position p, NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align h_align, Widget_Align v_align, int padding)
{
    switch (p)
    {
    case Position::ON_TOP:
        Layout::onTop(w1, w2, h_align, v_align, padding);
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

void Layout::rightOf(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, int padding)
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

void Layout::leftOf(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, int padding)
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

void Layout::below(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, int padding)
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

void Layout::above(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, int padding)
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

void Layout::onTop(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align h_align, Widget_Align v_align, int padding)
{
    float targetX, targetY;
    Rectangle r1 = w1->getAbsoluteArea();
    Rectangle r2 = w2->getAbsoluteArea();

    switch (h_align)
    {
    case Widget_Align::START:
        targetX = r2.getX() + padding;
        break;
    case Widget_Align::CENTER:
        targetX = r2.getX() + r2.getWidth() / 2.0f - r1.getWidth() / 2.0f;
        break;
    case Widget_Align::END:
        targetX = r2.getX() + r2.getWidth() - r1.getWidth() - padding;
    default:
        break;
    }

    switch (v_align)
    {
    case Widget_Align::START:
        targetY = r2.getY() + padding;
        break;
    case Widget_Align::CENTER:
        targetY = r2.getY() + r2.getHeight() / 2.0f - r1.getHeight() / 2.0f;
        break;
    case Widget_Align::END:
        targetY = r2.getY() + r2.getHeight() - r1.getHeight() - padding;
    default:
        break;
    }

    w1->setAbsolutePos(targetX, targetY);
}

END_NAMESPACE_DISTRHO