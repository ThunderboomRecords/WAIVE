#ifndef SAMPLE_INFO_HPP_INCLUDED
#define SAMPLE_INFO_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Source.hpp"
#include "Filters.hpp"
#include "Envelopes.hpp"

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
    explicit SampleInfo(const SampleInfo &sampleInfo);
    SampleInfo(int id, std::string name, std::string path, bool waive);

    int getId() const;
    void setId(int newId);
    json toJson() const;
    void print() const;
    void setTags(const std::vector<Tag> &tags_);

    std::string name;
    std::string path; // relative from DATA_DIR/WAIVE
    float embedX;
    float embedY;
    bool waive;
    std::vector<Tag> tags;
    Source sourceInfo;
    std::string tagString;
    float volume;
    float pitch;
    float percussiveBoost;
    float filterCutoff;
    float filterResonance;
    Filter::FilterType filterType;
    ADSR_Params adsr;
    float sustainLength;
    size_t sourceStart;
    size_t sampleLength;
    std::string preset;

    bool saved;

private:
    int id;
};

#endif // SAMPLE_INFO_HPP_INCLUDED