#ifndef BOX_HPP_INCLUDED
#define BOX_HPP_INCLUDED

#include "WAIVEWidget.hpp"

#include <iostream>

START_NAMESPACE_DGL

class Box : public WAIVEWidget
{
public:
    explicit Box(Widget *parent);

    float radius;

protected:
    void onNanoDisplay() override;
};

END_NAMESPACE_DGL

#endif