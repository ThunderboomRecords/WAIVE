#ifndef SOURCE_BROWSER_HPP_INCLUDED
#define SOURCE_BROWSER_HPP_INCLUDED

#include "NanoVG.hpp"
#include "Window.hpp"
#include "Spinner.hpp"
#include "Checkbox.hpp"
#include "TextInput.hpp"
#include "SourceList.hpp"
#include "CheckboxList.hpp"
#include "SampleDatabase.hpp"

START_NAMESPACE_DISTRHO

class SourceBrowser
    : public NanoTopLevelWidget,
      CheckboxList::Callback,
      TextInput::Callback,
      Checkbox::Callback
{
public:
    explicit SourceBrowser(Window &window, SampleDatabase *sd_);

    void setTagList(std::vector<Tag> tags);
    void setArchiveList(std::vector<std::string> archives);
    void setSourceList();
    void updateSearchString(std::string text);

    void updateSourceDatabase();
    void onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg);

protected:
    void onNanoDisplay() override;
    void checkboxUpdated(Checkbox *checkbox, bool value) override;
    void checkboxesUpdated(CheckboxList *group) override;
    void textEntered(TextInput *textInput, std::string text) override;
    void textInputChanged(TextInput *textInput, std::string text) override;

private:
    CheckboxList *tags, *archives;
    SourceList *source_list;
    SampleDatabase *sd;
    Spinner *loading;
    Checkbox *downloaded;
    TextInput *searchbox;

    std::string tagNotIn, archiveNotIn;
};

END_NAMESPACE_DISTRHO

#endif