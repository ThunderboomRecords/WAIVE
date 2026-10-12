#include "SampleDatabase.hpp"

namespace
{
    // Columns of the tables built from the server's TSV files, in the order
    // of the TSV columns. Shared by the startup code and the rebuild, so the
    // tables always have the same schema.
    const std::vector<std::string> SOURCES_COLUMNS = {"id", "description", "tags", "folder", "filename", "archive", "url", "license"};
    const std::vector<std::string> SOURCES_TYPES = {"INTEGER PRIMARY KEY", "TEXT", "TEXT", "TEXT", "TEXT", "TEXT", "TEXT", "TEXT"};
    const std::vector<std::string> TAGS_COLUMNS = {"id", "tag", "embedX", "embedY", "counts"};
    const std::vector<std::string> TAGS_TYPES = {"INTEGER PRIMARY KEY", "TEXT", "FLOAT", "FLOAT", "INTEGER"};
    const std::string SOURCES_TAGS_DEFINITION = "(source_id INT, "
                                                "tag_id INT, "
                                                "FOREIGN KEY (source_id) REFERENCES Sources(id), "
                                                "FOREIGN KEY (tag_id) REFERENCES Tags(id), "
                                                "PRIMARY KEY (source_id, tag_id))";

    std::string createTableQuery(const std::string &table, const std::vector<std::string> &column_names, const std::vector<std::string> &column_types, bool ifNotExists)
    {
        std::ostringstream query;
        query << "CREATE TABLE " << (ifNotExists ? "IF NOT EXISTS " : "") << table << " (";
        for (size_t i = 0; i < column_names.size(); ++i)
        {
            query << column_names[i] << " " << column_types[i];
            if (i < column_names.size() - 1)
                query << ", ";
        }
        query << ")";
        return query.str();
    }
}

SourceInfo::SourceInfo(const SourceInfo &sourceInfo) : id(sourceInfo.id),
                                                       archive(sourceInfo.archive),
                                                       description(sourceInfo.description),
                                                       filename(sourceInfo.filename),
                                                       tags(sourceInfo.tags),
                                                       downloaded(sourceInfo.downloaded),
                                                       license(sourceInfo.license),
                                                       url(sourceInfo.url)
{
}

SampleDatabase::SampleDatabase(std::shared_ptr<HTTPClient> _httpClient)
    : httpClient(std::move(_httpClient)),
      sourcesLoaded(false),
      sourceDatabaseInitialised(true),
      latestDownloadedId(-1),
      session(nullptr)
{
    // Get and create the directory where samples and sound files will
    // be saved to
    Poco::Path homedir = Poco::Path::dataHome();
    rootDir = homedir.append(Poco::Path("WAIVE"));

    Poco::File dir(rootDir);
    if (!dir.exists())
        dir.createDirectories();

    Poco::AutoPtr<Poco::SimpleFileChannel> pChannel(new Poco::SimpleFileChannel);
    pChannel->setProperty("path", Poco::Path(homedir).append("waive.log").toString());

    Poco::Logger::root().setChannel(pChannel);
    logger = &Poco::Logger::get("WAIVESampler::SampleDatabase");

    logger->information(fmt::format("Root directory: {:s}", rootDir.toString()));

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
    dbFilePath = db.path();
    session = std::make_unique<Poco::Data::Session>("SQLite", dbFilePath);

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
        logger->critical(fmt::format("Error initialising Samples table:\n {:s}", e.what()));
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
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Error initialising SamplesTags table: " << e.what() << std::endl;
        logger->critical(fmt::format("Error initialising SamplesTags table:\n {:s}", e.what()));
    }

    // SOURCES Database: create empty tables, so the plugin works (e.g.
    // importing sources) before the first download from the server
    try
    {
        *session << createTableQuery("Sources", SOURCES_COLUMNS, SOURCES_TYPES, true), Poco::Data::Keywords::now;
        *session << createTableQuery("Tags", TAGS_COLUMNS, TAGS_TYPES, true), Poco::Data::Keywords::now;
        *session << "CREATE TABLE IF NOT EXISTS SourcesTags " + SOURCES_TAGS_DEFINITION, Poco::Data::Keywords::now;
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Error initialising Sources tables: " << e.what() << std::endl;
        logger->critical(fmt::format("Error initialising Sources tables:\n {:s}", e.what()));
    }

    databaseUpdate += Poco::delegate(this, &SampleDatabase::onDatabaseChanged);

    std::cout << "SampleDatabase initialised\n";
    logger->information("SampleDatabase initialised");

    loadSampleDatabase();
}

SampleDatabase::~SampleDatabase()
{
    if (session->isTransaction())
        session->commit();

    taskManager.cancelAll();
    taskManager.joinAll();
    session->close();
}

void SampleDatabase::loadSampleDatabase()
{
    std::cout << "\nSampleDatabase::loadSampleDatabase()\n";
    std::unique_lock<std::recursive_mutex> lock(samplesMtx);
    Poco::Data::Statement select(*session);
    try
    {
        select << "SELECT * FROM Samples", Poco::Data::Keywords::now;
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Database query failed: " << e.what() << std::endl;
        logger->critical(fmt::format("Database query failed: {:s}", e.what()));
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

        SamplePoint point(s->embedX, s->embedY);
        point.info = s;
        points.push_back(point);
    }

    kdtree.build(points);
    std::cout << "Number of samples found: " << fAllSamples.size() << std::endl;
    lock.unlock();

    databaseUpdate.notify(this, DatabaseUpdate::SAMPLE_LIST_LOADED);
}

std::shared_ptr<SampleInfo> SampleDatabase::deserialiseSampleInfo(const json &data)
{
    try
    {
        // id, name, path and source come from the table's columns, so they're
        // always there. Anything else may be missing from rows saved by older
        // versions, so falls back to the SampleInfo defaults.
        std::shared_ptr<SampleInfo> s(new SampleInfo(data.at("id"), data.at("name"), data.at("path"), data.value("waive", false)));

        const json embedding = data.value("embedding", json::object());
        s->embedX = embedding.value("x", s->embedX);
        s->embedY = embedding.value("y", s->embedY);

        for (const auto &el : data.value("tags", json::array()))
        {
            if (el.is_string())
                s->tags.push_back({-1, el.get<std::string>(), 0.0f, 0.0f});
        }
        s->tagString = data.value("tagString", s->tagString);
        s->sampleLength = data.value("sampleLength", s->sampleLength);
        s->sourceInfo->fp = data.at("source");
        s->sourceInfo->tagString = s->tagString;
        s->sourceStart = data.value("sourceStart", s->sourceStart);

        const json parameters = data.value("parameters", json::object());
        s->volume = parameters.value("volume", s->volume);
        s->pitch = parameters.value("pitch", s->pitch);
        s->percussiveBoost = parameters.value("percussiveBoost", s->percussiveBoost);

        const json ampEnv = data.value("ampEnv", json::object());
        s->adsr.attack = ampEnv.value("attack", s->adsr.attack);
        s->adsr.decay = ampEnv.value("decay", s->adsr.decay);
        s->adsr.sustain = ampEnv.value("sustain", s->adsr.sustain);
        s->adsr.release = ampEnv.value("release", s->adsr.release);
        s->sustainLength = ampEnv.value("sustainLength", s->sustainLength);

        const json filter = data.value("filter", json::object());
        s->filterCutoff = filter.value("filterCutoff", s->filterCutoff);
        s->filterResonance = filter.value("filterResonance", s->filterResonance);
        s->filterType = filter.value("filterType", s->filterType);

        // Only saved samples are stored, whatever an older version wrote
        s->saved = true;
        s->preset = data.value("preset", s->preset);

        s->fullPath = getFullSamplePath(s);

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

    std::lock_guard<std::recursive_mutex> lock(samplesMtx);

    // Stored as saved, but only marked as saved once it's in the database
    json sampleJson = sample->toJson();
    sampleJson["saved"] = true;
    std::string parameters = sampleJson.dump();

    try
    {
        Poco::Data::Statement insert(*session);
        insert << "INSERT INTO Samples(name, path, source, parameters) VALUES(?, ?, ?, ?)",
            Poco::Data::Keywords::use(sample->name),
            Poco::Data::Keywords::use(sample->path),
            Poco::Data::Keywords::use(sample->sourceInfo->fp),
            Poco::Data::Keywords::use(parameters);
        insert.execute();
    }
    catch (const Poco::Data::DataException &ex)
    {
        std::cerr << "Error inserting new sample into database: " << ex.message() << std::endl;
        return false;
    }

    std::cout << " - insert done" << std::endl;

    unsigned long lastId;
    try
    {
        Poco::Data::Statement lastSampleId(*session);
        lastSampleId << "SELECT last_insert_rowid()",
            Poco::Data::Keywords::into(lastId);
        lastSampleId.execute();
    }
    catch (const Poco::Data::DataException &ex)
    {
        std::cerr << "Error getting new sampleId: " << ex.message() << std::endl;
        return false;
    }

    std::cout << " - lastId " << lastId << std::endl;

    sample->saved = true;
    sample->setId(lastId);
    sample->fullPath = getFullSamplePath(sample);

    // TODO: update Sample<->Tags database
    for (Tag t : sample->tags)
    {
        std::cout << t.name << std::endl;
        newTag(t.name);
    }

    fAllSamples.push_back(sample);

    SamplePoint point(sample->embedX, sample->embedY);
    point.info = sample;
    points.push_back(point);
    kdtree.build(points);
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
    catch (const Poco::FileExistsException &ex)
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

    std::cout << "SampleDatabase::addSourceToLibrary: File copied to sources folder" << std::endl;

    // 2 Update database
    std::string filename = Poco::Path(path).getFileName();
    Poco::Path url = Poco::Path("User").append(filename);

    std::cout << "SampleDatabase::addSourceToLibrary: filename: " << filename << ", url: " << url.toString() << std::endl;

    std::string description = Poco::Path(path).getBaseName();
    std::string urlString = url.toString();

    if (!session)
    {
        std::cerr << "Database session not initialized" << std::endl;
        return false;
    }

    unsigned long id;
    try
    {
        if (session->isTransaction())
            session->commit();

        std::string archive = "User";
        std::string tags = "";

        // The id is the next free one, but at least 200000000 to keep user
        // sources out of the range used by the remote sources. Computing it
        // in the INSERT itself means two imports can never get the same id.
        Poco::Data::Statement insert(*session);
        insert << "INSERT INTO Sources(id, description, tags, archive, filename, url) "
                  "VALUES((SELECT max(COALESCE(MAX(id), 0) + 1, 200000000) FROM Sources), ?, ?, ?, ?, ?)",
            Poco::Data::Keywords::use(description),
            Poco::Data::Keywords::use(tags),
            Poco::Data::Keywords::use(archive),
            Poco::Data::Keywords::use(filename),
            Poco::Data::Keywords::use(urlString);
        insert.execute();

        Poco::Data::Statement lastSourceId(*session);
        lastSourceId << "SELECT last_insert_rowid()",
            Poco::Data::Keywords::into(id);
        lastSourceId.execute();
    }
    catch (const Poco::Data::DataException &ex)
    {
        std::cerr << "Error inserting new source into database: " << ex.displayText() << std::endl;

        // Remove the copy, otherwise importing the file again fails with
        // "File already in database folder"
        try
        {
            Poco::File(Poco::Path(savePath).setFileName(filename)).remove();
        }
        catch (const Poco::Exception &e)
        {
            std::cerr << "Could not remove copied source file: " << e.displayText() << std::endl;
        }
        return false;
    }

    std::cout << "New source id: " << id << std::endl;

    getArchiveList();
    filterConditions.archiveIs = "";
    filterConditions.searchString = "";
    filterConditions.tagIn = "";
    filterSources();

    // 3 Select current Source (and notify UI) to load it
    latestDownloadedId = id;
    databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOADED);

    return true;
}

bool SampleDatabase::updateSample(std::shared_ptr<SampleInfo> sample)
{
    if (sample == nullptr)
        return false;

    // std::cout << "SampleDatabase::updateSample" << std::endl;

    std::lock_guard<std::recursive_mutex> lock(samplesMtx);
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
            Poco::Data::Keywords::use(sample->sourceInfo->fp),
            Poco::Data::Keywords::use(parameters),
            Poco::Data::Keywords::use(id);
        n = update.execute();
    }
    catch (const Poco::Data::DataException &ex)
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
    std::lock_guard<std::recursive_mutex> lock(samplesMtx);
    try
    {
        Poco::Data::Statement insertTag(*session);
        insertTag << "INSERT OR IGNORE INTO Tags (tag) VALUES (?)",
            Poco::Data::Keywords::use(tag);
        insertTag.execute();
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Error adding tag " << tag << ": " << e.displayText() << std::endl;
    }
}

bool SampleDatabase::renameSample(std::shared_ptr<SampleInfo> sample, std::string new_name)
{
    if (sample == nullptr)
        return false;

    Poco::Path newName(new_name);
    if (newName.getExtension().compare("wav") != 0)
        newName.setExtension("wav");

    std::unique_lock<std::recursive_mutex> lock(samplesMtx);

    // Not in the library yet, so there's no file or database row to rename
    if (!sample->saved)
    {
        sample->name.assign(newName.getFileName());
        sample->fullPath = getFullSamplePath(sample);
        return true;
    }

    Poco::File sampleFile(getFullSamplePath(sample));
    Poco::Path newPath(Poco::Path(sampleFolder).append(sample->path).append(newName.getFileName()));

    try
    {
        sampleFile.renameTo(newPath.toString(), Poco::File::OPT_FAIL_ON_OVERWRITE);
    }
    catch (const Poco::FileExistsException &e)
    {
        std::cerr << "Failed to rename sample to " << newPath.toString() << " since it already exists." << std::endl;
        return false;
    }
    catch (const Poco::Exception &e)
    {
        std::cerr << "Failed to rename sample " << sampleFile.path() << ": " << e.displayText() << std::endl;
        return false;
    }

    std::string oldName = sample->name;
    sample->name.assign(newName.getFileName());
    int id = sample->getId();

    try
    {
        if (session->isTransaction())
            session->commit();

        Poco::Data::Statement update(*session);
        update << "UPDATE Samples SET name = ? WHERE id = ?",
            Poco::Data::Keywords::use(sample->name),
            Poco::Data::Keywords::use(id);
        update.execute();
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Error renaming sample in database: " << e.displayText() << std::endl;

        // Put the file back, so it still matches the database
        try
        {
            Poco::File(newPath).renameTo(sampleFile.path());
        }
        catch (const Poco::Exception &e)
        {
            std::cerr << "Could not restore sample file name: " << e.displayText() << std::endl;
        }
        sample->name = oldName;
        return false;
    }

    sample->fullPath = getFullSamplePath(sample);
    lock.unlock();

    databaseUpdate.notify(this, DatabaseUpdate::SAMPLE_UPDATED);

    return true;
}

std::shared_ptr<SampleInfo> SampleDatabase::findSample(int id)
{
    if (id < 0)
        return nullptr;

    std::lock_guard<std::recursive_mutex> lock(samplesMtx);
    auto it = std::find_if(fAllSamples.begin(), fAllSamples.end(),
                           [id](const auto &sample)
                           { return sample && sample->getId() == id; });

    return (it != fAllSamples.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<SampleInfo>> SampleDatabase::findKNearest(float x, float y, int k)
{
    std::lock_guard<std::recursive_mutex> lock(samplesMtx);
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
    std::lock_guard<std::recursive_mutex> lock(samplesMtx);
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

std::string SampleDatabase::getFullSourcePath(std::shared_ptr<SourceInfo> source) const
{
    return Poco::Path(sourceFolder).append(source->url).toString();
}

std::string SampleDatabase::getSampleFolder() const
{
    return sampleFolder.toString();
}

std::string SampleDatabase::getSourceFolder() const
{
    return sourceFolder.toString();
}

const std::string &SampleDatabase::getSourcePreview() const
{
    return sourcePreviewPath;
}

std::string SampleDatabase::getNewSampleName(const std::string &name)
{
    std::cout << "SampleDatabase::getNewSampleName: " << name << std::endl;
    std::lock_guard<std::recursive_mutex> lock(samplesMtx);
    // Finds a unique name of the form "new_sample_X.wav"
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
            basename.resize(pos);
        }
    }

    std::string pattern = basename + "_{}." + file.getExtension();

    // Names in the library that could clash. LIKE treats '_' and '%' in the
    // basename as wildcards, which only finds extra names.
    std::set<std::string> taken;
    try
    {
        std::vector<std::string> names;
        std::string like = basename + "%";
        Poco::Data::Statement select(*session);
        select << "SELECT name FROM Samples WHERE name LIKE ?",
            Poco::Data::Keywords::use(like),
            Poco::Data::Keywords::into(names),
            Poco::Data::Keywords::now;
        taken.insert(names.begin(), names.end());
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Error in getNewSampleName: " << e.what() << std::endl;
    }

    // Also check the folder, so a file that isn't in the database (or was
    // saved by another instance of the plugin) is never overwritten
    auto isFree = [&](const std::string &n)
    {
        return taken.count(n) == 0 && !Poco::File(Poco::Path(sampleFolder).append(n)).exists();
    };

    int suffixCounter = 0;
    while (!isFree(newName))
    {
        suffixCounter++;
        newName = fmt::vformat(pattern, fmt::make_format_args(suffixCounter));
    }

    std::cout << "New name: " << newName << std::endl;

    return newName;
}

bool SampleDatabase::deleteSample(int id)
{
    std::lock_guard<std::recursive_mutex> lock(samplesMtx);
    // delete sample from database
    try
    {
        Poco::Data::Statement deleteSampleStmt(*session);
        deleteSampleStmt << "DELETE FROM Samples WHERE id = ?",
            Poco::Data::Keywords::use(id),
            Poco::Data::Keywords::now;
        deleteSampleStmt.execute();
    }
    catch (const Poco::Data::DataException &e)
    {
        std::cerr << "Error deleting sample from database: " << e.what() << std::endl;
        return false;
    }

    // delete sample from vector
    auto it = std::find_if(fAllSamples.begin(), fAllSamples.end(),
                           [id](const auto &sample)
                           { return sample && sample->getId() == id; });

    if (it == fAllSamples.end())
        return false;

    std::string fp = getFullSamplePath(*it);
    fAllSamples.erase(it);

    points.erase(std::remove_if(points.begin(), points.end(),
                                [id](const SamplePoint &p)
                                { return p.info && p.info->getId() == id; }),
                 points.end());
    kdtree.build(points);

    // delete sample file
    try
    {
        Poco::File file(fp);
        if (file.exists())
            file.remove();
    }
    catch (const Poco::Exception &e)
    {
        std::cerr << "Could not remove sample file " << fp << ": " << e.displayText() << std::endl;
    }

    return true;
}

std::shared_ptr<SampleInfo> SampleDatabase::duplicateSampleInfo(std::shared_ptr<SampleInfo> sample)
{
    if (!sample)
    {
        std::cerr << "Error: nullptr passed to duplicateSampleInfo\n";
        return nullptr;
    }

    std::string duplicateName = getNewSampleName(sample->name);
    auto s = std::make_shared<SampleInfo>(*sample); // Use copy constructor

    s->setId(-1); // Reset ID for the duplicated sample
    s->name = duplicateName;

    std::cout << "SampleDatabase::duplicateSampleInfo: s->tagString = "
              << s->tagString << ", sample->tagString = " << sample->tagString << std::endl;

    return s;
}

void SampleDatabase::checkLatestRemoteVersion()
{
    // Only one update at a time (e.g. if the UI is reopened during an update)
    if (updatingSources.exchange(true))
        return;

    int user_version;
    std::string exists;
    bool complete = false;
    try
    {
        *session << "PRAGMA user_version",
            Poco::Data::Keywords::into(user_version),
            Poco::Data::Keywords::now;

        // check if "Sources" table exists
        *session << "SELECT name FROM sqlite_master WHERE type='table' AND name='Sources'",
            Poco::Data::Keywords::into(exists),
            Poco::Data::Keywords::range(0, 1),
            Poco::Data::Keywords::now;

        // Older versions could save the new user_version and then fail part
        // way through the update, so also check the remote data is there
        int tables = 0;
        *session << "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name IN ('Sources', 'Tags', 'SourcesTags')",
            Poco::Data::Keywords::into(tables),
            Poco::Data::Keywords::now;

        if (tables == 3)
        {
            int remoteSources = 0, tags = 0;
            *session << "SELECT COUNT(*) FROM Sources WHERE archive != 'User'",
                Poco::Data::Keywords::into(remoteSources),
                Poco::Data::Keywords::now;
            *session << "SELECT COUNT(*) FROM Tags",
                Poco::Data::Keywords::into(tags),
                Poco::Data::Keywords::now;
            complete = remoteSources > 0 && tags > 0;
        }
    }
    catch (const Poco::Exception &e)
    {
        std::cerr << "Failed to read database version: " << e.displayText() << std::endl;
        logger->critical(fmt::format("Failed to read database version: {:s}", e.displayText()));
        updatingSources = false;
        return;
    }

    sourceDatabaseInitialised = !exists.empty();

    // If the source data is missing or incomplete, always download, whatever
    // the stored version
    if (!complete)
        user_version = 0;

    logger->information(fmt::format("Current user_version: {:d}", user_version));
    std::cout << "Current user_version: " << user_version << std::endl;

    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_CHECKING_UPDATE);

    logger->information(fmt::format("Sending request to: {:s}/latest", WAIVE_SERVER));
    httpClient->sendRequest(
        "CheckDatabaseVersion",
        WAIVE_SERVER,
        "/latest",
        [this, user_version](const std::string &response)
        {
            int latest_version;
            try
            {
                json result = json::parse(response);
                latest_version = result.at("version").get<int>();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Invalid response from /latest: " << e.what() << std::endl;
                logger->error(fmt::format("Invalid response from /latest: {:s}", e.what()));
                databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR);
                finishSourceUpdate();
                return;
            }

            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_CHECKED_UPDATE);

            std::cout << "latest_version: " << latest_version << ", user_version: " << user_version << std::endl;
            logger->information(fmt::format("Latest user_version: {:d}", latest_version));
            if (latest_version == user_version)
            {
                finishSourceUpdate();
                return;
            }

            downloadSourcesList(latest_version);
        },
        [this](const std::string &response)
        {
            std::cout << "cannot connect to remote database and verify if up-to-date." << std::endl;
            if (!sourceDatabaseInitialised)
                std::cout << "no Source info avaliable\n";
            logger->error(fmt::format("Cannot connect to remote database and verify if up-to-date. Response:\n{:s}", response));
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR);
            finishSourceUpdate();
        });
}

void SampleDatabase::finishSourceUpdate()
{
    updatingSources = false;
    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_READY);
}

void SampleDatabase::downloadSourcesList(int new_version)
{
    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOADING);

    logger->information(fmt::format("Sending request to: {:s}/filelist", WAIVE_SERVER));
    httpClient->sendRequest(
        "DownloadSourceList",
        WAIVE_SERVER,
        "/filelist",
        [this, new_version](const std::string &response)
        { downloadTagsList(new_version, response); },
        [this](const std::string &response)
        {
            std::cout << WAIVE_SERVER << "/filelist not avaliable" << std::endl;
            logger->error(fmt::format("/filelist not avaliable. Response:\n{:s}", response));
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR);
            finishSourceUpdate();
        });
}

void SampleDatabase::downloadTagsList(int new_version, const std::string &sourcesTSV)
{
    logger->information(fmt::format("Sending request to: {:s}/tags", WAIVE_SERVER));
    httpClient->sendRequest(
        "DownloadTagsList",
        WAIVE_SERVER,
        "/tags",
        [this, new_version, sourcesTSV](const std::string &response)
        {
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOADED);
            rebuildSourceDatabase(new_version, sourcesTSV, response);
        },
        [this](const std::string &response)
        {
            std::cout << WAIVE_SERVER << "/tags not avaliable" << std::endl;
            logger->error(fmt::format("/tags not avaliable. Response:\n{:s}", response));
            databaseUpdate.notify(this, DatabaseUpdate::TAG_LIST_DOWNLOAD_ERROR);
            finishSourceUpdate();
        });
}

void SampleDatabase::rebuildSourceDatabase(int new_version, const std::string &sourcesTSV, const std::string &tagsTSV)
{
    // Runs on an HTTP task thread, so use a separate connection to the
    // database. Everything happens in one transaction: if any step fails,
    // the existing tables and user_version are left untouched and the
    // update is retried next time.
    databaseUpdate.notify(this, DatabaseUpdate::PARSING_CSV_START);

    bool success = false;
    try
    {
        Poco::Data::Session db("SQLite", dbFilePath);
        db.begin();
        try
        {
            // Sources: build a new table, keep the user's own sources, then swap
            // it in. Rebuilding (rather than deleting rows) also picks up any
            // change to the table's columns.
            std::string exists;
            db << "SELECT name FROM sqlite_master WHERE type='table' AND name='Sources'",
                Poco::Data::Keywords::into(exists),
                Poco::Data::Keywords::range(0, 1),
                Poco::Data::Keywords::now;

            db << "DROP TABLE IF EXISTS Sources_new", Poco::Data::Keywords::now;
            size_t nSources = parseTSV(
                db,
                "Sources_new",
                SOURCES_COLUMNS,
                SOURCES_TYPES,
                sourcesTSV);

            if (!exists.empty())
            {
                // Source ids are only used at runtime (samples refer to their
                // source by file path), so a user source whose id is now taken
                // by a remote source is given a new id (NULL -> next rowid)
                db << "INSERT INTO Sources_new (id, description, tags, archive, filename, url) "
                      "SELECT CASE WHEN id IN (SELECT id FROM Sources_new) THEN NULL ELSE id END, "
                      "description, tags, archive, filename, url FROM Sources WHERE archive = 'User'",
                    Poco::Data::Keywords::now;
                db << "DROP TABLE Sources", Poco::Data::Keywords::now;
            }
            db << "ALTER TABLE Sources_new RENAME TO Sources", Poco::Data::Keywords::now;

            // Tags are entirely remote, so replace the table
            db << "DROP TABLE IF EXISTS Tags", Poco::Data::Keywords::now;
            size_t nTags = parseTSV(
                db,
                "Tags",
                TAGS_COLUMNS,
                TAGS_TYPES,
                tagsTSV);

            makeTagSourcesTable(db);

            db << "PRAGMA user_version = " + std::to_string(new_version), Poco::Data::Keywords::now;
            db.commit();

            logger->information(fmt::format("Source database updated to version {:d}: {:d} sources, {:d} tags", new_version, nSources, nTags));
            std::cout << "Source database updated to version " << new_version << std::endl;
            success = true;
        }
        catch (...)
        {
            if (db.isTransaction())
                db.rollback();
            throw;
        }
    }
    catch (const Poco::Exception &e)
    {
        std::cerr << "Failed to update source database: " << e.displayText() << std::endl;
        logger->critical(fmt::format("Failed to update source database: {:s}", e.displayText()));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to update source database: " << e.what() << std::endl;
        logger->critical(fmt::format("Failed to update source database: {:s}", e.what()));
    }

    databaseUpdate.notify(this, DatabaseUpdate::PARSING_CSV_END);

    if (success)
        sourceDatabaseInitialised = true;
    else
        databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR);

    finishSourceUpdate();
}

std::shared_ptr<SourceInfo> SampleDatabase::getSourceById(int id)
{
    if (id < 0)
        return nullptr;

    std::lock_guard<std::mutex> lock(sourceListMutex);
    auto it = std::find_if(sourcesList.begin(), sourcesList.end(),
                           [id](const auto &source)
                           { return source && source->id == id; });

    return (it != sourcesList.end()) ? *it : nullptr;
}

size_t SampleDatabase::parseTSV(
    Poco::Data::Session &db,
    const std::string &table,
    const std::vector<std::string> &column_names,
    const std::vector<std::string> &column_type,
    const std::string &csvData)
{
    // Inserts the rows of a TSV file (with a header line) into a new table.
    // Throws if the data looks invalid, e.g. an error page or a truncated
    // download; the caller's transaction is responsible for rolling back.
    std::cout << "\nSampleDatabase::parseTSV for table " << table << std::endl;

    db << createTableQuery(table, column_names, column_type, false), Poco::Data::Keywords::now;

    std::ostringstream insertQuery;
    insertQuery << "REPLACE INTO " << table << " (";
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

    std::istringstream lines(csvData);
    std::string line;

    // Header line
    if (!std::getline(lines, line))
        throw std::runtime_error(fmt::format("No data for table {:s}", table));
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    if (Poco::StringTokenizer(line, "\t").count() != column_names.size())
        throw std::runtime_error(fmt::format("Unexpected header for table {:s}: {:s}", table, line.substr(0, 200)));

    size_t rows = 0;
    size_t skipped = 0;
    while (std::getline(lines, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            continue;

        Poco::StringTokenizer lineTokenizer(line, "\t");
        if (lineTokenizer.count() != column_names.size())
        {
            std::cout << "Skipping row (" << lineTokenizer.count() << " columns, expected " << column_names.size() << "): " << line << std::endl;
            skipped++;
            continue;
        }

        Poco::Data::Statement insertStmt(db);
        insertStmt << insertQuery.str();
        for (size_t j = 0; j < lineTokenizer.count(); ++j)
            insertStmt, Poco::Data::Keywords::use(lineTokenizer[j]);
        insertStmt.execute();
        rows++;
    }

    std::cout << "  " << rows << " rows inserted, " << skipped << " skipped" << std::endl;
    if (skipped > 0)
        logger->warning(fmt::format("parseTSV {:s}: skipped {:d} malformed rows", table, skipped));

    if (rows == 0)
        throw std::runtime_error(fmt::format("No valid rows for table {:s}", table));

    return rows;
}

void SampleDatabase::downloadSourceFile(int index)
{
    if (index < 0 || index >= sourcesList.size())
        return;

    std::lock_guard<std::mutex> lock(sourceListMutex);
    std::shared_ptr<SourceInfo> si = sourcesList[index];

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

    logger->information(fmt::format("Sending request to: {:s}{:s}", WAIVE_SERVER, endpoint.toString(Poco::Path::Style::PATH_URI)));
    httpClient->sendRequest(
        "DownloadSourceFile",
        WAIVE_SERVER,
        endpoint.toString(Poco::Path::Style::PATH_URI),
        [this, filePath, si](const std::string &response)
        {
            // save to a temporary file first, so a failed write never leaves
            // a partial file that looks downloaded
            Poco::File part(filePath + ".part");
            try
            {
                Poco::File(Poco::Path(filePath).makeParent()).createDirectories();

                Poco::FileOutputStream out(part.path());
                out << response;
                out.close();
                if (out.fail())
                    throw std::runtime_error("Failed to write " + part.path());

                part.renameTo(filePath);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Cannot save download to file " << filePath << ": " << e.what() << std::endl;
                logger->error(fmt::format("Cannot save download to file {:s}: {:s}", filePath, e.what()));
                try
                {
                    if (part.exists())
                        part.remove();
                }
                catch (const std::exception &)
                {
                }
                si->downloaded = DownloadState::NOT_DOWNLOADED;
                databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOAD_FAILED);
                return;
            }

            Poco::Thread::sleep(1);
            si->downloaded = DownloadState::DOWNLOADED;
            this->latestDownloadedId = si->id;
            databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOADED); },
        [this, endpoint, si](const std::string &response)
        {
            Poco::Thread::sleep(1);
            si->downloaded = DownloadState::NOT_DOWNLOADED;
            std::cout << WAIVE_SERVER << endpoint.toString(Poco::Path::Style::PATH_URI) << " not avaliable" << std::endl;
            databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOAD_FAILED);
            logger->error(fmt::format("{:s} not avaliable. Response:\n{:s}", endpoint.toString(Poco::Path::Style::PATH_URI), response));
        });
}

void SampleDatabase::playTempSourceFile(int index)
{
    if (index < 0 || index >= sourcesList.size())
        return;

    std::lock_guard<std::mutex> lock(sourceListMutex);
    std::shared_ptr<SourceInfo> si = sourcesList[index];
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
    // std::cout << endpoint.toString(Poco::Path::Style::PATH_URI) << std::endl;

    logger->information(fmt::format("Sending request to: {:s}{:s}", WAIVE_SERVER, endpoint.toString(Poco::Path::Style::PATH_URI)));
    httpClient->sendRequest(
        "PlayTempSourceFile",
        WAIVE_SERVER,
        endpoint.toString(Poco::Path::Style::PATH_URI),
        [this](const std::string &response)
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

            this->latestDownloadedId = -1; // to avoid loading preview into SampleEditor
            databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOADED);
            
            this->sourcePreviewPath = tmp.path();
            databaseUpdate.notify(this, DatabaseUpdate::SOURCE_PREVIEW_READY); },
        [this, endpoint, si](const std::string &response)
        {
            Poco::Thread::sleep(1);
            std::cout << WAIVE_SERVER << endpoint.toString(Poco::Path::Style::PATH_URI) << " not avaliable" << std::endl;
            databaseUpdate.notify(this, DatabaseUpdate::FILE_DOWNLOAD_FAILED);
            logger->error(fmt::format("{:s} not avaliable. Response:\n{:s}", endpoint.toString(Poco::Path::Style::PATH_URI), response));
        });
}

void SampleDatabase::makeTagSourcesTable(Poco::Data::Session &db)
{
    // Rebuilds the Sources<->Tags table. Runs inside the caller's
    // transaction and throws on failure.
    databaseUpdate.notify(this, DatabaseUpdate::BUILDING_TAG_LIST_START);

    try
    {
        db << "DROP TABLE IF EXISTS SourcesTags", Poco::Data::Keywords::now;
        db << "CREATE TABLE SourcesTags " + SOURCES_TAGS_DEFINITION, Poco::Data::Keywords::now;

        std::vector<int> tagIds;
        std::vector<std::string> tagNames;
        db << "SELECT id, tag FROM Tags",
            Poco::Data::Keywords::into(tagIds),
            Poco::Data::Keywords::into(tagNames),
            Poco::Data::Keywords::now;

        std::unordered_map<std::string, int> tagLookup;
        for (size_t i = 0; i < tagIds.size(); ++i)
            tagLookup[tagNames[i]] = tagIds[i];

        std::vector<int> sourceIds;
        std::vector<std::string> sourceTags;
        db << "SELECT id, tags FROM Sources",
            Poco::Data::Keywords::into(sourceIds),
            Poco::Data::Keywords::into(sourceTags),
            Poco::Data::Keywords::now;

        std::vector<int> linkSourceIds;
        std::vector<int> linkTagIds;
        for (size_t i = 0; i < sourceIds.size(); ++i)
        {
            Poco::StringTokenizer tokenizer(sourceTags[i], "|", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
            for (size_t j = 0; j < tokenizer.count(); ++j)
            {
                auto it = tagLookup.find(tokenizer[j]);
                if (it == tagLookup.end())
                    continue;
                linkSourceIds.push_back(sourceIds[i]);
                linkTagIds.push_back(it->second);
            }
        }

        // OR IGNORE: a source may list the same tag twice
        if (!linkSourceIds.empty())
        {
            db << "INSERT OR IGNORE INTO SourcesTags (source_id, tag_id) VALUES (?, ?)",
                Poco::Data::Keywords::use(linkSourceIds),
                Poco::Data::Keywords::use(linkTagIds),
                Poco::Data::Keywords::now;
        }

        std::cout << "SampleDatabase::makeTagSourcesTable() finished, " << linkSourceIds.size() << " links" << std::endl;
    }
    catch (...)
    {
        databaseUpdate.notify(this, DatabaseUpdate::BUILDING_TAG_LIST_END);
        throw;
    }

    databaseUpdate.notify(this, DatabaseUpdate::BUILDING_TAG_LIST_END);
}

void SampleDatabase::getTagList()
{
    // std::cout << "SampleDatabase::getTagList()" << std::endl;
    if (!sourceDatabaseInitialised)
        return;

    if (session->isTransaction())
        session->commit();

    tagList.clear();

    try
    {
        Poco::Data::Statement select(*session);
        select << "SELECT id, tag, embedX, embedY FROM Tags",
            Poco::Data::Keywords::now;

        Poco::Data::RecordSet rs(select);
        std::size_t rows = rs.rowCount();

        for (std::size_t i = 0; i < rows; ++i)
        {
            int tagId = rs.value(0, i).convert<int>();
            std::string tag = rs.value(1, i).convert<std::string>();
            float embedX = rs.value(2, i).convert<float>();
            float embedY = rs.value(3, i).convert<float>();

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
        select << "SELECT DISTINCT archive from Sources",
            Poco::Data::Keywords::now;

        Poco::Data::RecordSet rs(select);
        std::size_t rows = rs.rowCount();

        for (std::size_t i = 0; i < rows; ++i)
        {
            std::string archive = rs.value(0, i).convert<std::string>();
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
    databaseUpdate.notify(this, DatabaseUpdate::SOURCE_LIST_ANALYSED);
}

void SampleDatabase::filterSources()
{
    // std::cout << "SampleDatabase::filterSources()" << std::endl;
    if (!sourceDatabaseInitialised)
        return;

    if (!session)
        return;

    if (session->isTransaction())
        session->commit();

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

            // SourceInfo source;
            auto source = std::make_shared<SourceInfo>();
            source->id = id;
            source->archive = archive;
            source->filename = filename;
            source->description = description;
            source->license = license;
            source->url = url;
            Poco::File sourceFile = getFullSourcePath(source);
            if (sourceFile.exists())
                source->downloaded = DownloadState::DOWNLOADED;
            else
                source->downloaded = DownloadState::NOT_DOWNLOADED;

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
                source->tags.push_back({tagId, std::string(tag), embedX, embedY});
            }

            sourcesList.push_back(source);
        }
    }
    catch (const Poco::Data::DataException &e)
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
    // std::cout << "SampleDatabase::onDatabaseChanged " << databaseUpdateString(arg) << std::endl;
    switch (arg)
    {
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADING:
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOAD_ERROR:
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_DOWNLOADED:
        break;
    case SampleDatabase::DatabaseUpdate::TAG_LIST_DOWNLOAD_ERROR:
        break;
    case SampleDatabase::DatabaseUpdate::SOURCE_LIST_READY:
        getTagList();
        getArchiveList();
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
