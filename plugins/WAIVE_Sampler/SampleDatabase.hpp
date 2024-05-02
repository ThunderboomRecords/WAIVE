#ifndef SAMPLE_DATABASE_HPP
#define SAMPLE_DATABASE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <fmt/core.h>

// struct SampleInfo
// {
//     int id;
//     std::string name;
//     std::string path;
//     float embedX;
//     float embedY;
//     bool waive;
//     std::string tags;
//     std::string source;
//     float volume;
//     float pitch;
// };

class SampleInfo
{
public:
    SampleInfo(int id, std::string name, std::string path, float embedX, float embedY, bool waive);

    int getId();

    std::string name;
    std::string path;
    float embedX;
    float embedY;
    bool waive;
    std::string tags;
    std::string source;
    float volume;
    float pitch;

private:
    int id;
};

class SampleDatabase
{
public:
    SampleDatabase(std::string);
    ~SampleDatabase();

    bool createTable();
    bool insertSample(SampleInfo);
    bool updateSample(SampleInfo);
    bool deleteSample(SampleInfo);
    bool findSample(int);
    std::vector<SampleInfo> getAllSamples();

    bool verbose;

private:
    bool execSQL(std::string);

    char *zErrMsg = 0;
    int rc;

    sqlite3 *db;
};

#endif