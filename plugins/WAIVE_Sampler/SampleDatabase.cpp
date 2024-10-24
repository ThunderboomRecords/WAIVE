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

void SampleInfo::setId(int newId)
{
    id = newId;
}

void SampleInfo::setTags(std::vector<Tag> tags_)
{
    std::cout << "setTags\n";
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
    printf(" - source: %s\n - sourceStart: %ld\n - sampleLength: %ld\n", source.c_str(), sourceStart, sampleLength);
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
    data["tagString"] = tagString;
    data["saved"] = saved;

    return data;
}

SampleDatabase::SampleDatabase(HTTPClient *_httpClient)
    : httpClient(_httpClient),
      sourcesLoaded(false),
      sourceDatabaseInitialised(true),
      latestDownloadedIndex(-1)
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

    try
    {
        *session << "CREATE TABLE IF NOT EXISTS Samples ("
                    "id INTEGER PRIMARY KEY, "
                    "name TEXT, "
                    "path TEXT, "
                    "source TEXT, "
                    "parameters TEXT)",
            Poco::Data::Keywords::now;
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Error initialising Samples table: " << e.what() << std::endl;
    }

    try
    {
        *session << "CREATE TABLE IF NOT EXISTS SamplesTags ("
                    "sample_id INT, "
                    "tag_id INT, "
                    "FOREIGN KEY (sample_id) REFERENCES Samples(id), "
                    "FOREIGN KEY (tag_id) REFERENCES Tags(id), "
                    "PRIMARY KEY (sample_id, tag_id))",
            Poco::Data::Keywords::now;
    }
    catch (const Poco::DataException &e)
    {
        std::cerr << "Error initialising SamplesTags table: " << e.what() << std::endl;
    }

    databaseUpdate += Poco::delegate(this, &SampleDatabase::onDatabaseChanged);

    std::cout << "SampleDatabase initialised\n";

    loadSampleDatabase();
}

SampleDatabase::~SampleDatabase()
{
    taskManager.cancelAll();
    taskManager.joinAll();
    session->close();
}

void SampleDatabase::loadSampleDatabase()
{
    Poco::Data::Statement select(*session);
    try
    {
        select << "SELECT * FROM Samples", Poco::Data::Keywords::now;
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Database query failed: " << e.what() << std::endl;
        return;
    }

    Poco::Data::RecordSet rs(select);
    std::size_t cols = rs.columnCount();
    if (cols < 5)
    {
        std::cerr << "Unexpected number of columns in the result set." << std::endl;
        return;
    }

    fAllSamples.clear();
    points.clear();

    for (auto &row : rs)
    {
        int id = row.get(0);
        std::string name = row.get(1).convert<std::string>();
        std::string path = row.get(2).convert<std::string>();
        std::string source = row.get(3).convert<std::string>();
        std::string parameters = row.get(4).convert<std::string>();

        json data;
        try
        {
            data = json::parse(parameters);
        }
        catch (const json::parse_error &e)
        {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            continue;
        }

        data["id"] = id;
        data["name"] = name;
        data["path"] = path;
        data["source"] = source;

        std::shared_ptr<SampleInfo> s = deserialiseSampleInfo(data);
        if (s == nullptr)
        {
            std::cerr << "Could not parse row:\n"
                      << row << std::endl;
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
        s->tagString.assign(data["tagString"]);
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

    std::cout << "SampleDatabase::addToLibrary" << std::endl;

    sample->saved = true;

    // int id = sample->getId();
    std::string parameters = sample->toJson().dump();

    try
    {
        Poco::Data::Statement insert(*session);
        insert << "INSERT INTO Samples(name, path, source, parameters) VALUES(?, ?, ?, ?)",
            Poco::Data::Keywords::use(sample->name),
            Poco::Data::Keywords::use(sample->path),
            Poco::Data::Keywords::use(sample->source),
            Poco::Data::Keywords::use(parameters);
        insert.execute();
    }
    catch (const Poco::DataException &ex)
    {
        std::cerr << "Error inserting new sample into database: " << ex.message() << std::endl;
        return false;
    }

    std::cout << "insert done" << std::endl;

    int lastId;
    try
    {
        Poco::Data::Statement lastSampleId(*session);
        lastSampleId << "SELECT last_insert_rowid()",
            Poco::Data::Keywords::into(lastId);
        lastSampleId.execute();
    }
    catch (const Poco::DataException &ex)
    {
        std::cerr << "Error getting new sampleId: " << ex.message() << std::endl;
        return false;
    }

    std::cout << "lastId " << lastId << std::endl;

    sample->setId(lastId);

    // TODO: update Sample<->Tags database
    for (Tag t : sample->tags)
    {
        std::cout << t.name << std::endl;
        newTag(t.name);
    }

    fAllSamples.push_back(sample);
    // databaseUpdate.notify(this, DatabaseUpdate::SAMPLE_ADDED);

    std::cout << "SampleDatabase::addToLibrary done" << std::endl;
    return true;
}

bool SampleDatabase::addSourceToLibrary(const std::string &path)
{
    // 1 Copy source to folder

    Poco::File sourceFile(path);
    Poco::Path savePath(sourceFolder);
    savePath.append("User").makeDirectory();
    Poco::File copyDir(savePath);
    if (!copyDir.exists())
    {
        copyDir.createDirectories();
    }

    if (!sourceFile.exists())
    {
        std::cerr << "Imported file " << path << " does not exist" << std::endl;
        return false;
    }

    try
    {
        sourceFile.copyTo(savePath.toString(), Poco::File::OPT_FAIL_ON_OVERWRITE);
    }
    catch (Poco::FileExistsException &ex)
    {
        std::cout << "File already in database folder" << std::endl;
        return false;
    }
    catch (Poco::FileNotFoundException &ex)
    {
        std::cerr << "Copy destination not found: " << savePath.toString() << std::endl;
        return false;
    }
    catch (std::exception &ex)
    {
        std::cerr << "An error occurred during file copy: " << ex.what() << std::endl;
        return false;
    }

    std::cout << "File copied to sources folder" << std::endl;

    // 2 Update database
    std::string filename = Poco::Path(path).getFileName();
    Poco::Path url = Poco::Path("User").append(filename);

    std::cout << "filename: " << filename << ", url: " << url.toString() << std::endl;

    std::string description = Poco::Path(path).getBaseName();
    std::string archive = "User";
    std::string tags = "";
    std::string urlString = url.toString();

    try
    {
        if (!session)
        {
            std::cerr << "Database session not initialized" << std::endl;
            return false;
        }

        Poco::Data::Statement insert(*session);
        insert << "INSERT INTO Sources(description, tags, archive, filename, url) VALUES(?, ?, ?, ?, ?)",
            Poco::Data::Keywords::use(description),
            Poco::Data::Keywords::use(tags),
            Poco::Data::Keywords::use(archive),
            Poco::Data::Keywords::use(filename),
            Poco::Data::Keywords::use(urlString);

        insert.execute();
    }
    catch (const Poco::DataException &ex)
    {
        std::cerr << "Error inserting new sample into database: " << ex.message() << std::endl;
        return false;
    }

    filterSources();

    return true;

    // 3 Select current Source (and notify UI) to load it
}

bool SampleDatabase::updateSample(std::shared_ptr<SampleInfo> sample)
{
    if (sample == nullptr)
        return false;

    // std::cout << "SampleDatabase::updateSample" << std::endl;

    int id = sample->getId();
    std::string parameters = sample->toJson().dump();

    // std::cout << parameters << std::endl;

    int n = 0;
    try
    {
        Poco::Data::Statement update(*session);
        update << "UPDATE Samples SET name = ?, path = ?, source = ?, parameters = ? WHERE id = ?",
            Poco::Data::Keywords::use(sample->name),
            Poco::Data::Keywords::use(sample->path),
            Poco::Data::Keywords::use(sample->source),
            Poco::Data::Keywords::use(parameters),
            Poco::Data::Keywords::use(id);
        n = update.execute();
    }
    catch (const Poco::DataException &ex)
    {
        std::cerr << "Error updating sample info" << std::endl;
        return false;
    }

    // TODO: update Sample<->Tags database
    for (Tag t : sample->tags)
    {
        newTag(t.name);
    }

    if (n > 0)
    {
        // databaseUpdate.notify(this, DatabaseUpdate::SAMPLE_UPDATED);
        return true;
    }

    return false;
}

void SampleDatabase::newTag(std::string &tag)
{
    Poco::Data::Statement insertTag(*session);
    insertTag << "INSERT OR IGNORE INTO Tags (tag) VALUES (?)",
        Poco::Data::Keywords::use(tag);
    insertTag.execute();
}

bool SampleDatabase::renameSample(std::shared_ptr<SampleInfo> sample, std::string new_name)
{
    if (sample == nullptr)
        return false;

    // if (!sample->saved)
    // {
    //     // just update the SampleInfo and return
    //     sample->name.assign()
    //     return;
    // }

    Poco::File sampleFile(Poco::Path(rootDir).append(sample->path).append(sample->name));
    Poco::Path newName(Poco::Path(rootDir).append(sample->path).append(new_name));

    if (newName.getExtension().compare("wav") != 0)
        newName.setExtension("wav");

    try
    {
        sampleFile.renameTo(newName.toString(), Poco::File::OPT_FAIL_ON_OVERWRITE);
    }
    catch (Poco::FileExistsException &e)
    {
        std::cerr << "Failed to rename sample to " << newName.toString() << " since it already exists." << std::endl;
        std::cerr << e.what() << std::endl;

        return false;
    }
    catch (Poco::FileNotFoundException &e)
    {
        std::cout << "Sample is not saved yet, updating SampleInfo\n";
        sample->name.assign(newName.getFileName());
        sample->saved = false;
        return true;
    }

    // updated SampleInfo
    sample->name.assign(newName.getFileName());
    sample->saved = true;
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
    return Poco::Path(sample->path).append(sample->name).toString();
}

std::string SampleDatabase::getFullSamplePath(std::shared_ptr<SampleInfo> sample) const
{
    return Poco::Path(sampleFolder).append(sample->path).append(sample->name).toString();
}

std::string SampleDatabase::getFullSourcePath(SourceInfo source) const
{
    return Poco::Path(sourceFolder).append(source.url).toString();
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

std::string SampleDatabase::getNewSampleName(const std::string &name)
{
    std::cout << "SampleDatabase::getNewSampleName: " << name << std::endl;
    // Finds a unique name of the form "new_sample_X.wav"
    int suffixCounter = 0;
    int id;
    std::string newName(name);

    Poco::Path file(name);
    std::string basename = file.getBaseName();

    // if basename ends in `_X` or `_XX`, dont include that in pattern..
    size_t pos = basename.rfind('_');
    if (pos != std::string::npos)
    {
        bool isNumericSuffix = true;
        for (size_t i = pos + 1; i < basename.length(); ++i)
        {
            if (!std::isdigit(basename[i]))
            {
                isNumericSuffix = false;
                break;
            }
        }

        if (isNumericSuffix)
        {
            // remove numeric suffix
            basename = basename.substr(0, pos);
        }
    }

    std::string pattern = basename + "_{}." + file.getExtension();

    try
    {
        Poco::Data::Statement select(*session);
        select << "SELECT id FROM Samples WHERE name = ?",
            Poco::Data::Keywords::use(newName),
            Poco::Data::Keywords::into(id);

        while (select.execute())
        {
            if (suffixCounter > 199)
            {
                std::cerr << "Max iterations finding new sample name...\n";
                break;
            }
            suffixCounter++;
            newName = fmt::format(pattern, suffixCounter);
        }
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Error in getNewSampleName: " << e.what() << std::endl;
    }

    std::cout << "New name: " << newName << std::endl;

    return newName;
}

std::shared_ptr<SampleInfo> SampleDatabase::duplicateSampleInfo(std::shared_ptr<SampleInfo> sample)
{
    std::string duplicateName = getNewSampleName(sample->name);

    std::shared_ptr<SampleInfo> s(new SampleInfo(-1, duplicateName, sample->path, sample->waive));
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
    s->tagString = sample->tagString;
    for (auto &t : sample->tags)
        s->tags.push_back(t);
    s->source = sample->source;
    s->sourceStart = sample->sourceStart;
    s->embedX = sample->embedX;
    s->embedY = sample->embedY;

    return s;
}

void SampleDatabase::checkLatestRemoteVersion()
{
    int user_version;
    try
    {
        *session << "PRAGMA user_version",
            Poco::Data::Keywords::into(user_version),
            Poco::Data::Keywords::now;
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Failed to retrieve user version: " << e.what() << std::endl;
        user_version = 0;
    }

    std::cout << "user_version: " << user_version << std::endl;

    // check if "Sources" table exists
    std::string exists;
    try
    {
        *session << "SELECT name FROM sqlite_master WHERE type='table' AND name='Sources'",
            Poco::Data::Keywords::into(exists),
            Poco::Data::Keywords::range(0, 1),
            Poco::Data::Keywords::now;
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Failed to check if 'Sources' table exists: " << e.what() << std::endl;
        return;
    }

    sourceDatabaseInitialised = !exists.empty();

    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_READY);
    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_CHECKING_UPDATE);

    httpClient->sendRequest(
        "CheckDatabaseVersion",
        WAIVE_SERVER,
        "/latest",
        [this, user_version](const std::string &response)
        { 
            try {
                json result = json::parse(response); 
                databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_CHECKED_UPDATE);

                int latest_version = result["version"];
                if(latest_version <= user_version)
                    return;
                
                this->updateDatabaseVersion(latest_version);

            } catch(json::parse_error &e) {
                std::cerr << "JSON parsing error: " << e.what() << std::endl;
            	databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR);
            } catch(const std::exception &e){
                std::cerr << "Unexpected error: " << e.what() << std::endl;
           		databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR);
            } },
        [this]()
        {
            std::cout << "cannot connect to remote database and verify if up-to-date." << std::endl;
            if (!sourceDatabaseInitialised)
                std::cout << "no Source info avaliable\n";
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR);
        });
}

void SampleDatabase::updateDatabaseVersion(int new_version)
{
    std::cout << "SampleDatabase::updateDatabaseVersion, new_version: " << new_version << std::endl;

    try
    {
        Poco::Data::Statement pragma(*session);
        pragma << "PRAGMA user_version = " << new_version;

        std::cout << pragma.toString() << std::endl;

        pragma.execute();
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Error updating PRAGMA user_version: " << e.what() << std::endl;
    }

    downloadSourcesList();
}

void SampleDatabase::downloadSourcesList()
{
    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOADING);

    httpClient->sendRequest(
        "DownloadSourceList",
        WAIVE_SERVER, "/filelist", [this](const std::string &response)
        {
            parseTSV(
                "Sources",
                {"id", "description", "tags", "folder", "filename", "archive", "url", "license"},
                {"INTEGER PRIMARY KEY", "TEXT", "TEXT", "TEXT", "TEXT", "TEXT", "TEXT", "TEXT"},
                response);
            sourceDatabaseInitialised = true;
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOADED); },
        [this]()
        {
            // std::cout << "/sources not avaliable" << std::endl;
            // wait 1 second before reporting connection error...
            Poco::Thread::sleep(1000);
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR);
        });
}

void SampleDatabase::downloadTagsList()
{
    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOADING);

    httpClient->sendRequest(
        "DownloadTagsList",
        WAIVE_SERVER, "/tags", [this](const std::string &response)
        {
            parseTSV(
                "Tags",
                {"id", "tag", "embedX", "embedY", "counts"},
                {"INTEGER PRIMARY KEY", "TEXT", "FLOAT", "FLOAT", "INTEGER"},
                response);
            databaseUpdate.notify(this, DatabaseUpdate::TAG_LIST_DOWNLOADED); },
        [this]()
        {
            // std::cout << "/sources not avaliable" << std::endl;
            // wait 1 second before reporting connection error...
            sleep(1);
            databaseUpdate.notify(this, DatabaseUpdate::TAG_LIST_DOWNLOAD_ERROR);
        });
}

void SampleDatabase::parseTSV(
    const std::string table,
    const std::vector<std::string> &column_names,
    const std::vector<std::string> &column_type,
    const std::string &csvData)
{
    // std::cout << "SampleDatabase::parseTSV\n";

    // 1. Create database
    try
    {
        *session << "DROP TABLE IF EXISTS " << table,
            Poco::Data::Keywords::now;
    }
    catch (const Poco::DataException &e)
    {
        std::cerr << e.what() << '\n';
    }

    std::ostringstream createTableQuery;
    createTableQuery << "CREATE TABLE IF NOT EXISTS " << table << " (";
    for (size_t i = 0; i < column_names.size(); ++i)
    {
        createTableQuery << column_names[i] << " " << column_type[i];
        if (i < column_names.size() - 1)
            createTableQuery << ", ";
    }
    createTableQuery << ")";

    try
    {
        *session << createTableQuery.str(), Poco::Data::Keywords::now;
    }
    catch (const Poco::DataException &e)
    {
        std::cerr << e.what() << '\n';
    }

    // 2. Tokenise CSV data
    Poco::StringTokenizer tokenizer(csvData, "\n", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);

    // 3. Build INSERT statement
    std::ostringstream insertQuery;
    insertQuery << "INSERT INTO " << table << " (";
    for (size_t i = 0; i < column_names.size(); ++i)
    {
        insertQuery << column_names[i];
        if (i < column_names.size() - 1)
            insertQuery << ", ";
    }
    insertQuery << ") VALUES (";
    for (size_t i = 0; i < column_names.size(); ++i)
    {
        insertQuery << "?";
        if (i < column_names.size() - 1)
            insertQuery << ", ";
    }
    insertQuery << ")";

    // 4. Iterate line by line
    double progress = 0.0;
    double pStep = 1.0 / tokenizer.count();

    int batch_size = 1000;

    session->begin();

    size_t i;
    for (i = 0; i < tokenizer.count(); ++i)
    {
        std::string line = tokenizer[i];
        Poco::StringTokenizer lineTokenizer(line, "\t", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);

        if (lineTokenizer.count() == column_names.size())
        {
            try
            {
                Poco::Data::Statement insertStmt(*session);
                insertStmt << insertQuery.str();
                for (size_t j = 0; j < lineTokenizer.count(); ++j)
                    insertStmt, Poco::Data::Keywords::use(lineTokenizer[j]);

                insertStmt.execute();

                if (i % batch_size == 0)
                {
                    std::cout << "Row " << i << " parsed" << std::endl;
                    session->commit();
                    session->begin();
                }
            }
            catch (const Poco::DataException &e)
            {
                std::cerr << "Error in parseTSV: " << e.what() << std::endl;
            }
        }
        progress += pStep;
    }

    if (i % batch_size != 0)
    {
        std::cout << "Row " << i << " parsed" << std::endl;
        try
        {
            session->commit();
        }
        catch (const Poco::DataException &e)
        {
            std::cerr << "Error in parseTSV: " << e.what() << std::endl;
        }
    }

    std::cout << "ParseCSV DONE\n";
}

void SampleDatabase::downloadSourceFile(int i)
{
    if (i < 0 || i > sourcesList.size())
        return;

    SourceInfo *si = &sourcesList[i];

    if (si->downloaded == DownloadState::DOWNLOADED)
        return;

    Poco::Path location = Poco::Path(si->url);
    Poco::Path endpoint = Poco::Path("/file").append(location);
    Poco::File file = Poco::Path(sourceFolder).append(location);

    if (file.exists())
    {
        si->downloaded = DownloadState::DOWNLOADED;
        return;
    }

    databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOADING);
    si->downloaded = DownloadState::DOWNLOADING;

    std::string filePath = file.path();

    httpClient->sendRequest(
        "DownloadSourceFile",
        WAIVE_SERVER, endpoint.toString(Poco::Path::Style::PATH_URI), [this, filePath, si, i](const std::string &response)
        {
            // save file...
            Poco::File fp(filePath);
            Poco::Path parent = Poco::Path(filePath).makeParent();

            Poco::File(parent).createDirectories();
            bool res = fp.createFile();
            if(!res)
            {
                std::cerr << "Cannot create download to file: " << fp.path() << std::endl;
                return;
            }
            
            Poco::FileOutputStream out = Poco::FileOutputStream(fp.path());
            out << response;
            out.close();

            sleep(1);
            si->downloaded = DownloadState::DOWNLOADED;
            this->latestDownloadedIndex = i;
            databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOADED); },
        [this, endpoint, si]()
        {
            sleep(1);
            si->downloaded = DownloadState::NOT_DOWNLOADED;
            std::cout << WAIVE_SERVER << endpoint.toString(Poco::Path::Style::PATH_URI) << " not avaliable" << std::endl;
            databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOAD_FAILED);
        });
}

void SampleDatabase::playTempSourceFile(int i)
{
    if (i < 0 || i > sourcesList.size())
        return;

    SourceInfo *si = &sourcesList[i];
    Poco::Path location = Poco::Path(si->url);

    if (si->downloaded == DownloadState::DOWNLOADED)
    {
        Poco::File preview = Poco::Path(sourceFolder).append(location);
        sourcePreviewPath = preview.path();
        databaseUpdate.notify(this, DatabaseUpdate::SOURCE_PREVIEW_READY);
        return;
    }

    databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOADING);
    Poco::Path endpoint = Poco::Path("/file").append(location);
    std::cout << endpoint.toString(Poco::Path::Style::PATH_URI) << std::endl;

    httpClient->sendRequest(
        "PlayTempSourceFile",
        WAIVE_SERVER, endpoint.toString(Poco::Path::Style::PATH_URI), [this](const std::string &response)
        {
            Poco::TemporaryFile tmp = Poco::TemporaryFile();
            tmp.keepUntilExit();

            if (!tmp.createFile())
            {
                std::cerr << "Failed to open tmp file" << tmp.path() << std::endl;
                return;
            }

            try {
                Poco::FileOutputStream out = Poco::FileOutputStream(tmp.path());
                out << response;
                out.close();

                if(out.fail()) {
                    throw std::runtime_error("Failed to write response to file: " + tmp.path());
                }
            } catch (std::exception&e) {
                std::cerr << "Error: " << e.what() << std::endl;

            }

            this->latestDownloadedIndex = -1; // to avoid loading preview into SampleEditor
            databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOADED);
            
            this->sourcePreviewPath = tmp.path();
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_PREVIEW_READY); },
        [this, endpoint, si]()
        {
            sleep(1);
            std::cout << WAIVE_SERVER << endpoint.toString(Poco::Path::Style::PATH_URI) << " not avaliable" << std::endl;
            databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOAD_FAILED);
        });
}

void SampleDatabase::makeTagSourcesTable()
{
    // std::cout << "SampleDatabase::makeTagSourcesTable()" << std::endl;

    try
    {
        *session << "DROP TABLE IF EXISTS SourcesTags",
            Poco::Data::Keywords::now;
    }
    catch (const Poco::DataException &e)
    {
        std::cerr << "Error updating SourcesTags: " << e.what() << std::endl;
        return;
    }

    try
    {
        *session << "CREATE TABLE IF NOT EXISTS SourcesTags ("
                    "source_id INT, "
                    "tag_id INT, "
                    "FOREIGN KEY (source_id) REFERENCES Sources(id), "
                    "FOREIGN KEY (tag_id) REFERENCES Tags(id), "
                    "PRIMARY KEY (source_id, tag_id))",
            Poco::Data::Keywords::now;
    }
    catch (const Poco::DataException &e)
    {
        std::cerr << "Error creating SourcesTags: " << e.what() << std::endl;
    }

    // prepare statement for selecting all Sources
    Poco::Data::Statement selectSource(*session);
    std::string tags;
    int sourceId;
    selectSource << "SELECT id, tags FROM Sources",
        Poco::Data::Keywords::into(sourceId),
        Poco::Data::Keywords::into(tags),
        Poco::Data::Keywords::range(0, 1);

    // prepare statement for selecting tags
    Poco::Data::Statement selectTag(*session);
    std::string tag;
    int tagId;
    selectTag << "SELECT id FROM Tags WHERE tag = ?",
        Poco::Data::Keywords::into(tagId),
        Poco::Data::Keywords::use(tag);

    // updating the Sources<->Tags table
    Poco::Data::Statement insertTagSource(*session);
    insertTagSource << "INSERT INTO SourcesTags (source_id, tag_id) VALUES (?, ?)",
        Poco::Data::Keywords::use(sourceId),
        Poco::Data::Keywords::use(tagId);

    session->begin();

    int count = 0;

    try
    {

        while (selectSource.execute())
        {
            if (selectSource.done())
                return;

            Poco::StringTokenizer tokenizer(tags, "|", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
            for (size_t i = 0; i < tokenizer.count(); ++i)
            {
                tag.assign(tokenizer[i]);
                if (selectTag.execute())
                {
                    insertTagSource.execute();
                    count++;
                    if (count % 1000 == 0)
                    {
                        std::cout << "committing at " << count << std::endl;
                        session->commit();
                        session->begin();
                    }
                }
            }
        }
    }
    catch (const Poco::DataException &e)
    {
        std::cerr << "Error populating SourcesTags: " << e.what() << std::endl;
        return;
    }

    if (count % 1000 != 0)
    {
        std::cout << "committing at " << count << std::endl;
        session->commit();
    }

    std::cout << "SampleDatabase::makeTagSourcesTable() finished" << std::endl;
}

void SampleDatabase::getTagList()
{
    // std::cout << "SampleDatabase::getTagList()" << std::endl;
    if (!sourceDatabaseInitialised)
        return;

    tagList.clear();

    try
    {
        Poco::Data::Statement select(*session);
        int tagId;
        std::string tag;
        float embedX, embedY;
        select << "SELECT id, tag, embedX, embedY FROM Tags",
            Poco::Data::Keywords::now;

        Poco::Data::RecordSet rs(select);
        std::size_t rows = rs.rowCount();

        for (std::size_t i = 0; i < rows; ++i)
        {
            tagId = rs.value(0, i).convert<int>();
            tag = rs.value(1, i).convert<std::string>();
            embedX = rs.value(2, i).convert<float>();
            embedY = rs.value(3, i).convert<float>();

            tagList.push_back({tagId, tag, embedX, embedY});
        }
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Failed to fetch tags from database: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Unexpected error in SampleDatabase::getTagsList(): " << e.what() << std::endl;
    }
}

void SampleDatabase::getArchiveList()
{
    std::cout << "SampleDatabase::getArchiveList()" << std::endl;
    archives.clear();

    try
    {
        Poco::Data::Statement select(*session);
        std::string archive;
        select << "SELECT DISTINCT archive from Sources",
            Poco::Data::Keywords::now;

        Poco::Data::RecordSet rs(select);
        std::size_t rows = rs.rowCount();

        for (std::size_t i = 0; i < rows; ++i)
        {
            archive = rs.value(0, i).convert<std::string>();
            archives.push_back(archive);
        }
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Failed to fetch archives from database: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Unexpected error in SampleDatabase::getArchivesList(): " << e.what() << std::endl;
    }

    std::cout << "number of archives: " << archives.size() << std::endl;
}

void SampleDatabase::filterSources()
{
    // std::cout << "SampleDatabase::filterSources()" << std::endl;
    if (!sourceDatabaseInitialised)
        return;

    if (!session)
        return;

    std::string exists;
    try
    {
        *session << "SELECT name FROM sqlite_master WHERE type='table' AND name='Sources'",
            Poco::Data::Keywords::into(exists),
            Poco::Data::Keywords::range(0, 1),
            Poco::Data::Keywords::now;
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Failed to check if 'Sources' table exists: " << e.what() << std::endl;
        return;
    }

    if (exists.empty())
    {
        sourceDatabaseInitialised = false;
        return;
    }

    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_FILTER_START);
    std::lock_guard<std::mutex> lock(sourceListMutex);
    sourcesList.clear();

    std::vector<std::string> conditions = {};
    if (filterConditions.tagIn.length() > 0)
        conditions.push_back("Tags.id IN (" + filterConditions.tagIn + ")");

    if (filterConditions.archiveNotIn.length() > 0)
        conditions.push_back("Sources.archive NOT IN (" + filterConditions.archiveNotIn + ")");

    if (filterConditions.archiveIs.length() > 0)
        conditions.push_back("Sources.archive LIKE \"%" + filterConditions.archiveIs + "%\"");

    if (filterConditions.downloadsOnly)
        conditions.push_back("Sources.downloaded = 1");

    if (filterConditions.searchString.length() > 0)
    {
        conditions.push_back(
            "(Sources.filename LIKE \"%" + filterConditions.searchString + "%\" OR Sources.description LIKE \"%" + filterConditions.searchString + "%\")");
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
        Poco::Data::Statement select(*session);
        std::string filename, archive, description, url, license;
        int id;
        select << "SELECT DISTINCT Sources.id, Sources.filename, Sources.archive, Sources.description, Sources.url, Sources.license "
                  "FROM Sources "
                  "LEFT JOIN SourcesTags ON Sources.id = SourcesTags.source_id "
                  "LEFT JOIN Tags ON Tags.id = SourcesTags.tag_id " +
                      where,
            Poco::Data::Keywords::into(id),
            Poco::Data::Keywords::into(filename),
            Poco::Data::Keywords::into(archive),
            Poco::Data::Keywords::into(description),
            Poco::Data::Keywords::into(url),
            Poco::Data::Keywords::into(license),
            Poco::Data::Keywords::range(0, 1);

        std::cout << "\nSampleDatabase::filterSources select:\n  \""
                  << select.toString() << "\"\n"
                  << std::endl;

        while (!select.done() && select.execute())
        {
            // std::cout << filename << std::endl;

            SourceInfo source;
            source.id = id;
            source.archive = archive;
            source.filename = filename;
            source.description = description;
            source.license = license;
            source.url = url;
            Poco::File sourceFile = getFullSourcePath(source);
            if (sourceFile.exists())
                source.downloaded = DownloadState::DOWNLOADED;
            else
                source.downloaded = DownloadState::NOT_DOWNLOADED;

            Poco::Data::Statement selectSourcesTag(*session);
            int tagId;
            selectSourcesTag << "SELECT tag_id FROM SourcesTags WHERE source_id = ?",
                Poco::Data::Keywords::into(tagId),
                Poco::Data::Keywords::use(id),
                Poco::Data::Keywords::range(0, 1);

            Poco::Data::Statement selectTag(*session);
            std::string tag;
            float embedX, embedY;
            selectTag << "SELECT tag, embedX, embedY FROM Tags WHERE id = ?",
                Poco::Data::Keywords::into(tag),
                Poco::Data::Keywords::into(embedX),
                Poco::Data::Keywords::into(embedY),
                Poco::Data::Keywords::use(tagId),
                Poco::Data::Keywords::range(0, 1);

            while (!selectSourcesTag.done())
            {
                int res = selectSourcesTag.execute();
                if (!res)
                    break;
                selectTag.execute();
                source.tags.push_back({tagId, std::string(tag), embedX, embedY});
            }

            sourcesList.push_back(source);
        }
    }
    catch (const Poco::DataException &e)
    {
        databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_QUERY_ERROR);
        std::cerr << e.displayText() << std::endl;
        return;
    }

    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_FILTER_END);

    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_UPDATED);
}

void SampleDatabase::onTaskStarted(Poco::TaskStartedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();

    if (pTask->name().compare("ParseSourceList") == 0)
    {
    }
    else if (pTask->name().compare("ParseTagsList") == 0)
    {
    }

    pTask->release();
}

void SampleDatabase::onTaskFinished(Poco::TaskFinishedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();

    if (pTask->name().compare("ParseSourceList") == 0)
    {
        if (!pTask->isCancelled())
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOADED);
    }
    else if (pTask->name().compare("ParseTagsList") == 0)
    {
        if (!pTask->isCancelled())
            databaseUpdate.notify(this, DatabaseUpdate::TAG_LIST_DOWNLOADED);
    }

    pTask->release();
}

void SampleDatabase::onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg)
{
    std::cout << "SampleDatabase::onDatabaseChanged " << databaseUpdateString(arg) << std::endl;
    switch (arg)
    {
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADING:
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR:
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADED:
        downloadTagsList();
        break;
    case SampleDatabase::DatabaseUpdate::TAG_LIST_DOWNLOAD_ERROR:
        break;
    case SampleDatabase::DatabaseUpdate::TAG_LIST_DOWNLOADED:
        makeTagSourcesTable();
        databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_READY);
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_READY:
        getTagList();
        getArchiveList();
        databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_ANALYSED);
        filterSources();
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_UPDATED:
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_FILTER_START:
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_FILTER_END:
        break;
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOADING:
        break;
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOADED:
        break;
    case SampleDatabase::DatabaseUpdate::FILE_DOWNLOAD_FAILED:
        break;
    case SampleDatabase::DatabaseUpdate::SAMPLE_ADDED:
    case SampleDatabase::DatabaseUpdate::SAMPLE_DELETED:
    case SampleDatabase::DatabaseUpdate::SAMPLE_UPDATED:
    case SampleDatabase::DatabaseUpdate::SAMPLE_LIST_LOADED:
    case SampleDatabase::DatabaseUpdate::SOURCE_ADDED:
    default:
        break;
    }
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
    // std::string tagString = "";
    // for (int i = 0; i < tags.size(); i++)
    // {
    //     if (tagString.size() > 0)
    //         tagString += "|";
    //     tagString += tags[i].name;
    // }

    // std::cout << tagString << std::endl;
    // return tagString;
}
