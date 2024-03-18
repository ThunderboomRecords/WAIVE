#ifndef NOTES_HPP
#define NOTES_HPP

#include <bits/stdc++.h> 

struct Note {
    int tick;
    uint8_t velocity;
    uint8_t midiNote;
    uint8_t channel;
    bool noteOn;
};

static uint8_t midiMap[9] = {36, 38, 47, 50, 43, 42, 46, 51, 49};

static char* labels[9] = {
    (char *)"kick",
    (char *)"snare",
    (char *)"mid tom",
    (char *)"high tom",
    (char *)"percusion",
    (char *)"hihat closed",
    (char *)"hihat open",
    (char *)"ride",
    (char *)"crash"
};


#endif