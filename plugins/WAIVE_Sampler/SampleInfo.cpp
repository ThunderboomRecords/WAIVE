#include "SampleInfo.hpp"

bool saveJson(json data, std::string fp)
{
    std::ofstream ofs(fp, std::ofstream::trunc);

    if (!ofs.is_open())
    {
        std::cerr << "Failed to open " << fp << std::endl;
        return false;
    }

    ofs << data;
    ofs.close();
    return true;
}

std::string makeTagString(const std::vector<Tag> &tags)
{
    if (tags.empty())
        return "";

    std::ostringstream tagStream;
    for (const auto &tag : tags)
    {
        if (&tag != &tags[0])
            tagStream << "|";
        tagStream << tag.name;
    }

    std::string tagString = tagStream.str();
    std::cout << tagString << std::endl;

    return tagString;
}

SampleInfo::SampleInfo(const SampleInfo &other) : id(other.id),
                                                  embedX(other.embedX),
                                                  embedY(other.embedY),
                                                  volume(other.volume),
                                                  pitch(other.pitch),
                                                  percussiveBoost(other.percussiveBoost),
                                                  sustainLength(other.sustainLength),
                                                  sourceStart(other.sourceStart),
                                                  sampleLength(other.sampleLength),
                                                  saved(other.saved),
                                                  name(other.name),
                                                  path(other.path),
                                                  waive(other.waive),
                                                  filterCutoff(other.filterCutoff),
                                                  filterResonance(other.filterResonance),
                                                  filterType(other.filterType),
                                                  adsr(other.adsr),
                                                  preset(other.preset),
                                                  tags(other.tags),
                                                  tagString(other.tagString),
                                                  sourceInfo(other.sourceInfo),
                                                  fullPath(other.fullPath)
{
}

SampleInfo::SampleInfo(
    int id,
    std::string name,
    std::string path,
    bool waive) : id(id),
                  embedX(0.0f),
                  embedY(0.0f),
                  volume(1.0f),
                  pitch(1.0f),
                  percussiveBoost(1.0f),
                  sustainLength(10.0f),
                  sourceStart(0),
                  sampleLength(0),
                  saved(false),
                  name(name),
                  path(path),
                  waive(waive),
                  filterCutoff(0.99),
                  filterResonance(0.0),
                  filterType(Filter::FILTER_LOWPASS)
{
}

int SampleInfo::getId() const
{
    return id;
}

void SampleInfo::setId(int newId)
{
    id = newId;
}

void SampleInfo::setTags(const std::vector<Tag> &tags_)
{
    tags = tags_;
    tagString = makeTagString(tags_);
}

void SampleInfo::print() const
{
    std::string f_type;
    switch (filterType)
    {
    case Filter::FILTER_LOWPASS:
        f_type.assign("lowpass");
        break;
    case Filter::FILTER_HIGHPASS:
        f_type.assign("highpass");
        break;
    case Filter::FILTER_BANDPASS:
        f_type.assign("bandpass");
        break;
    default:
        f_type.assign("<unknown>");
        break;
    }

    printf("================\n");
    printf("SampleInfo: %d (waive: %d)\n", id, waive);
    printf(" - source: %s\n - sourceStart: %zu\n - sampleLength: %zu\n", sourceInfo.fp.c_str(), sourceStart, sampleLength);
    printf(" - embedding: %.3f %.3f\n", embedX, embedY);
    printf(" - Parameters:\n   volume: %.3f  percussiveBoost: %.3f  pitch: %.3f\n", volume, percussiveBoost, pitch);
    printf(" - ADSR:\n    A: %.3fms  D: %.3fms  S:  %.3f (length %.1fms) R: %.3fms\n", adsr.attack, adsr.decay, adsr.sustain, sustainLength, adsr.release);
    printf(" - Filter:\n    filterType: %s  cuffoff: %.3f  resonance: %.3f\n", f_type.c_str(), filterCutoff, filterResonance);
    printf(" - Preset: %s\n", preset.c_str());
    printf("================\n");
}

json SampleInfo::toJson() const
{
    json data;
    data["id"] = id;
    data["name"] = name;
    data["path"] = path;
    data["waive"] = waive;
    data["source"] = sourceInfo.fp;
    data["sourceStart"] = sourceStart;
    data["sampleLength"] = sampleLength;
    data["embedding"] = {{"x", embedX}, {"y", embedY}};
    data["ampEnv"] = {
        {"attack", adsr.attack},
        {"decay", adsr.decay},
        {"sustain", adsr.sustain},
        {"release", adsr.release},
        {"sustainLength", sustainLength},
    };
    data["parameters"] = {
        {"volume", volume},
        {"pitch", pitch},
        {"percussiveBoost", percussiveBoost},
    };
    data["filter"] = {
        {"filterCutoff", filterCutoff},
        {"filterResonance", filterResonance},
        {"filterType", filterType},
    };
    data["tags"] = json::array();
    for (const Tag &t : tags)
        data["tags"].push_back(t.name);
    data["tagString"] = tagString;
    data["saved"] = saved;
    data["preset"] = preset;

    return data;
}