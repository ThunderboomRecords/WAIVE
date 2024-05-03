#include "SampleDatabase.hpp"

SampleInfo::SampleInfo(
    int id_,
    std::string name_,
    std::string path_,
    bool waive_) : source(""),
                   embedX(0.0f),
                   embedY(0.0f),
                   tags(""),
                   volume(1.0f),
                   pitch(1.0f),
                   sourceStart(0),
                   sourceEnd(0),
                   saved(false)
{
    id = id_;
    name = name_;
    path = path_;
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
                        "pitch          REAL,"
                        "sourceStart    INT,"
                        "sourceEnd      INT"
                        ");";
    return execSQL(query);
}

bool SampleDatabase::insertSample(SampleInfo s)
{
    sqlite3_stmt *stmt;
    const char *query = "INSERT INTO Samples (id, name, folder, embedX, embedY, waive, tags, source, volume, pitch, sourceStart, sourceEnd) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        if (verbose)
        {
            std::cout << "sqlite3_prepare_v2 failed with status " << rc << std::endl;
            std::cout << sqlite3_errstr(rc) << std::endl;
        }
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_bind_int(stmt, 1, s.getId());
    sqlite3_bind_text(stmt, 2, s.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, s.path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, s.embedX);
    sqlite3_bind_double(stmt, 5, s.embedY);
    sqlite3_bind_int(stmt, 6, s.waive);
    sqlite3_bind_text(stmt, 7, s.tags.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, s.source.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 9, s.volume);
    sqlite3_bind_double(stmt, 10, s.pitch);
    sqlite3_bind_int(stmt, 11, s.sourceStart);
    sqlite3_bind_int(stmt, 12, s.sourceEnd);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        if (verbose)
            std::cout << "sqlite3_step failed with status " << rc << std::endl;
        std::cout << sqlite3_errstr(rc) << ": " << sqlite3_errstr(sqlite3_extended_errcode(db)) << std::endl;
    }

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool SampleDatabase::updateSample(SampleInfo s)
{
    sqlite3_stmt *stmt;
    const char *query = "UPDATE Samples SET name=?, folder=?, embedX=?, embedY=?, waive=?, tags=?, source=?, volume=?, pitch=?, sourceStart=?, sourceEnd=? WHERE id=?";

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        if (verbose)
        {
            std::cout << "sqlite3_prepare_v2 failed with status " << rc << std::endl;
            std::cout << sqlite3_errstr(rc) << std::endl;
        }
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_bind_text(stmt, 1, s.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, s.path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, s.embedX);
    sqlite3_bind_double(stmt, 4, s.embedY);
    sqlite3_bind_int(stmt, 5, s.waive);
    sqlite3_bind_text(stmt, 6, s.tags.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, s.source.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 8, s.volume);
    sqlite3_bind_double(stmt, 9, s.pitch);
    sqlite3_bind_int(stmt, 10, s.sourceStart);
    sqlite3_bind_int(stmt, 11, s.sourceEnd);
    sqlite3_bind_int(stmt, 12, s.getId());

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        if (verbose)
            std::cout << "sqlite3_step failed with status " << rc << std::endl;
        std::cout << sqlite3_errstr(rc) << ": " << sqlite3_errstr(sqlite3_extended_errcode(db)) << std::endl;
    }

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
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
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, "SELECT * FROM Samples", -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        if (verbose)
        {
            std::cout << "getAllSamples() sqlite3_prepare_v2 failed with status " << rc << std::endl;
            std::cout << sqlite3_errstr(rc) << std::endl;
        }
        sqlite3_finalize(stmt);
        return {};
    }

    std::vector<SampleInfo> samples;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id(sqlite3_column_int(stmt, ID_COLUMN));
        std::string name(reinterpret_cast<const char*>(sqlite3_column_text(stmt, NAME_COLUMN)));
        std::string path(reinterpret_cast<const char*>(sqlite3_column_text(stmt, FOLDER_COLUMN)));
        bool waive(sqlite3_column_int(stmt, WAIVE_COLUMN));

        SampleInfo s(id, name, path, waive);
        s.embedX = sqlite3_column_double(stmt, EMBEDX_COLUMN);
        s.embedY = sqlite3_column_double(stmt, EMBEDY_COLUMN);
        s.source = reinterpret_cast<const char*>(sqlite3_column_text(stmt, SOURCE_COLUMN));
        s.pitch = sqlite3_column_double(stmt, PITCH_COLUMN);
        s.volume = sqlite3_column_double(stmt, VOLUME_COLUMN);
        s.tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, TAGS_COLUMN));
        s.sourceStart = sqlite3_column_int(stmt, SOURCE_START_COLUMN);
        s.sourceEnd = sqlite3_column_int(stmt, SOURCE_END_COLUMN);

        s.saved = true;

        samples.push_back(s);
    }

    std::cout << "loaded " << samples.size() << " samples" << std::endl;

    return samples;
}