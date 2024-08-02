#ifndef NOTES_HPP
#define NOTES_HPP

#include <stdint.h>
#include <map>

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
    int tick;
    uint8_t velocity;
    uint8_t midiNote;
    uint8_t channel;
    bool noteOn;
};

/**
   For sorting Note structs by time, noteOn/Off, then by
   midiNote number
*/
bool compareNotes(Note n0, Note n1);

static uint8_t midiMap[9] = {36, 38, 47, 50, 43, 42, 46, 51, 49};

static char *midiNoteLabels[9] = {
    (char *)"kick",
    (char *)"snare",
    (char *)"mid tom",
    (char *)"high tom",
    (char *)"percusion",
    (char *)"hihat closed",
    (char *)"hihat open",
    (char *)"ride",
    (char *)"crash"};

#endif