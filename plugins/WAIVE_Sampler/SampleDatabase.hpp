#ifndef SAMPLE_DATABASE_HPP
#define SAMPLE_DATABASE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <functional>
#include <mutex>

#include "Filters.hpp"
#include "Envelopes.hpp"
#include "WAIVESamplerParams.h"

#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/Task.h"
#include "Poco/Delegate.h"
#include "Poco/FileStream.h"
#include "Poco/BasicEvent.h"
#include "Poco/TaskManager.h"
#include "Poco/TemporaryFile.h"
#include <Poco/StringTokenizer.h>
#include "Poco/Data/Session.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Transaction.h"
#include <Poco/Data/SQLite/Utility.h>
#include "Poco/Data/SQLite/Connector.h"

#include <sndfile.hh>
#include "kdtree.h"
#include <nlohmann/json.hpp>
#include <fmt/core.h>
#include "HTTPClient.hpp"

#define WAIVE_SERVER "https://arranlyon.com/waivesampler"

using json = nlohmann::json;

// typedef Tag std::string;
struct Tag
{
    int id;
    std::string name;
    float embedX;
    float embedY;
};

class SampleInfo
{
public:
    SampleInfo(int id, std::string name, std::string path, bool waive);

    int getId() const;
    void setId(int newId);
    json toJson() const;
    void print() const;

    std::string name;
    std::string path; // relative from DATA_DIR/WAIVE
    float embedX;
    float embedY;
    bool waive;
    std::vector<Tag> tags;
    std::string source;
    float volume;
    float pitch;
    float percussiveBoost;
    float filterCutoff;
    float filterResonance;
    Filter::FilterType filterType;
    ADSR_Params adsr;
    float sustainLength;
    int sourceStart;
    int sampleLength;

    bool saved;

private:
    int id;
};

enum DownloadState
{
    NOT_DOWNLOADED,
    DOWNLOADED,
    DOWNLOADING,
};

struct SourceInfo
{
    int id;
    std::string archive;
    std::string description;
    std::string filename;
    std::vector<Tag> tags;
    DownloadState downloaded;
    std::string license;
    std::string url;
};

class SampleDatabase
{
public:
    enum DatabaseUpdate
    {
        SAMPLE_LIST_LOADED = 0,
        SAMPLE_ADDED,
        SAMPLE_UPDATED,
        SAMPLE_DELETED,
        SOURCE_LIST_DOWNLOADING,
        SOURCE_LIST_DOWNLOADED,
        SOURCE_LIST_DOWNLOAD_ERROR,
        SOURCE_LIST_UPDATED,
        SOURCE_LIST_READY,
        SOURCE_LIST_FILTER_START,
        SOURCE_LIST_FILTER_END,
        SOURCE_LIST_QUERY_ERROR,
        TAG_LIST_DOWNLOADED,
        TAG_LIST_DOWNLOAD_ERROR,
        BUILDING_TAG_LIST,
        FILE_DOWNLOADING,
        FILE_DOWNLOADED,
        FILE_DOWNLOAD_FAILED,
        SOURCE_PREVIEW_READY,
    };

    inline std::string databaseUpdateString(DatabaseUpdate v)
    {
        switch (v)
        {
        case SAMPLE_LIST_LOADED:
            return "SAMPLE_LIST_LOADED";
        case SAMPLE_ADDED:
            return "SAMPLE_ADDED";
        case SAMPLE_UPDATED:
            return "SAMPLE_UPDATED";
        case SAMPLE_DELETED:
            return "SAMPLE_DELETED";
        case SOURCE_LIST_DOWNLOADING:
            return "SOURCE_LIST_DOWNLOADING";
        case SOURCE_LIST_DOWNLOADED:
            return "SOURCE_LIST_DOWNLOADED";
        case SOURCE_LIST_DOWNLOAD_ERROR:
            return "SOURCE_LIST_DOWNLOAD_ERROR";
        case SOURCE_LIST_UPDATED:
            return "SOURCE_LIST_UPDATED";
        case SOURCE_LIST_READY:
            return "SOURCE_LIST_READY";
        case SOURCE_LIST_FILTER_START:
            return "SOURCE_LIST_FILTER_START";
        case SOURCE_LIST_FILTER_END:
            return "SOURCE_LIST_FILTER_END";
        case SOURCE_LIST_QUERY_ERROR:
            return "SOURCE_LIST_QUERY_ERROR";
        case TAG_LIST_DOWNLOADED:
            return "TAG_LIST_DOWNLOADED";
        case TAG_LIST_DOWNLOAD_ERROR:
            return "TAG_LIST_DOWNLOAD_ERROR";
        case BUILDING_TAG_LIST:
            return "BUILDING_TAG_LIST";
        case FILE_DOWNLOADING:
            return "FILE_DOWNLOADING";
        case FILE_DOWNLOADED:
            return "FILE_DOWNLOADED";
        case FILE_DOWNLOAD_FAILED:
            return "FILE_DOWNLOAD_FAILED";
        case SOURCE_PREVIEW_READY:
            return "SOURCE_PREVIEW_READY";
        default:
            return "Unknown: " + v;
        }
    }

    struct WhereConditions
    {
        std::string tagNotIn = "";
        std::string tagIn = "";
        std::string archiveNotIn = "";
        std::string searchString = "";
        bool downloadsOnly = false;
    };

    class SamplePoint : public std::array<float, 2>
    {
    public:
        using std::array<float, 2>::array;
        SamplePoint(float x, float y) : std::array<float, 2>({x, y}) {}

        static const int DIM = 2;
        std::shared_ptr<SampleInfo> info;
    };

    explicit SampleDatabase(HTTPClient *httpClient);
    ~SampleDatabase();

    Poco::BasicEvent<const DatabaseUpdate> databaseUpdate;

    // SAMPLES
    void loadSampleDatabase();
    void newTag(std::string &tag);
    bool addToLibrary(std::shared_ptr<SampleInfo> sample);
    bool updateSample(std::shared_ptr<SampleInfo> sample);
    bool renameSample(std::shared_ptr<SampleInfo> sample, std::string new_name);
    std::shared_ptr<SampleInfo> duplicateSampleInfo(std::shared_ptr<SampleInfo> sample);
    std::string getSamplePath(std::shared_ptr<SampleInfo> sample) const;
    std::string getFullSamplePath(std::shared_ptr<SampleInfo> sample) const;
    std::string getFullSourcePath(SourceInfo source) const;
    std::string getSampleFolder() const;
    std::string getSourceFolder() const;
    std::string getSourcePreview() const;
    std::string getNewSampleName(const std::string &name);

    std::shared_ptr<SampleInfo> findSample(int id);
    std::vector<std::shared_ptr<SampleInfo>> findKNearest(float x, float y, int k);
    std::vector<std::shared_ptr<SampleInfo>> findRadius(float x, float y, float r);

    // SOURCES + Remote
    void parseTSV(const std::string table, const std::vector<std::string> &column_names, const std::vector<std::string> &column_type, const std::string &csvData);
    void checkLatestRemoteVersion();
    void updateDatabaseVersion(int new_version);
    void downloadSourcesList();
    void downloadTagsList();
    void makeTagSourcesTable();
    void downloadSourceFile(int index);
    void playTempSourceFile(int index);
    void getTagList();
    std::vector<std::string> getArchiveList() const;
    void filterSources();

    // CALLBACKS
    void onTaskStarted(Poco::TaskStartedNotification *pNf);
    void onTaskFinished(Poco::TaskFinishedNotification *pNf);
    void onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg);

    std::mutex sourceListMutex;
    std::shared_ptr<SampleInfo> deserialiseSampleInfo(json data);
    std::vector<std::shared_ptr<SampleInfo>> fAllSamples;
    std::vector<SourceInfo> sourcesList;

    bool sourceDatabaseInitialised;
    bool sourcesLoaded;

    WhereConditions filterConditions;

    Poco::TaskManager taskManager;
    std::vector<Tag> tagList;

private:
    Poco::Data::Session *session;
    Poco::Path rootDir, sampleFolder, sourceFolder;
    std::string sourcePreviewPath;

    std::vector<SamplePoint> points;
    kdt::KDTree<SamplePoint> kdtree;

    HTTPClient *httpClient;
};

#endif