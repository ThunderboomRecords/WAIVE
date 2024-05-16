#ifndef SAMPLE_DATABASE_HPP
#define SAMPLE_DATABASE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>

#include "Envelopes.hpp"
#include "WAIVESamplerParams.h"

#include <sndfile.hh>
#include "kdtree.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

fs::path get_homedir();

class SampleInfo
{
public:
    SampleInfo(int id, std::string name, std::string path, bool waive);

    int getId() const;
    json toJson() const;

    float operator[](int index);

    static const int DIM = 2;

    std::string name;
    std::string path;
    float embedX;
    float embedY;
    bool waive;
    std::string tags;
    std::string source;
    float volume;
    float pitch;
    float percussiveBoost;
    ADSR_Params adsr;
    float sustainLength;
    int sourceStart;
    int sampleLength;

    bool saved;

private:
    const int id;
};

class SampleDatabase
{
public:
    class SamplePoint : public std::array<float, 2>
    {
    public:
        using std::array<float, 2>::array;
        SamplePoint(float x, float y) : std::array<float, 2>({x, y}) {}

        static const int DIM = 2;
        std::shared_ptr<SampleInfo> info;
    };

    SampleDatabase();

    bool addToLibrary(std::shared_ptr<SampleInfo> sample);
    bool saveSamples();
    bool renameSample(std::shared_ptr<SampleInfo> sample, std::string new_name);
    std::string getSamplePath(std::shared_ptr<SampleInfo> sample);

    std::shared_ptr<SampleInfo> findSample(int id);
    std::vector<std::shared_ptr<SampleInfo>> findKNearest(float x, float y, int k);
    std::vector<std::shared_ptr<SampleInfo>> findRadius(float x, float y, float r);

    bool saveJson(json data, std::string fp);
    std::shared_ptr<SampleInfo> deserialiseSampleInfo(json data);
    std::vector<std::shared_ptr<SampleInfo>> fAllSamples;

private:
    fs::path fCacheDir;
    std::vector<SamplePoint> points;
    kdt::KDTree<SamplePoint> kdtree;
};

#endif