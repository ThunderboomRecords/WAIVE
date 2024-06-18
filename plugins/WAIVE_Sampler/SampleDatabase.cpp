#include "SampleDatabase.hpp"

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

SampleInfo::SampleInfo(
    int id,
    std::string name,
    std::string path,
    bool waive) : id(id),
                  source(""),
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
    printf(" - source: %s\n - sourceStart: %d\n - sampleLength: \n", source.c_str(), sourceStart, sampleLength);
    printf(" - embedding: %.3f %.3f\n", embedX, embedY);
    printf(" - Parameters:\n   volume: %.3f  percussiveBoost: %.3f  pitch: %.3f\n", volume, percussiveBoost, pitch);
    printf(" - ADSR:\n    A: %.3fms  D: %.3fms  S:  %.3f (length %.1fms) R: %.3fms\n", adsr.attack, adsr.decay, adsr.sustain, sustainLength, adsr.release);
    printf(" - Filter:\n    filterType: %s  cuffoff: %.3f  resonance: %.3f\n", f_type.c_str(), filterCutoff, filterResonance);
    printf("================\n");
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
    data["tags"] = json::array();
    for (const Tag t : tags)
        data["tags"].push_back(t.name);

    data["saved"] = saved;

    return data;
}

SampleDatabase::SampleDatabase(HTTPClient *_httpClient)
    : sourceDatabaseConnected(false),
      httpClient(_httpClient),
      sourcesLoaded(false)
{
    // Get and create the directory where samples and sound files will
    // be saved to
    Poco::Path homedir = Poco::Path::dataHome();
    rootDir = homedir.append(Poco::Path("WAIVE"));

    Poco::File dir(rootDir);
    if (!dir.exists())
        dir.createDirectories();

    sourceFolder = Poco::Path(homedir).append(SOURCE_DIR);
    Poco::File sourceDir(sourceFolder);
    if (!sourceDir.exists())
        sourceDir.createDirectories();

    sampleFolder = Poco::Path(homedir).append(SAMPLE_DIR);
    Poco::File sampleDir(sampleFolder);
    if (!sampleDir.exists())
        sampleDir.createDirectories();

    Poco::File db = Poco::Path(homedir).append("WAIVE.db");

    // SAMPLES Database
    Poco::Data::SQLite::Connector::registerConnector();
    session = new Poco::Data::Session("SQLite", db.path());
    *session << "CREATE TABLE IF NOT EXISTS Samples ("
                "id INTEGER PRIMARY KEY, "
                "name TEXT, "
                "path TEXT, "
                "source TEXT, "
                "parameters TEXT)",
        Poco::Data::Keywords::now;

    // SOURCES Database
    *session << "CREATE TABLE IF NOT EXISTS Sources ("
                "id INTEGER PRIMARY KEY, "
                "name TEXT, "
                "archive TEXT, "
                "folder TEXT, "
                "downloaded INT, "
                "license INT, "
                "description TEXT, "
                "UNIQUE(name, archive, folder))",
        Poco::Data::Keywords::now;

    // TAGS Databases
    *session << "CREATE TABLE IF NOT EXISTS Tags ("
                "id INTEGER PRIMARY KEY, "
                "tag TEXT UNIQUE)",
        Poco::Data::Keywords::now;

    *session << "CREATE TABLE IF NOT EXISTS SourcesTags ("
                "source_id INT, "
                "tag_id INT, "
                "FOREIGN KEY (source_id) REFERENCES Sources(id), "
                "FOREIGN KEY (tag_id) REFERENCES Tags(id), "
                "PRIMARY KEY (source_id, tag_id))",
        Poco::Data::Keywords::now;

    *session << "CREATE TABLE IF NOT EXISTS SamplesTags ("
                "sample_id INT, "
                "tag_id INT, "
                "FOREIGN KEY (sample_id) REFERENCES Samples(id), "
                "FOREIGN KEY (tag_id) REFERENCES Tags(id), "
                "PRIMARY KEY (sample_id, tag_id))",
        Poco::Data::Keywords::now;

    // License database
    *session << "CREATE TABLE IF NOT EXISTS License ("
                "id INTEGER PRIMARY KEY, "
                "tag TEXT UNIQUE)",
        Poco::Data::Keywords::now;

    std::cout << "SampleDatabase initialised\n";

    loadSampleDatabase();
}

SampleDatabase::~SampleDatabase()
{
    session->close();
}

void SampleDatabase::loadSampleDatabase()
{
    Poco::Data::Statement select(*session);
    select << "SELECT * FROM Samples", Poco::Data::Keywords::now;

    Poco::Data::RecordSet rs(select);
    std::size_t cols = rs.columnCount();

    fAllSamples.clear();
    points.clear();

    for (Poco::Data::RecordSet::Iterator it = rs.begin(); it != rs.end(); ++it)
    {
        int id = it->get(0);
        std::string name = it->get(1);
        std::string path = it->get(2);
        std::string source = it->get(3);
        std::string parameters = it->get(4);
        json data = json::parse(parameters);
        data["id"] = id;
        data["name"] = name;
        data["path"] = path;
        data["source"] = source;

        std::shared_ptr<SampleInfo> s = deserialiseSampleInfo(data);
        if (s == nullptr)
        {
            std::cout << "Could not parse row:\n"
                      << *it << std::endl;
            continue;
        }

        fAllSamples.push_back(s);
        SamplePoint point(data["embedding"]["x"], data["embedding"]["y"]);
        point.info = s;
        points.push_back(point);
    }

    kdtree.build(points);
    std::cout << "Number of samples found: " << fAllSamples.size() << std::endl;

    databaseUpdate.notify(this, DatabaseUpdate::SAMPLE_LIST_LOADED);
}

std::shared_ptr<SampleInfo> SampleDatabase::deserialiseSampleInfo(json data)
{
    try
    {
        std::shared_ptr<SampleInfo> s(new SampleInfo(data["id"], data["name"], data["path"], data["waive"]));
        s->embedX = data["embedding"]["x"];
        s->embedY = data["embedding"]["y"];
        for (auto &el : data["tags"])
            s->tags.push_back({el});
        s->sampleLength = data["sampleLength"];
        s->source = data["source"];
        s->sourceStart = data["sourceStart"];
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

bool SampleDatabase::addToLibrary(std::shared_ptr<SampleInfo> sample)
{
    if (sample == nullptr)
        return false;

    sample->saved = true;

    int id = sample->getId();
    std::string parameters = sample->toJson().dump();

    std::cout << "addToLibrary id " << id << std::endl;
    Poco::Data::Statement insert(*session);
    insert << "INSERT INTO Samples(name, path, source, parameters) VALUES(?, ?, ?, ?)",
        Poco::Data::Keywords::use(sample->name),
        Poco::Data::Keywords::use(sample->path),
        Poco::Data::Keywords::use(sample->source),
        Poco::Data::Keywords::use(parameters),
        Poco::Data::Keywords::now;
    insert.execute();

    // TODO: update Tags and Sample<->Tags database
    for (Tag t : sample->tags)
        newTag(t.name);

    std::cout << "done\n";

    fAllSamples.push_back(sample);
    databaseUpdate.notify(this, DatabaseUpdate::SAMPLE_ADDED);
    return true;
}

void SampleDatabase::newTag(std::string &tag)
{
    Poco::Data::Statement insertTag(*session);
    insertTag << "INSERT OR IGNORE INTO Tags (tag) VALUES (?)",
        Poco::Data::Keywords::use(tag),
        Poco::Data::Keywords::now;
    insertTag.execute();
}

bool SampleDatabase::renameSample(std::shared_ptr<SampleInfo> sample, std::string new_name)
{
    if (sample == nullptr)
        return false;

    Poco::File sampleFile(rootDir.append(sample->path).append(sample->name));
    Poco::Path newName(rootDir.append(sample->path).append(new_name));

    if (newName.getExtension().compare(".wav") != 0)
        newName.setExtension(".wav");

    try
    {
        sampleFile.renameTo(newName.toString(), Poco::File::OPT_FAIL_ON_OVERWRITE);
    }
    catch (Poco::FileExistsException &e)
    {
        std::cerr << "Failed to rename sample to " << newName.toString() << std::endl;
        std::cerr << e.what() << std::endl;

        return false;
    }

    // updated SampleInfo
    sample->name.assign(newName.getFileName());
    int id = sample->getId();

    Poco::Data::Statement update(*session);
    update << "UPDATE Samples SET name = ? WHERE id = ?",
        Poco::Data::Keywords::use(new_name),
        Poco::Data::Keywords::use(id),
        Poco::Data::Keywords::now;

    std::cout << update.toString() << std::endl;

    update.execute();
    databaseUpdate.notify(this, DatabaseUpdate::SAMPLE_UPDATED);

    return true;
}

std::shared_ptr<SampleInfo> SampleDatabase::findSample(int id)
{
    // TODO: make more efficient
    // - caching?
    // - hash table/unordered map?
    if (id < 0)
        return nullptr;

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

std::string SampleDatabase::getSamplePath(std::shared_ptr<SampleInfo> sample) const
{
    // std::string saveName = fmt::format("{:d}_{}", sample->getId(), sample->name);
    return Poco::Path(sampleFolder).append(sample->path).append(sample->name).toString();
}

std::string SampleDatabase::getSampleFolder() const
{
    return sampleFolder.toString();
}

std::string SampleDatabase::getSourceFolder() const
{
    return sourceFolder.toString();
}

std::string SampleDatabase::getSourcePreview() const
{
    return sourcePreviewPath;
}

std::shared_ptr<SampleInfo> SampleDatabase::duplicateSampleInfo(std::shared_ptr<SampleInfo> sample)
{
    time_t current_time = time(NULL);

    if (sample == nullptr)
    {
        std::cout << "duplucateSample: null input, initialising new.\n";
        std::string name = fmt::format("Sample{:d}.wav", current_time % 10000);
        return std::make_shared<SampleInfo>(current_time, name, SAMPLE_DIR, true);
    }

    std::shared_ptr<SampleInfo> s(new SampleInfo(current_time, sample->name, sample->path, sample->waive));
    s->pitch = sample->pitch;
    s->percussiveBoost = sample->percussiveBoost;
    s->volume = sample->volume;
    s->filterCutoff = sample->filterCutoff;
    s->filterResonance = sample->filterResonance;
    s->filterType = sample->filterType;
    s->sustainLength = sample->sustainLength;
    s->adsr.attack = sample->adsr.attack;
    s->adsr.decay = sample->adsr.decay;
    s->adsr.sustain = sample->adsr.sustain;
    s->adsr.release = sample->adsr.release;

    s->source = sample->source;
    s->sourceStart = sample->sourceStart;
    s->embedX = sample->embedX;
    s->embedY = sample->embedY;

    return s;
}

void SampleDatabase::downloadSourcesList()
{
    if (sourcesLoaded)
        return;

    printf("SampleDatabase::downloadSourcesList()\n");

    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOADING);

    httpClient->sendRequest(
        "127.0.0.1", 3000, "/sources", [this](const std::string &response)
        {
            this->sourcesLoaded = true;
            this->sourcesData = json::parse(response); 
            try
            {
                this->updateSourcesDatabase();
                databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOADED);
            } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR);

            } },
        [this]()
        {
            std::cout << "/sources not avaliable" << std::endl;
            // wait 1 second before reporting connection error...
            sleep(1);
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR);
        });
}

void SampleDatabase::downloadSourceFile(int i)
{
    if (i < 0 || i > sourcesList.size())
        return;

    SourceInfo *si = &sourcesList[i];
    if (si->downloaded == DownloadState::DOWNLOADED)
        return;

    Poco::Path location = Poco::Path(si->archive).append(si->folder);
    Poco::Path endpoint = Poco::Path("/file").append(location).append(si->name);
    Poco::File file = Poco::Path(sourceFolder).append(location).append(si->name);

    if (file.exists())
    {
        std::cout << file.path() << " already exists\n";
        si->downloaded = DownloadState::DOWNLOADED;
        return;
    }

    databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOADING);
    si->downloaded = DownloadState::DOWNLOADING;

    std::string filePath = file.path();

    httpClient->sendRequest(
        "127.0.0.1", 3000, endpoint.toString(Poco::Path::Style::PATH_URI), [this, filePath, si](const std::string &response)
        {
            // save file...
            Poco::File fp(filePath);
            Poco::Path parent = Poco::Path(filePath).makeParent();

            Poco::File(parent).createDirectories();
            bool res = fp.createFile();
            if(!res)
            {
                std::cout << "cannot create file\n";
                return;
            }
            
            Poco::FileOutputStream out = Poco::FileOutputStream(fp.path());
            out << response;
            out.close();

            // update database
            int id = si->id;
            Poco::Data::Statement update(*session);
            update << "UPDATE Sources SET downloaded = 1 WHERE id == ?", 
                Poco::Data::Keywords::use(id);
            update.execute();

            sleep(1);
            si->downloaded = DownloadState::DOWNLOADED;
            databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOADED); },
        [this, endpoint, si]()
        {
            sleep(1);
            si->downloaded = DownloadState::NOT_DOWNLOADED;
            std::cout << "http://127.0.0.1:3000" << endpoint.toString(Poco::Path::Style::PATH_URI) << " not avaliable" << std::endl;
            databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOAD_FAILED);
        });
}

void SampleDatabase::playTempSourceFile(int i)
{
    if (i < 0 || i > sourcesList.size())
        return;

    databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOADING);

    SourceInfo *si = &sourcesList[i];
    Poco::Path location = Poco::Path(si->archive).append(si->folder);

    if (si->downloaded == DownloadState::DOWNLOADED)
    {
        Poco::File preview = Poco::Path(sourceFolder).append(location).append(si->name);
        sourcePreviewPath = preview.path();
        databaseUpdate.notify(this, DatabaseUpdate::SOURCE_PREVIEW_READY);
        return;
    }

    Poco::Path endpoint = Poco::Path("/file").append(location).append(si->name);
    std::cout << endpoint.toString(Poco::Path::Style::PATH_URI) << std::endl;

    httpClient->sendRequest(
        "127.0.0.1", 3000, endpoint.toString(Poco::Path::Style::PATH_URI), [this](const std::string &response)
        {
            Poco::TemporaryFile tmp = Poco::TemporaryFile();
            tmp.keepUntilExit();

            if (!tmp.createFile())
            {
                std::cerr << "FAILED to open filestream\n";
                return;
            }

            Poco::FileOutputStream out = Poco::FileOutputStream(tmp.path());
            out << response;
            out.close(); 
            
            this->sourcePreviewPath = tmp.path();
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_PREVIEW_READY); },
        [this, endpoint, si]()
        {
            sleep(1);
            std::cout << endpoint.toString(Poco::Path::Style::PATH_URI) << " not avaliable" << std::endl;
        });
}

void SampleDatabase::updateSourcesDatabase()
{
    sourcesLoaded = false;
    // prepare statement for inserting Sources
    Poco::Data::Statement insert(*session);
    std::string name, archive, folder;
    int sourceId;

    insert << "INSERT OR IGNORE INTO Sources(name, archive, folder, downloaded) VALUES (?, ?, ?, 0)",
        Poco::Data::Keywords::use(name),
        Poco::Data::Keywords::use(archive),
        Poco::Data::Keywords::use(folder);

    // prepare statement for selecting tags
    Poco::Data::Statement selectTag(*session);
    std::string tag;
    int tagId;
    selectTag << "SELECT id FROM Tags WHERE tag = ?",
        Poco::Data::Keywords::into(tagId),
        Poco::Data::Keywords::use(tag);

    // updating the Sources<->Tags table
    Poco::Data::Statement insertTagSource(*session);
    insertTagSource << "INSERT INTO SourcesTags(source_id, tag_id) VALUES (?, ?)",
        Poco::Data::Keywords::use(sourceId),
        Poco::Data::Keywords::use(tagId);

    for (json::iterator it = sourcesData.begin(); it != sourcesData.end(); ++it)
    {
        name.assign(it.value()["name"]);
        archive.assign(it.value()["archive"]);
        folder.assign(it.value()["folder"]);

        size_t num = insert.execute();
        if (num)
        {
            *session << "SELECT last_insert_rowid()", Poco::Data::Keywords::into(sourceId), Poco::Data::Keywords::now;
            std::stringstream taglist(std::string(it.value()["tags"]));
            while (std::getline(taglist, tag, '|'))
            {
                newTag(tag);
                selectTag.execute();
                insertTagSource.execute();
            }
        }
    }

    sourcesLoaded = true;
}

std::vector<Tag> SampleDatabase::getTagList() const
{

    std::vector<Tag> tags;
    Poco::Data::Statement select(*session);
    std::string tag;
    select << "SELECT tag FROM Tags",
        Poco::Data::Keywords::into(tag),
        Poco::Data::Keywords::range(0, 1);

    while (!select.done())
    {
        if (select.execute())
            tags.push_back({std::string(tag)});
    }

    return tags;
}

std::vector<std::string> SampleDatabase::getArchiveList() const
{
    std::vector<std::string> archives;
    Poco::Data::Statement select(*session);
    std::string archive;
    select << "SELECT DISTINCT archive from Sources",
        Poco::Data::Keywords::into(archive),
        Poco::Data::Keywords::range(0, 1);

    while (!select.done())
    {
        if (select.execute())
            archives.push_back(std::string(archive));
    }

    return archives;
}

void SampleDatabase::filterSources()
{
    sourceListMutex.lock();
    sourcesList.clear();

    std::vector<std::string> conditions = {};
    if (filterConditions.tagNotIn.length() > 0)
        conditions.push_back("Tags.tag NOT IN (" + filterConditions.tagNotIn + ")");

    if (filterConditions.archiveNotIn.length() > 0)
        conditions.push_back("Sources.archive NOT IN (" + filterConditions.archiveNotIn + ")");

    if (filterConditions.downloadsOnly)
        conditions.push_back("Sources.downloaded = 1");

    if (filterConditions.searchString.length() > 0)
    {
        conditions.push_back(
            fmt::format("(Sources.name LIKE \"%{0}%\" OR Sources.description LIKE \"%{0}%\" OR Sources.folder LIKE \"%{0}%\")",
                        filterConditions.searchString));
    }

    std::string where = "";
    for (int i = 0; i < conditions.size(); i++)
    {
        if (i == 0)
            where += "WHERE ";
        else
            where += " AND ";
        where += conditions[i];
    }

    try
    {
        Poco ::Data::Statement select(*session);
        std::string name, archive, folder;
        int id, downloaded;
        select << "SELECT Sources.id, Sources.name, Sources.archive, Sources.folder, Sources.downloaded "
                  "FROM Sources "
                  "JOIN SourcesTags ON Sources.id = SourcesTags.source_id "
                  "JOIN Tags ON Tags.id = SourcesTags.tag_id "
               << where,
            Poco::Data::Keywords::into(id),
            Poco::Data::Keywords::into(name),
            Poco::Data::Keywords::into(archive),
            Poco::Data::Keywords::into(folder),
            Poco::Data::Keywords::into(downloaded),
            Poco::Data::Keywords::range(0, 1);

        std::cout << "\nSampleDatabase::filterSources select:\n  \""
                  << select.toString() << "\"\n"
                  << std::endl;

        Poco::Data::Statement selectSourcesTag(*session);
        int tagId;
        selectSourcesTag << "SELECT tag_id FROM SourcesTags WHERE source_id = ?",
            Poco::Data::Keywords::into(tagId),
            Poco::Data::Keywords::use(id),
            Poco::Data::Keywords::range(0, 1);

        Poco::Data::Statement selectTag(*session);
        std::string tag;
        selectTag << "SELECT tag FROM Tags WHERE id = ?",
            Poco::Data::Keywords::into(tag),
            Poco::Data::Keywords::use(tagId);

        while (!select.done())
        {
            if (!select.execute())
                continue;
            SourceInfo source;
            source.id = id;
            source.archive = archive;
            source.folder = folder;
            source.name = name;
            Poco::File sourceFile = Poco::Path(sourceFolder).append(source.archive).append(source.folder).append(source.name);
            if (sourceFile.exists())
                source.downloaded = DownloadState::DOWNLOADED;
            else
                source.downloaded = DownloadState::NOT_DOWNLOADED;

            while (!selectSourcesTag.done())
            {
                selectSourcesTag.execute();
                selectTag.execute();
                source.tags.push_back({std::string(tag)});
            }

            sourcesList.push_back(source);
        }
    }
    catch (const Poco::DataException &e)
    {
        sourceListMutex.unlock();
        databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_QUERY_ERROR);
        std::cerr << e.displayText() << std::endl;
        return;
    }

    sourceListMutex.unlock();
    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_UPDATED);
}
