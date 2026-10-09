#include "SourceCache.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <thread>

#include "Poco/DigestEngine.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/Process.h"
#include "Poco/SHA1Engine.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"

namespace SourceCache
{
    namespace
    {
        // Least recently used entries are deleted above this total size.
        // A 3 minute source at 48 kHz is about 35 MB of resampled audio.
        const Poco::File::FileSize MAX_CACHE_BYTES = 1024ull * 1024 * 1024;

        const char AUDIO_MAGIC[4] = {'W', 'S', 'C', 'A'};
        const char ANALYSIS_MAGIC[4] = {'W', 'S', 'C', 'F'};
        const uint32_t FORMAT_VERSION = 1;

        struct AudioHeader
        {
            char magic[4];
            uint32_t version;
            uint64_t frames;
        };

        struct AnalysisHeader
        {
            char magic[4];
            uint32_t version;
            uint32_t measurementSize;
            uint32_t reserved;
            uint64_t measurements;
            uint64_t features;
        };

        Poco::Path cacheDir()
        {
            // makeDirectory(): cacheHome() returns $XDG_CACHE_HOME as is, which
            // usually has no trailing slash, so would otherwise be read as a file name
            Poco::Path dir(Poco::Path::cacheHome());
            dir.makeDirectory();
            dir.pushDirectory("WAIVE");
            dir.pushDirectory("SourceCache");
            return dir;
        }

        Poco::Path entryPath(const std::string &key, const std::string &extension)
        {
            return Poco::Path(cacheDir(), key + extension);
        }

        void evict()
        {
            std::vector<Poco::File> entries;
            Poco::File::FileSize total = 0;
            Poco::Timestamp now;

            for (Poco::DirectoryIterator it(cacheDir()), end; it != end; ++it)
            {
                if (!it->isFile())
                    continue;

                // Leftover partial write, e.g. from a crash
                if (it.name().find(".tmp-") != std::string::npos)
                {
                    if (now - it->getLastModified() > Poco::Timespan::HOURS)
                        it->remove();
                    continue;
                }

                total += it->getSize();
                entries.push_back(*it);
            }

            if (total <= MAX_CACHE_BYTES)
                return;

            std::sort(entries.begin(), entries.end(), [](const Poco::File &a, const Poco::File &b)
                      { return a.getLastModified() < b.getLastModified(); });

            for (Poco::File &entry : entries)
            {
                if (total <= MAX_CACHE_BYTES)
                    break;
                try
                {
                    Poco::File::FileSize size = entry.getSize();
                    entry.remove();
                    total -= size;
                }
                catch (const std::exception &e)
                {
                    // e.g. on Windows, a file another instance has open
                    std::cerr << "SourceCache: could not remove " << entry.path() << ": " << e.what() << std::endl;
                }
            }
        }

        // Writes to a temporary file and renames it into place, so a reader
        // never sees a partial entry
        void writeEntry(const Poco::Path &path, const std::function<void(std::ostream &)> &write)
        {
            Poco::File(cacheDir()).createDirectories();

            std::string tmp = path.toString() + ".tmp-" + std::to_string(Poco::Process::id()) + "-" +
                              std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            {
                std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
                write(out);
                out.flush();
                if (!out)
                {
                    out.close();
                    Poco::File(tmp).remove();
                    throw std::runtime_error("failed to write " + tmp);
                }
            }
            Poco::File(tmp).renameTo(path.toString());

            evict();
        }

        // Opens an entry, checking it exists and has exactly the expected
        // size once the header is read. Marks the entry as recently used.
        template <typename Header>
        bool openEntry(const Poco::Path &path, std::ifstream &in, Header &header, const char (&magic)[4])
        {
            Poco::File file(path);
            if (!file.exists())
                return false;

            in.open(path.toString(), std::ios::binary);
            if (!in.read(reinterpret_cast<char *>(&header), sizeof(header)))
                return false;
            if (std::memcmp(header.magic, magic, 4) != 0 || header.version != FORMAT_VERSION)
                return false;

            file.setLastModified(Poco::Timestamp());
            return true;
        }
    }

    std::string makeKey(const std::string &filePath, const std::string &variant)
    {
        try
        {
            if (filePath.empty())
                return "";

            // Temporary files (e.g. source previews) are never loaded again
            std::string temp = Poco::Path::temp();
            if (filePath.compare(0, temp.size(), temp) == 0)
                return "";

            Poco::File file(filePath);
            if (!file.exists() || !file.isFile())
                return "";

            Poco::SHA1Engine sha1;
            sha1.update(Poco::Path(filePath).absolute().toString());
            sha1.update('\n');
            sha1.update(std::to_string(file.getSize()));
            sha1.update('\n');
            sha1.update(std::to_string(file.getLastModified().epochMicroseconds()));
            sha1.update('\n');
            sha1.update(variant);
            return Poco::DigestEngine::digestToHex(sha1.digest());
        }
        catch (const std::exception &e)
        {
            std::cerr << "SourceCache: could not make key for " << filePath << ": " << e.what() << std::endl;
            return "";
        }
    }

    bool readAudio(const std::string &key, std::vector<float> &buffer)
    {
        if (key.empty())
            return false;

        try
        {
            Poco::Path path = entryPath(key, ".audio");
            std::ifstream in;
            AudioHeader header;
            if (!openEntry(path, in, header, AUDIO_MAGIC))
                return false;

            if (Poco::File(path).getSize() != sizeof(header) + header.frames * sizeof(float))
                return false;

            std::vector<float> data(header.frames);
            if (!in.read(reinterpret_cast<char *>(data.data()), data.size() * sizeof(float)))
                return false;

            buffer.swap(data);
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "SourceCache: could not read audio entry " << key << ": " << e.what() << std::endl;
            return false;
        }
    }

    void writeAudio(const std::string &key, const std::vector<float> &buffer)
    {
        if (key.empty())
            return;

        try
        {
            writeEntry(entryPath(key, ".audio"), [&](std::ostream &out)
                       {
                AudioHeader header = {};
                std::memcpy(header.magic, AUDIO_MAGIC, 4);
                header.version = FORMAT_VERSION;
                header.frames = buffer.size();
                out.write(reinterpret_cast<const char *>(&header), sizeof(header));
                out.write(reinterpret_cast<const char *>(buffer.data()), buffer.size() * sizeof(float)); });
        }
        catch (const std::exception &e)
        {
            std::cerr << "SourceCache: could not write audio entry " << key << ": " << e.what() << std::endl;
        }
    }

    bool readAnalysis(const std::string &key, std::vector<WaveformFeature> &features, std::vector<WaveformMeasurements> &measurements)
    {
        if (key.empty())
            return false;

        try
        {
            Poco::Path path = entryPath(key, ".analysis");
            std::ifstream in;
            AnalysisHeader header;
            if (!openEntry(path, in, header, ANALYSIS_MAGIC))
                return false;

            if (header.measurementSize != sizeof(WaveformMeasurements))
                return false;

            // Guard against a corrupt count before allocating
            Poco::File::FileSize fileSize = Poco::File(path).getSize();
            if (header.measurements * sizeof(WaveformMeasurements) > fileSize)
                return false;

            std::vector<WaveformMeasurements> m(header.measurements);
            if (!in.read(reinterpret_cast<char *>(m.data()), m.size() * sizeof(WaveformMeasurements)))
                return false;

            std::vector<WaveformFeature> f;
            for (uint64_t i = 0; i < header.features; i++)
            {
                int32_t type, index;
                float value;
                int64_t start, end;
                uint32_t labelLength;
                in.read(reinterpret_cast<char *>(&type), sizeof(type));
                in.read(reinterpret_cast<char *>(&value), sizeof(value));
                in.read(reinterpret_cast<char *>(&start), sizeof(start));
                in.read(reinterpret_cast<char *>(&end), sizeof(end));
                in.read(reinterpret_cast<char *>(&index), sizeof(index));
                in.read(reinterpret_cast<char *>(&labelLength), sizeof(labelLength));
                if (!in || labelLength > 1024)
                    return false;
                std::string label(labelLength, '\0');
                if (!in.read(label.data(), labelLength))
                    return false;
                f.push_back({static_cast<FeatureType>(type), label, value, static_cast<long>(start), static_cast<long>(end), index});
            }

            // Must be at the end of the file
            if (in.peek() != std::ifstream::traits_type::eof())
                return false;

            features.swap(f);
            measurements.swap(m);
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "SourceCache: could not read analysis entry " << key << ": " << e.what() << std::endl;
            return false;
        }
    }

    void writeAnalysis(const std::string &key, const std::vector<WaveformFeature> &features, const std::vector<WaveformMeasurements> &measurements)
    {
        if (key.empty())
            return;

        try
        {
            writeEntry(entryPath(key, ".analysis"), [&](std::ostream &out)
                       {
                AnalysisHeader header = {};
                std::memcpy(header.magic, ANALYSIS_MAGIC, 4);
                header.version = FORMAT_VERSION;
                header.measurementSize = sizeof(WaveformMeasurements);
                header.measurements = measurements.size();
                header.features = features.size();
                out.write(reinterpret_cast<const char *>(&header), sizeof(header));
                out.write(reinterpret_cast<const char *>(measurements.data()), measurements.size() * sizeof(WaveformMeasurements));

                for (const WaveformFeature &f : features)
                {
                    int32_t type = static_cast<int32_t>(f.type);
                    int32_t index = f.index;
                    int64_t start = f.start, end = f.end;
                    uint32_t labelLength = static_cast<uint32_t>(f.label.size());
                    out.write(reinterpret_cast<const char *>(&type), sizeof(type));
                    out.write(reinterpret_cast<const char *>(&f.value), sizeof(f.value));
                    out.write(reinterpret_cast<const char *>(&start), sizeof(start));
                    out.write(reinterpret_cast<const char *>(&end), sizeof(end));
                    out.write(reinterpret_cast<const char *>(&index), sizeof(index));
                    out.write(reinterpret_cast<const char *>(&labelLength), sizeof(labelLength));
                    out.write(f.label.data(), labelLength);
                } });
        }
        catch (const std::exception &e)
        {
            std::cerr << "SourceCache: could not write analysis entry " << key << ": " << e.what() << std::endl;
        }
    }
}
