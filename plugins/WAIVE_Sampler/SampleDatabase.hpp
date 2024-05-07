#ifndef SAMPLE_DATABASE_HPP
#define SAMPLE_DATABASE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <fmt/core.h>
#include "Envelopes.hpp"

enum ColumnIndex
{
    ID_COLUMN = 0,
    NAME_COLUMN,
    FOLDER_COLUMN,
    EMBEDX_COLUMN,
    EMBEDY_COLUMN,
    WAIVE_COLUMN,
    TAGS_COLUMN,
    SOURCE_COLUMN,
    VOLUME_COLUMN,
    PITCH_COLUMN,
    AMP_ATTACK_COLUMN,
    AMP_DECAY_COLUMN,
    AMP_SUSTAIN_COLUMN,
    AMP_RELEASE_COLUMN,
    SUSTAIN_LENGTH_COLUMN,
    SOURCE_START_COLUMN,
    SOURCE_END_COLUMN,
};

class SampleInfo
{
public:
    SampleInfo(int id, std::string name, std::string path, bool waive);

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
    ADSR_Params adsr;
    float sustainLength;
    uint sourceStart;
    uint sourceEnd;

    bool saved;

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
    std::vector<SampleInfo> getAllSamples();

    bool verbose;

private:
    bool execSQL(std::string);

    char *zErrMsg = 0;
    int rc;

    sqlite3 *db;
};

#endif