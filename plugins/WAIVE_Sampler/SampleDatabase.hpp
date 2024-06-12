#ifndef SAMPLE_DATABASE_HPP
#define SAMPLE_DATABASE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <functional>

#include "Filters.hpp"
#include "Envelopes.hpp"
#include "WAIVESamplerParams.h"

#include "Poco/TaskManager.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/SQLite/Connector.h"

#include <sndfile.hh>
#include "kdtree.h"
#include <nlohmann/json.hpp>
#include <fmt/core.h>
#include "HTTPClient.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

fs::path get_homedir();
struct Tag
{
    std::string name;
};

class SampleInfo
{
public:
    SampleInfo(int id, std::string name, std::string path, bool waive);

    int getId() const;
    json toJson() const;
    void print() const;

    std::string name;
    std::string path;
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
    const int id;
};

struct SourceInfo
{
    std::string name;
    std::string archive;
    std::string folder;
    std::vector<Tag> tags;
    bool downloaded;
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

    Poco::BasicEvent<const DatabaseUpdate &> databaseUpdate;

    void loadSampleDatabase();
    void newTag(std::string &tag);
    bool addToLibrary(std::shared_ptr<SampleInfo> sample);
    bool renameSample(std::shared_ptr<SampleInfo> sample, std::string new_name);
    static std::shared_ptr<SampleInfo> duplicateSampleInfo(std::shared_ptr<SampleInfo> sample);
    std::string getSamplePath(std::shared_ptr<SampleInfo> sample) const;
    std::string getSampleFolder() const;
    std::string makeNewSamplePath(std::string name) const;

    std::shared_ptr<SampleInfo> findSample(int id);
    std::vector<std::shared_ptr<SampleInfo>> findKNearest(float x, float y, int k);
    std::vector<std::shared_ptr<SampleInfo>> findRadius(float x, float y, float r);

    void downloadSourcesList();
    std::vector<Tag> getTagList() const;
    void updateSourcesDatabase();
    void rebuildSourceTree();
    void filterSources(const std::string &tagNotIn = "");
    std::map<std::string, std::map<std::string, std::vector<SourceInfo *>>> sourceTree;

    bool saveJson(json data, std::string fp);
    std::shared_ptr<SampleInfo> deserialiseSampleInfo(json data);
    std::vector<std::shared_ptr<SampleInfo>> fAllSamples;
    std::vector<SourceInfo> sourcesList;

    bool sourceDatabaseConnected;
    std::string sourceDatabaseStatus;

    json sourcesData;
    bool sourcesLoaded;

private:
    fs::path fCacheDir;
    Poco::Data::Session *session;

    std::vector<SamplePoint> points;
    kdt::KDTree<SamplePoint> kdtree;

    HTTPClient *httpClient;
};

#endif