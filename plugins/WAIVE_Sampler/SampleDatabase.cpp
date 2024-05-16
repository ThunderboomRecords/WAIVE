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

float SampleInfo::operator[](int index)
{
    if (index == 0)
        return embedX;
    else
        return embedY;
}

json SampleInfo::toJson() const
{
    json data;
    data["id"] = id;
    data["name"] = name;
    data["path"] = path;
    data["waive"] = waive;
    data["source"] = source;
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
    data["tags"] = tags;
    data["saved"] = saved;

    return data;
}

fs::path get_homedir()
{
    std::string homedir = "";
#ifdef WIN32
    homedir += getenv("HOMEDRIVE") + getenv("HOMEPATH");
#else
    homedir += getenv("HOME");
#endif
    return fs::path(homedir);
}

SampleDatabase::SampleDatabase()
{
    // Get and create the directory where samples and sound files will
    // be saved to
    fs::path homedir = get_homedir();
    fCacheDir = homedir / DATA_DIR;

    bool result = fs::create_directory(fCacheDir);

    fs::create_directory(fCacheDir / SOURCE_DIR);
    fs::create_directory(fCacheDir / SAMPLE_DIR);

    std::ifstream f(fCacheDir / "waive_samples.json");
    if (f.is_open())
    {
        json data = json::parse(f);
        auto samples = data["samples"];
        for (int i = 0; i < samples.size(); i++)
        {
            std::shared_ptr<SampleInfo> s = deserialiseSampleInfo(samples.at(i));
            if (s != nullptr)
            {
                fAllSamples.push_back(s);

                SamplePoint p = {0.0f, 0.0f};
                p.info = s;
            }
        }
    }
    else
        std::cout << "error reading waive_samples.json\n";

    std::cout << "Number of samples found: " << fAllSamples.size() << std::endl;

    kdtree.build(points);
}

std::shared_ptr<SampleInfo> SampleDatabase::deserialiseSampleInfo(json data)
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
        }
        s->volume = data["parameters"]["volume"];
        s->pitch = data["parameters"]["pitch"];
        s->percussiveBoost = data["parameters"]["percussiveBoost"];
        ADSR_Params adsr = {
            data["ampEnv"]["attack"],
            data["ampEnv"]["decay"],
            data["ampEnv"]["sustain"],
            data["ampEnv"]["release"],
        };
        s->adsr = adsr;
        s->sustainLength = data["ampEnv"]["sustainLength"];
        s->filterCutoff = data["filter"]["filterCutoff"];
        s->filterResonance = data["filter"]["filterResonance"];
        s->filterType = data["filter"]["filterType"];

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

bool SampleDatabase::saveJson(json data, std::string fp)
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

bool SampleDatabase::saveSamples()
{
    json data;
    data["samples"] = {};
    for (int i = 0; i < fAllSamples.size(); i++)
    {
        data["samples"].push_back(fAllSamples.at(i)->toJson());
    }

    return saveJson(data, fCacheDir / "waive_samples.json");
}

bool SampleDatabase::addToLibrary(std::shared_ptr<SampleInfo> sample)
{
    if (sample == nullptr)
        return false;

    sample->saved = true;

    fAllSamples.push_back(sample);

    return saveSamples();
}

bool SampleDatabase::renameSample(std::shared_ptr<SampleInfo> sample, std::string new_name)
{
    if (sample == nullptr)
        return false;

    // TODO: add guards, such as
    // - checking if new_name contains folder seperator character
    // - file extension included or not
    // - check if filename exists already

    // rename saved waveform if exists
    try
    {
        fs::rename(
            fCacheDir / sample->path / sample->name,
            fCacheDir / sample->path / new_name);
    }
    catch (fs::filesystem_error &e)
    {
        std::cerr << "Failed to rename sample to " << fCacheDir / sample->path / new_name << std::endl;
        std::cerr << e.what() << std::endl;
    }

    // updated SampleInfo
    sample->name.assign(new_name);

    return true;
}

std::shared_ptr<SampleInfo> SampleDatabase::findSample(int id)
{
    // TODO: make more efficient
    // - caching?
    // - hash table/unordered map?

    for (int i = 0; i < fAllSamples.size(); i++)
    {
        if (fAllSamples[i]->getId() == id)
            return fAllSamples.at(i);
    }
    return nullptr;
}

std::vector<std::shared_ptr<SampleInfo>> SampleDatabase::findKNearest(float x, float y, int k)
{
    std::vector<int> indices = kdtree.knnSearch({x, y}, k);
    std::vector<std::shared_ptr<SampleInfo>> result;
    for (int i = 0; i < indices.size(); i++)
    {
        result.push_back(points[indices[i]].info);
    }
    return result;
}

std::vector<std::shared_ptr<SampleInfo>> SampleDatabase::findRadius(float x, float y, float r)
{
    std::vector<int> indices = kdtree.radiusSearch({x, y}, r);
    std::vector<std::shared_ptr<SampleInfo>> result;
    for (int i = 0; i < indices.size(); i++)
    {
        result.push_back(points[indices[i]].info);
    }
    return result;
}

std::string SampleDatabase::getSamplePath(std::shared_ptr<SampleInfo> sample)
{
    return (fCacheDir / sample->path / sample->name).string();
}