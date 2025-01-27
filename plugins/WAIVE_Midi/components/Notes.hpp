#ifndef NOTES_HPP
#define NOTES_HPP

#include <stdint.h>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <iomanip>

// #include <fmt/core.h>

struct GrooveEvent
{
    float position;
    float velocity;
};

/**
   For sorting GrooveEvents structs by time, then by
   velocity.
*/
bool compareGrooveEvents(GrooveEvent g0, GrooveEvent g1);

struct Trigger
{
    uint32_t tick;
    uint8_t velocity;
    int instrument;
    bool user;
};

struct Note
{
    uint32_t tick;
    uint8_t velocity;
    uint8_t midiNote;
    uint8_t channel;
    bool noteOn;
    int instrument = -1;
    bool user = false;
    int32_t offset;
    std::shared_ptr<Note> other;
    std::shared_ptr<Trigger> trigger;
};

/**
   For sorting Note structs by time, noteOn/Off, then by
   midiNote number
*/
bool compareNotes(std::shared_ptr<Note> n0, std::shared_ptr<Note> n1);

void printNoteDetails(const std::shared_ptr<Note> &n);

static uint8_t midiMap[9] = {36, 38, 47, 50, 43, 42, 46, 51, 49};

const int DRUM_CHANNEL = 0;

static char *midiNoteLabels[9] = {
    (char *)"Kick",
    (char *)"Snare",
    (char *)"Mid tom",
    (char *)"High tom",
    (char *)"Percussion",
    (char *)"Hihat closed",
    (char *)"Hihat open",
    (char *)"Ride",
    (char *)"Crash"};

/**
 * Midi Export functionality
 */

void writeBigEndian4(std::ofstream &out, uint32_t value);
void writeBigEndian2(std::ofstream &out, uint16_t value);
void writeVariableLength(std::ofstream &out, uint32_t value);

bool exportMidiFile(const std::vector<std::shared_ptr<Note>> &events, const std::string &filename);

#endif