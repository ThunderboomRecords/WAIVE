#include "SourceBrowser.hpp"

SourceBrowser::SourceBrowser(Window &window, SampleDatabase *sd_)
    : NanoTopLevelWidget(window),
      sd(sd_),
      status(ConnectionStatus::CONNECTING),
      previewIndex(-1),
      callback(nullptr)
{
    loadSharedResources();

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

    searchbox = new TextInput(this);
    searchbox->setSize(160, 20);
    searchbox->setCallback(this);
    searchbox->background_color = Color(220, 220, 220);
    searchbox->placeholder = "Search...";
    Layout::below(searchbox, tags, Widget_Align::START, 5);

    downloaded = new Checkbox(this);
    downloaded->setSize(160, 20);
    downloaded->label = "saved only";
    downloaded->resize();
    downloaded->setChecked(false, false);
    downloaded->setCallback(this);
    Layout::below(downloaded, archives, Widget_Align::END, 5);

    source_list = new SourceList(this);
    source_list->setSize(width - 20, height - height / 3 - 70);
    source_list->source_info = &(sd->sourcesList);
    source_list->source_info_mtx = &(sd->sourceListMutex);
    source_list->setCallback(this);
    Layout::below(source_list, searchbox, Widget_Align::START, 5);

    loading = new Spinner(this);
    loading->setSize(20, 20);
    Layout::below(loading, source_list, Widget_Align::END, 5);
    loading->toFront();

    previewSample = new Label(this, "");
    previewSample->resizeToFit();
    previewSample->setCallback(this);
    Layout::below(previewSample, source_list, Widget_Align::START, 5);

    connectionStatus = new Label(this, "");
    connectionStatus->resizeToFit();
    connectionStatus->setCallback(this);
    Layout::leftOf(connectionStatus, loading, Widget_Align::CENTER, 5);

    setTagList(sd->getTagList());
    setArchiveList(sd->getArchiveList());
    sd->filterSources();
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
    sd->filterConditions.tagNotIn.assign(tagNotIn);

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
    sd->filterConditions.archiveNotIn.assign(archiveNotIn);

    sd->filterSources();
}

void SourceBrowser::checkboxUpdated(Checkbox *checkbox, bool value)
{
    sd->filterConditions.downloadsOnly = value;
    sd->filterSources();
}

void SourceBrowser::textEntered(TextInput *textInput, std::string text)
{
    updateSearchString(text);
}

void SourceBrowser::textInputChanged(TextInput *textInput, std::string text)
{
    updateSearchString(text);
}

void SourceBrowser::sourceDownload(int index)
{
    sd->downloadSourceFile(index);
}

void SourceBrowser::sourcePreview(int index)
{
    if (index < 0)
        return;

    previewTitle.assign(sd->sourcesList.at(index).name);
    previewIndex = index;
    sd->playTempSourceFile(index);
    previewSample->setText(std::string("||  ") + previewTitle);
    previewSample->resizeToFit();
    previewingSource = true;
}

void SourceBrowser::updateSearchString(std::string text)
{
    std::string search = "";
    search.reserve(text.size());

    for (int i = 0; i < text.size(); i++)
    {
        if (text[i] != '"')
            search += text[i];
    }

    if (text.compare(sd->filterConditions.searchString) != 0)
    {
        sd->filterConditions.searchString.assign(search);
        sd->filterSources();
    }
}

void SourceBrowser::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(Color(200, 200, 200));
    rect(0, 0, width, height);
    fill();
    closePath();
}

void SourceBrowser::labelClicked(Label *label)
{
    if (label == connectionStatus)
    {
        if (status == ConnectionStatus::FAILED)
            updateSourceDatabase();
    }
    else if (label == previewSample)
    {
        if (previewingSource)
        {
            // signal plugin to stop preview...
            if (callback != nullptr)
                callback->browserStopPreview();

            previewSample->setText(std::string("▶  ") + previewTitle);
            previewSample->resizeToFit();
            previewingSource = false;
        }
        else
        {
            sourcePreview(previewIndex);
        }
    }
};

void SourceBrowser::setTagList(std::vector<Tag> tagList)
{
    tags->clear();

    for (int i = 0; i < tagList.size(); i++)
        tags->addItem(tagList[i].name, true);

    tags->repaint();
}

void SourceBrowser::setArchiveList(std::vector<std::string> archiveList)
{
    archives->clear();

    for (int i = 0; i < archiveList.size(); i++)
        archives->addItem(archiveList[i], true);

    archives->repaint();
}

void SourceBrowser::onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg)
{
    switch (arg)
    {
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADING:
        loading->setLoading(true);
        status = ConnectionStatus::DOWNLOADING;
        connectionStatus->setText("downloading list...");
        connectionStatus->resizeToFit();
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR:
        loading->setLoading(false);
        status = ConnectionStatus::FAILED;
        connectionStatus->setText("connection failed, click to retry");
        connectionStatus->resizeToFit();
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADED:
        loading->setLoading(false);
        setTagList(sd->getTagList());
        setArchiveList(sd->getArchiveList());
        status = ConnectionStatus::CONNECTED;
        connectionStatus->setText("source list downloaded");
        connectionStatus->resizeToFit();
        source_list->repaint();
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_UPDATED:
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOADING:
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOADED:
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOAD_FAILED:
        source_list->repaint();
        break;
    default:
        break;
    }

    Layout::leftOf(connectionStatus, loading, Widget_Align::CENTER, 5);
    repaint();
}

void SourceBrowser::updateSourceDatabase()
{
    if (sd == nullptr)
        return;

    if (!sd->sourcesLoaded)
        sd->downloadSourcesList();
}

void SourceBrowser::setCallback(Callback *cb)
{
    callback = cb;
}
