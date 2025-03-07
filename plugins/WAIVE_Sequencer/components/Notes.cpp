#include "Notes.hpp"

void printNoteDetails(const std::shared_ptr<Note> &n)
{
    std::cout << "Note " << (n->noteOn ? "ON " : "OFF") << "  Instrument " << n->instrument << ", midiNote " << (int)n->midiNote << ", velocity " << (int)n->velocity << ", channel " << (int)n->channel << ", other " << (n->other == nullptr ? "NULL" : "note") << std::endl;
}

std::string Trigger::serialize() const
{
    std::ostringstream oss;
    oss << tick << "," << static_cast<int>(velocity) << "," << instrument << "," << (user ? "1" : "0") << "," << (active ? "1" : "0");
    return oss.str();
}

Trigger Trigger::deserialize(const std::string &data)
{
    std::istringstream iss(data);
    Trigger t;
    std::string tickStr, velocityStr, instrumentStr, userStr, activeStr;

    if (std::getline(iss, tickStr, ',') &&
        std::getline(iss, velocityStr, ',') &&
        std::getline(iss, instrumentStr, ',') &&
        std::getline(iss, userStr, ',') &&
        std::getline(iss, activeStr, ','))
    {
        t.tick = std::stoi(tickStr);
        t.velocity = std::stoi(velocityStr);
        t.instrument = std::stoi(instrumentStr);
        t.user = (userStr == "1");
        t.active = (activeStr == "1");
        return t;
    }
    t.active = false;
    std::cout << "Trigger::deserialize  Invalid serialization format: " << data << std::endl;
    return t;
    // throw std::runtime_error("Invalid serialization format");
}

bool compareGrooveEvents(GrooveEvent g0, GrooveEvent g1)
{
    if (g0.position < g1.position)
        return true;
    else if (g0.position > g1.position)
        return false;

    return g0.velocity > g1.velocity;
}

bool compareNotes(std::shared_ptr<Note> n0, std::shared_ptr<Note> n1)
{
    if (n0->tick + n0->offset < n1->tick + n1->offset)
        return true;
    else if (n0->tick + n0->offset > n1->tick + n1->offset)
        return false;

    if (!n0->noteOn && n1->noteOn)
        return true;
    else if (n0->noteOn && !n1->noteOn)
        return false;

    return n0->midiNote < n1->midiNote;
}

void writeBigEndian4(std::ofstream &out, uint32_t value)
{
    out.put((value >> 24) & 0xFF);
    out.put((value >> 16) & 0xFF);
    out.put((value >> 8) & 0xFF);
    out.put(value & 0xFF);
}

// Helper function to write big-endian 2-byte integers
void writeBigEndian2(std::ofstream &out, uint16_t value)
{
    out.put((value >> 8) & 0xFF);
    out.put(value & 0xFF);
}

void writeVariableLength(std::ofstream &out, uint32_t value)
{
    uint8_t buffer[4]; // MIDI VLQ supports up to 4 bytes
    int index = 0;

    // Break the value into 7-bit groups
    do
    {
        buffer[index++] = (value & 0x7F); // Get the last 7 bits
        value >>= 7;                      // Shift right by 7 bits
    } while (value > 0);

    // Write the bytes in reverse order with MSB set for all but the last
    for (int i = index - 1; i > 0; --i)
    {
        out.put(buffer[i] | 0x80); // Set MSB for continuation
    }
    out.put(buffer[0]); // Last byte, MSB not set
}

bool exportMidiFile(const std::vector<std::shared_ptr<Note>> &events, const std::string &filename)
{
    // Sort events by time to ensure correct ordering
    auto sortedEvents = events;
    std::sort(sortedEvents.begin(), sortedEvents.end(), [](const std::shared_ptr<Note> &a, const std::shared_ptr<Note> &b)
              { return a->tick < b->tick; });

    // Open file for binary writing
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }

    // Write MIDI header chunk
    outFile.write("MThd", 4);       // Header identifier
    writeBigEndian4(outFile, 6);    // Header size
    writeBigEndian2(outFile, 0);    // Format type (single track)
    writeBigEndian2(outFile, 1);    // Number of tracks
    writeBigEndian2(outFile, 1920); // Division (ticks per quarter note)

    // Write track chunk
    outFile.write("MTrk", 4); // Track identifier
    std::streampos sizePos = outFile.tellp();
    writeBigEndian4(outFile, 0); // Placeholder for track size

    // Write track events
    uint32_t previousTime = 0;
    int count = 0;
    // std::cout << "TRACK EVENTS" << std::endl;

    for (const auto &event : sortedEvents)
    {
        // Write delta time
        uint32_t deltaTime = event->tick - previousTime;
        writeVariableLength(outFile, deltaTime);
        previousTime = event->tick;

        // Write MIDI event
        uint8_t statusByte = (event->noteOn ? 0x90 : 0x80) | (event->channel & 0x0F);
        outFile.put(statusByte);
        outFile.put(event->midiNote);
        outFile.put(event->velocity);

        // std::cout << " Event " << std::hex << count << ":" << std::endl;
        // std::cout << " - Delta:     " << std::hex << deltaTime << std::dec << " (" << deltaTime << ")" << std::endl;
        // std::cout << " - Status:    " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(statusByte) << std::dec << " (" << static_cast<int>(statusByte) << ")" << std::endl;
        // std::cout << " - Midi note: " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(event.midiNote) << std::dec << " (" << static_cast<int>(event.midiNote) << ") " << std::endl;
        // std::cout << " - Velocity:  " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(event.velocity) << std::dec << " (" << static_cast<int>(event.velocity) << ")" << std::endl;

        count++;
    }
    // std::cout << "Track events done" << std::endl;

    // Write End of Track meta-event
    writeVariableLength(outFile, 0); // Delta time
    outFile.put(0xFF);               // Meta-event
    outFile.put(0x2F);               // End of Track type
    outFile.put(0x00);               // Length

    // Update track size
    std::streampos endPos = outFile.tellp();
    uint32_t trackSize = static_cast<uint32_t>(endPos - sizePos - 4);
    // std::cout << "trackSize: " << trackSize << "(endPos: " << endPos << ", sizePos: " << sizePos << ")" << std::endl;
    outFile.seekp(sizePos);
    writeBigEndian4(outFile, trackSize);

    std::cout << "MIDI file written successfully to " << filename << std::endl;

    return true;
}
