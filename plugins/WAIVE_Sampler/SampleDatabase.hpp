#ifndef SAMPLE_DATABASE_HPP
#define SAMPLE_DATABASE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <functional>
#include <mutex>
#include <chrono>

#include "Source.hpp"
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
#include "Poco/StringTokenizer.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Transaction.h"
#include "Poco/Data/SQLite/Utility.h"
#include "Poco/Data/SQLite/Connector.h"

#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/AutoPtr.h"

#include <sndfile.hh>
#include "kdtree.h"
#include <nlohmann/json.hpp>
#include <fmt/core.h>
#include "HTTPClient.hpp"

#include "SampleInfo.hpp"

#ifdef LOCAL_SERVER
#define WAIVE_SERVER "http://localhost:3000"
#else
#define WAIVE_SERVER "https://arranlyon.com/waivesampler"
#endif

using json = nlohmann::json;

enum DownloadState
{
    NOT_DOWNLOADED,
    DOWNLOADED,
    DOWNLOADING,
};

class SourceInfo
{
public:
    explicit SourceInfo(const SourceInfo &sourceInfo);
    SourceInfo() = default;

    int id = -1;
    std::string archive = "";
    std::string description = "";
    std::string filename = "";
    std::vector<Tag> tags = {};
    DownloadState downloaded = DownloadState::NOT_DOWNLOADED;
    std::string license = "";
    std::string url = "";
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
        SOURCE_ADDED,
        SOURCE_LIST_CHECKING_UPDATE,
        SOURCE_LIST_CHECKED_UPDATE,
        SOURCE_LIST_DOWNLOADING,
        SOURCE_LIST_DOWNLOADED,
        SOURCE_LIST_DOWNLOAD_ERROR,
        SOURCE_LIST_UPDATED,
        SOURCE_LIST_READY,
        SOURCE_LIST_FILTER_START,
        SOURCE_LIST_FILTER_END,
        SOURCE_LIST_QUERY_ERROR,
        SOURCE_LIST_ANALYSED,
        TAG_LIST_DOWNLOADED,
        TAG_LIST_DOWNLOAD_ERROR,
        BUILDING_TAG_LIST_START,
        BUILDING_TAG_LIST_END,
        PARSING_CSV_START,
        PARSING_CSV_END,
        FILE_DOWNLOADING,
        FILE_DOWNLOADED,
        FILE_DOWNLOAD_FAILED,
        SOURCE_PREVIEW_READY,
    };

    inline static std::string databaseUpdateString(DatabaseUpdate v)
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
        case SOURCE_ADDED:
            return "SOURCE_ADDED";
        case SOURCE_LIST_DOWNLOADING:
            return "SOURCE_LIST_DOWNLOADING";
        case SOURCE_LIST_CHECKING_UPDATE:
            return "SOURCE_LIST_CHECKING_UPDATE";
        case SOURCE_LIST_CHECKED_UPDATE:
            return "SOURCE_LIST_CHECKED_UPDATE";
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
        case SOURCE_LIST_ANALYSED:
            return "SOURCE_LIST_ANALYSED";
        case TAG_LIST_DOWNLOADED:
            return "TAG_LIST_DOWNLOADED";
        case TAG_LIST_DOWNLOAD_ERROR:
            return "TAG_LIST_DOWNLOAD_ERROR";
        case BUILDING_TAG_LIST_START:
            return "BUILDING_TAG_LIST_START";
        case BUILDING_TAG_LIST_END:
            return "BUILDING_TAG_LIST_END";
        case PARSING_CSV_START:
            return "PARSING_CSV_START";
        case PARSING_CSV_END:
            return "PARSING_CSV_END";
        case FILE_DOWNLOADING:
            return "FILE_DOWNLOADING";
        case FILE_DOWNLOADED:
            return "FILE_DOWNLOADED";
        case FILE_DOWNLOAD_FAILED:
            return "FILE_DOWNLOAD_FAILED";
        case SOURCE_PREVIEW_READY:
            return "SOURCE_PREVIEW_READY";
        default:
            return "<UNKNOWN ENUM>  " + std::to_string(v);
        }
    }

    struct WhereConditions
    {
        std::string tagNotIn = "";
        std::string tagIn = "";
        std::string archiveNotIn = "";
        std::string archiveIs = "";
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

    explicit SampleDatabase(std::shared_ptr<HTTPClient> _httpClient);
    ~SampleDatabase();

    SampleDatabase(const SampleDatabase &) = delete;
    SampleDatabase &operator=(const SampleDatabase &) = delete;

    Poco::BasicEvent<const DatabaseUpdate> databaseUpdate;

    // SAMPLES
    void loadSampleDatabase();
    void newTag(std::string &tag);
    bool addToLibrary(std::shared_ptr<SampleInfo> sample);
    bool updateSample(std::shared_ptr<SampleInfo> sample);
    bool renameSample(std::shared_ptr<SampleInfo> sample, std::string new_name);
    bool addSourceToLibrary(const std::string &path);
    bool deleteSample(int id);
    std::shared_ptr<SampleInfo> duplicateSampleInfo(std::shared_ptr<SampleInfo> sample);
    std::string getSamplePath(std::shared_ptr<SampleInfo> sample) const;
    std::string getFullSamplePath(std::shared_ptr<SampleInfo> sample) const;
    std::string getFullSourcePath(std::shared_ptr<SourceInfo> source) const;
    std::string getSampleFolder() const;
    std::string getSourceFolder() const;
    const std::string &getSourcePreview() const;
    std::string getNewSampleName(const std::string &name);

    std::shared_ptr<SampleInfo> findSample(int id);
    std::vector<std::shared_ptr<SampleInfo>> findKNearest(float x, float y, int k);
    std::vector<std::shared_ptr<SampleInfo>> findRadius(float x, float y, float r);

    // SOURCES + Remote
    std::shared_ptr<SourceInfo> getSourceById(int id);
    void parseTSV(const std::string &table, const std::vector<std::string> &column_names, const std::vector<std::string> &column_type, const std::string &csvData);
    void checkLatestRemoteVersion();
    void updateDatabaseVersion(int new_version);
    void downloadSourcesList();
    void downloadTagsList();
    void makeTagSourcesTable();
    void downloadSourceFile(int index);
    void playTempSourceFile(int index);
    void getTagList();
    void getArchiveList();
    void filterSources();

    // CALLBACKS
    void onTaskStarted(Poco::TaskStartedNotification *pNf);
    void onTaskFinished(Poco::TaskFinishedNotification *pNf);
    void onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg);

    std::mutex sourceListMutex;
    std::shared_ptr<SampleInfo> deserialiseSampleInfo(json data);
    std::vector<std::shared_ptr<SampleInfo>> fAllSamples;
    std::vector<std::shared_ptr<SourceInfo>> sourcesList;

    bool sourceDatabaseInitialised;
    bool sourcesLoaded;

    WhereConditions filterConditions;

    Poco::TaskManager taskManager;
    std::vector<Tag> tagList;
    std::vector<std::string> archives;

    unsigned long latestDownloadedId;

private:
    std::unique_ptr<Poco::Data::Session> session;
    Poco::Path rootDir, sampleFolder, sourceFolder;
    std::string sourcePreviewPath;

    std::vector<SamplePoint> points;
    kdt::KDTree<SamplePoint> kdtree;

    std::shared_ptr<HTTPClient> httpClient;

    Poco::Logger *logger;
};

std::string makeTagString(const std::vector<Tag> &tags);

#endif