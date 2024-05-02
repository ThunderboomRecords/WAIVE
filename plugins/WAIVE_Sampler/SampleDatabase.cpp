#include "SampleDatabase.hpp"

SampleInfo::SampleInfo(int id_, std::string name_, std::string path_, float embedX_, float embedY_, bool waive_)
{
    id = id_;
    name = name_;
    path = path_;

    embedX = embedX_;
    embedY = embedY_;

    waive = waive_;
}

int SampleInfo::getId()
{
    return id;
}

SampleDatabase::SampleDatabase(std::string db_file)
{
    rc = sqlite3_open(db_file.c_str(), &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n");
        createTable();
    }
}

SampleDatabase::~SampleDatabase()
{
    sqlite3_close(db);
}

int SQLCallback(void *data, int argc, char **argv, char **azColName)
{
    fprintf(stderr, "%s: ", (const char *)data);

    for (int i = 0; i < argc; i++)
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

    printf("\n");
    return 0;
}

bool SampleDatabase::execSQL(std::string query)
{
    if (verbose)
    {
        std::cout << query << std::endl;
        rc = sqlite3_exec(db, query.c_str(), SQLCallback, 0, &zErrMsg);
    }
    else
    {
        rc = sqlite3_exec(db, query.c_str(), NULL, 0, &zErrMsg);
    }

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }
    else
    {
        if (verbose)
            fprintf(stdout, "execSQL completed successfully.\n");
        return true;
    }
}

bool SampleDatabase::createTable()
{
    std::string query = "CREATE TABLE IF NOT EXISTS Samples ("
                        "id INT PRIMARY KEY     NOT NULL,"
                        "name           TEXT    NOT NULL,"
                        "folder         TEXT    NOT NULL,"
                        "embedX         REAL    NOT NULL,"
                        "embedY         REAL    NOT NULL,"
                        "waive          BOOL    NOT NULL,"
                        "tags           TEXT,"
                        "source         TEXT,"
                        "volume         REAL,"
                        "pitch          REAL "
                        ");";
    return execSQL(query);
}

bool SampleDatabase::insertSample(SampleInfo s)
{
    std::string query = "INSERT INTO Samples (id,name,folder,embedX,embedY,waive,tags,source,volume,pitch) ";
    std::string data = fmt::format(
        "VALUES ({:d},'{}','{}',{},{},{},'{}','{}',{},{});",
        s.getId(),
        s.name,
        s.path,
        s.embedX,
        s.embedY,
        s.waive,
        s.tags,
        s.source,
        s.volume,
        s.pitch);

    return execSQL(query + data);
}

bool SampleDatabase::updateSample(SampleInfo s)
{
    std::string format = "UPDATE Samples SET"
                         "name='{}',"
                         "folder='{}',"
                         "embedX={},"
                         "embedY={},"
                         "waive={},"
                         "tags='{}',"
                         "source='{}',"
                         "volume={},"
                         "pitch={}"
                         "WHERE id={}";

    std::string query = fmt::format(format, s.name, s.path, s.embedX, s.embedY, s.waive, s.source, s.volume, s.pitch, s.getId());

    return execSQL(query);
}

bool SampleDatabase::deleteSample(SampleInfo s)
{
    std::string query = fmt::format("DELETE FROM Samples WHERE id={}", s.getId());

    return execSQL(query);
}

bool SampleDatabase::findSample(int id)
{
    return false;
}

std::vector<SampleInfo> SampleDatabase::getAllSamples()
{
    return {};
}