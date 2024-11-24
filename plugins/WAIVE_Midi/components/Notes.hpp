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

struct Note
{
    uint32_t tick;
    uint8_t velocity;
    uint8_t midiNote;
    uint8_t channel;
    bool noteOn;
    int instrument = -1;
};

/**
   For sorting Note structs by time, noteOn/Off, then by
   midiNote number
*/
bool compareNotes(Note n0, Note n1);

static uint8_t midiMap[9] = {36, 38, 47, 50, 43, 42, 46, 51, 49};

const int DRUM_CHANNEL = 0;

static char *midiNoteLabels[9] = {
    (char *)"kick",
    (char *)"snare",
    (char *)"mid tom",
    (char *)"high tom",
    (char *)"percussion",
    (char *)"hihat closed",
    (char *)"hihat open",
    (char *)"ride",
    (char *)"crash"};

/**
 * Midi Export functionality
 */

void writeBigEndian4(std::ofstream &out, uint32_t value);
void writeBigEndian2(std::ofstream &out, uint16_t value);
void writeVariableLength(std::ofstream &out, uint32_t value);

bool exportMidiFile(const std::vector<Note> &events, const std::string &filename);

#endif