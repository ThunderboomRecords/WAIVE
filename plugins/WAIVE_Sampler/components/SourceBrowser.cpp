#include "SourceBrowser.hpp"

SourceBrowser::SourceBrowser(Application &app, float width, float height, SampleDatabase *sd_)
    : NanoStandaloneWindow(app), sd(sd_)
{
    setGeometryConstraints(width, height, true, false);

    std::cout << "width: " << width << " height: " << height << std::endl;

    sd->databaseUpdate += Poco::delegate(this, &SourceBrowser::onDatabaseChanged);

    tags = new CheckboxList(this);
    tags->setAbsolutePos(10, 10);
    tags->setSize(width / 3, height / 3);
    tags->setCallback(this);
    tags->label = "categories";
    tags->accent_color = Color(30, 30, 30);
    tags->reposition();

    archives = new CheckboxList(this);
    archives->setSize(width / 4, height / 3);
    Layout::rightOf(archives, tags, Widget_Align::START, 10);
    archives->setColumnCount(1);
    archives->setCallback(this);
    archives->label = "archives";
    archives->accent_color = Color(30, 30, 30);
    archives->reposition();

    source_list = new SourceList(this);
    source_list->setSize(width / 3 + width / 4 + 10, 200);
    Layout::below(source_list, tags, Widget_Align::START, 10);
    // source_list->source_list = &sd->sourcesList;

    loading = new Spinner(this);
    loading->setSize(100, 100);
    loading->setAbsolutePos(200, 200);
    loading->toFront();
    loading->setLoading(true);
    addIdleCallback(loading);
}

void SourceBrowser::checkboxesUpdated(CheckboxList *group)
{
    std::cout << "SourceBrowser::checkboxesUpdated" << std::endl;

    std::string tagNotIn = "";
    std::vector<CheckboxList::CheckboxData> *tagList = tags->getData();
    int count = 0;
    for (int i = 0; i < tagList->size(); i++)
    {
        if (tagList->at(i).checked)
            continue;

        if (count > 0)
            tagNotIn += ", ";
        tagNotIn += "'" + tagList->at(i).data + "'";
        count++;
    }

    std::cout << tagNotIn << std::endl;

    sd->filterSources(tagNotIn);
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

void SourceBrowser::setTagList(std::vector<Tag> tagList)
{
    printf("SourceBrowser::setTagList %d\n", tagList.size());
    tags->clear();

    for (int i = 0; i < tagList.size(); i++)
        tags->addItem(tagList[i].name, true);

    tags->repaint();
}

void SourceBrowser::setArchiveList(std::vector<std::string> archiveList)
{
    printf("SourceBrowser::setArchiveList %d\n", archiveList.size());
    archives->clear();

    for (int i = 0; i < archiveList.size(); i++)
        archives->addItem(archiveList[i], true);

    archives->repaint();
}

void SourceBrowser::setSourceList()
{
    source_list->source_list.clear();
    for (int i = 0; i < sd->sourcesList.size(); i++)
    {
        std::cout << " - " << sd->sourcesList[i].archive
                  << ": " << sd->sourcesList[i].folder
                  << ": " << sd->sourcesList[i].name
                  << std::endl;

        source_list->source_list.push_back(sd->sourcesList[i].folder + " " + sd->sourcesList[i].name);
    }
}

void SourceBrowser::onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg)
{
    switch (arg)
    {
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADING:
        loading->setLoading(true);
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADED:
        loading->setLoading(false);
        setTagList(sd->getTagList());
        setArchiveList(sd->getArchiveList());
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_UPDATED:
        setSourceList();
        break;
    default:
        break;
    }
}

void SourceBrowser::show()
{
    NanoStandaloneWindow::show();
    if (sd == nullptr)
        return;

    if (!sd->sourcesLoaded)
    {
        sd->downloadSourcesList();
    }
}
