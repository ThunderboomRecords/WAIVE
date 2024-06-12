#include "SourceBrowser.hpp"

SourceBrowser::SourceBrowser(Application &app, float width, float height) : NanoStandaloneWindow(app)
{
    setGeometryConstraints(width, height, true, false);

    tags = new CheckboxGroup(this, 10, 10, width / 3, height - 20);
    tags->setCallback(this);
    tags->label = "categories";
    tags->setCallback(this);
    tags->toFront();

    archives = new CheckboxGroup(this, 20 + width / 3, 10, width / 3, height - 20);
    archives->setCallback(this);
    archives->label = "archives";
    archives->setCallback(this);
    archives->toFront();
}

void SourceBrowser::checkboxesUpdated(CheckboxGroup *group)
{
    std::cout << "SourceBrowser::checkboxesUpdated" << std::endl;
}

void SourceBrowser::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(Color(200, 200, 200));
    rect(0, 0, width, height);
    fill();
    stroke();
    closePath();
}