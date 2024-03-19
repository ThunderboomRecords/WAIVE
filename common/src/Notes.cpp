#include "Notes.hpp"

bool compareGrooveEvents(GrooveEvent g0, GrooveEvent g1)
{
    if(g0.position < g1.position) return true;
    else if(g0.position > g1.position) return false;

    return g0.velocity > g1.velocity;
}

bool compareNotes(Note n0, Note n1)
{
    if(n0.tick < n1.tick) return true;
    else if(n0.tick > n1.tick) return false;
    
    if(!n0.noteOn && n1.noteOn) return true;
    else if(n0.noteOn && !n1.noteOn) return false;

    return n0.midiNote < n1.midiNote;
}