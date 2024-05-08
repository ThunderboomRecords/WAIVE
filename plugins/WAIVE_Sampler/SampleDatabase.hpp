#ifndef SAMPLE_DATABASE_HPP
#define SAMPLE_DATABASE_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Envelopes.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class SampleInfo
{
public:
    SampleInfo(int id, std::string name, std::string path, bool waive);

    int getId() const;

    std::string name;
    std::string path;
    float embedX;
    float embedY;
    bool waive;
    std::string tags;
    std::string source;
    float volume;
    float pitch;
    ADSR_Params adsr;
    float sustainLength;
    int sourceStart;
    int sampleLength;

    bool saved;

private:
    const int id;
};

json serialiseSampleInfo(std::shared_ptr<SampleInfo> s);
std::shared_ptr<SampleInfo> deserialiseSampleInfo(json data);

bool saveJson(json data, std::string fp);

#endif