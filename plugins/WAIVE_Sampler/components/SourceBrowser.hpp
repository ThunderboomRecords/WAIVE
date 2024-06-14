#ifndef SOURCE_BROWSER_HPP_INCLUDED
#define SOURCE_BROWSER_HPP_INCLUDED

#include "NanoVG.hpp"
#include "Window.hpp"
#include "Spinner.hpp"
#include "SourceList.hpp"
#include "CheckboxList.hpp"
#include "SampleDatabase.hpp"

START_NAMESPACE_DISTRHO

class SourceBrowser
    : public NanoTopLevelWidget,
      CheckboxList::Callback
{
public:
    explicit SourceBrowser(Window &window, SampleDatabase *sd_);

    void setTagList(std::vector<Tag> tags);
    void setArchiveList(std::vector<std::string> archives);
    void setSourceList();

    void show();
    void onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg);

protected:
    void checkboxesUpdated(CheckboxList *group);
    void onNanoDisplay() override;

private:
    CheckboxList *tags, *archives;
    SourceList *source_list;
    SampleDatabase *sd;
    Spinner *loading;
};

END_NAMESPACE_DISTRHO

#endif