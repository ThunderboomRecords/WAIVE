#include "SourceBrowser.hpp"

SourceBrowser::SourceBrowser(Window &window, SampleDatabase *sd_)
    : NanoTopLevelWidget(window), sd(sd_)
{
    sd->databaseUpdate += Poco::delegate(this, &SourceBrowser::onDatabaseChanged);

    float width = window.getWidth();
    float height = window.getHeight();

    tags = new CheckboxList(this);
    tags->setAbsolutePos(10, 10);
    tags->setSize((width - 30) / 2, height / 3);
    tags->setCallback(this);
    tags->label = "categories";
    tags->accent_color = Color(30, 30, 30);
    tags->reposition();

    archives = new CheckboxList(this);
    archives->setSize((width - 30) / 2, height / 3);
    Layout::rightOf(archives, tags, Widget_Align::START, 10);
    archives->setColumnCount(1);
    archives->setCallback(this);
    archives->label = "archives";
    archives->accent_color = Color(30, 30, 30);
    archives->reposition();

    source_list = new SourceList(this);
    source_list->setSize(width - 20, height - height / 3 - 30);
    Layout::below(source_list, tags, Widget_Align::START, 10);

    loading = new Spinner(this);
    loading->setSize(100, 100);
    loading->setAbsolutePos(200, 200);
    loading->toFront();
    loading->setLoading(true);
    addIdleCallback(loading);

    setTagList(sd->getTagList());
    setArchiveList(sd->getArchiveList());
    sd->filterSources(tagNotIn, archiveNotIn);
}

void SourceBrowser::checkboxesUpdated(CheckboxList *group)
{
    tagNotIn = "";
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

    archiveNotIn = "";
    std::vector<CheckboxList::CheckboxData> *archiveList = archives->getData();
    count = 0;
    for (int i = 0; i < archiveList->size(); i++)
    {
        if (archiveList->at(i).checked)
            continue;

        if (count > 0)
            archiveNotIn += ", ";
        archiveNotIn += "'" + archiveList->at(i).data + "'";
        count++;
    }

    sd->filterSources(tagNotIn, archiveNotIn);
}

void SourceBrowser::onNanoDisplay()
{
    // printf("SourceBrowser::onNanoDisplay(): isVisible %d\n", isVisible());
    // TODO: why is this trying to render when window is closed?

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
        source_list->source_list.push_back(sd->sourcesList[i].folder + " " + sd->sourcesList[i].name);
    source_list->repaint();
}

void SourceBrowser::onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg)
{
    switch (arg)
    {
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADING:
        loading->setLoading(true);
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR:
        loading->setLoading(false);
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

void SourceBrowser::updateSourceDatabase()
{
    if (sd == nullptr)
        return;

    if (!sd->sourcesLoaded)
        sd->downloadSourcesList();
}
