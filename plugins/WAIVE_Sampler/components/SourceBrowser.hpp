#ifndef SOURCE_BROWSER_HPP_INCLUDED
#define SOURCE_BROWSER_HPP_INCLUDED

#include "NanoVG.hpp"
#include "Window.hpp"

#include "Label.hpp"
#include "Spinner.hpp"
#include "Checkbox.hpp"
#include "TextInput.hpp"
#include "SourceList.hpp"
#include "CheckboxList.hpp"
#include "SimpleButton.hpp"
#include "SampleDatabase.hpp"

START_NAMESPACE_DISTRHO

class SourceBrowser
    : public NanoTopLevelWidget,
      CheckboxList::Callback,
      TextInput::Callback,
      Checkbox::Callback,
      Label::Callback,
      SourceList::Callback
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void browserStopPreview() = 0;
    };

    explicit SourceBrowser(Window &window, SampleDatabase *sd_);

    void setCallback(Callback *cb);

    void setTagList(std::vector<Tag> tags);
    void setArchiveList(std::vector<std::string> archives);
    void updateSearchString(std::string text);

    void updateSourceDatabase();
    void onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg);

protected:
    void onNanoDisplay() override;
    void checkboxUpdated(Checkbox *checkbox, bool value) override;
    void checkboxesUpdated(CheckboxList *group) override;
    void textEntered(TextInput *textInput, std::string text) override;
    void textInputChanged(TextInput *textInput, std::string text) override;
    void labelClicked(Label *label) override;
    void sourceDownload(int index) override;
    void sourcePreview(int index) override;

private:
    Callback *callback;
    CheckboxList *tags, *archives;
    SourceList *source_list;
    SampleDatabase *sd;
    Spinner *loading;
    Checkbox *downloaded;
    TextInput *searchbox;
    Label *previewSample, *connectionStatus;

    std::string tagNotIn, archiveNotIn;
    std::string previewTitle;
    int previewIndex;
    bool previewingSource;

    enum ConnectionStatus
    {
        CONNECTING,
        CONNECTED,
        DOWNLOADING,
        FAILED
    };

    ConnectionStatus status;

    DISTRHO_LEAK_DETECTOR(SourceBrowser);
};

END_NAMESPACE_DISTRHO

#endif