#include "SampleDatabase.hpp"

SampleInfo::SampleInfo(
    int id,
    std::string name,
    std::string path,
    bool waive) : id(id),
                  source(""),
                  embedX(0.0f),
                  embedY(0.0f),
                  tags(""),
                  volume(1.0f),
                  pitch(1.0f),
                  sustainLength(10.0f),
                  sourceStart(0),
                  sampleLength(0),
                  saved(false),
                  name(name),
                  path(path),
                  waive(waive)
{
}

json serialiseSampleInfo(std::shared_ptr<SampleInfo> s)
{
    json data;
    data["id"] = s->getId();
    data["name"] = s->name;
    data["path"] = s->path;
    data["waive"] = s->waive;
    data["source"] = s->source;
    data["sourceStart"] = s->sourceStart;
    data["sampleLength"] = s->sampleLength;
    data["embedding"] = {{"x", s->embedX}, {"y", s->embedY}};
    data["ampEnv"] = {
        {"attack", s->adsr.attack},
        {"decay", s->adsr.decay},
        {"sustain", s->adsr.sustain},
        {"release", s->adsr.release},
        {"sustainLength", s->sustainLength},
    };
    data["parameters"] = {
        {"volume", s->volume},
        {"pitch", s->pitch},
    };
    data["tags"] = s->tags;
    data["saved"] = s->saved;
    return data;
}

std::shared_ptr<SampleInfo> deserialiseSampleInfo(json data)
{
    try
    {
        std::shared_ptr<SampleInfo> s(new SampleInfo(data["id"], data["name"], data["path"], data["waive"]));
        s->embedX = data["embedding"]["x"];
        s->embedY = data["embedding"]["y"];
        s->tags = data["tags"];
        s->sampleLength = data["sampleLength"];
        if (s->waive)
        {
            s->source = data["source"];
            s->sourceStart = data["sourceStart"];
            s->volume = data["parameters"]["volume"];
            s->pitch = data["parameters"]["pitch"];
            ADSR_Params adsr = {
                data["ampEnv"]["attack"],
                data["ampEnv"]["decay"],
                data["ampEnv"]["sustain"],
                data["ampEnv"]["release"],
            };
            s->adsr = adsr;
            s->sustainLength = data["ampEnv"]["sustainLength"];
        }
        s->saved = data["saved"];
        return s;
    }
    catch (const json::exception &e)
    {
        std::cout << "Failed to load sample data" << std::endl;
        std::cerr << e.what() << '\n';
    }

    return nullptr;
}

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

json openJson(std::string fp)
{
    std::ifstream f(fp);
    json data = json::parse(f);
    f.close();
    return data;
}

int SampleInfo::getId() const
{
    return id;
}