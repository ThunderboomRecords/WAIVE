#include "WAIVESampler.hpp"
#include "Tasks.hpp"
#include "SourceCache.hpp"

START_NAMESPACE_DISTRHO

// libsamplerate converter used when a file's sample rate differs from the
// plugin's. SINC_MEDIUM_QUALITY (121 dB SNR, 90% bandwidth) is about 4x
// faster than SINC_BEST_QUALITY (145 dB SNR, 96% bandwidth).
static constexpr int RESAMPLER_TYPE = SRC_SINC_MEDIUM_QUALITY;

size_t loadWaveform(const char *fp, std::vector<float> &buffer, int sampleRate, int flags)
{
    // Open the file
    SndfileHandle fileHandle(fp, 16, flags);
    int sampleLength = fileHandle.frames();
    if (sampleLength == 0)
    {
        std::cerr << "Error: Unable to open input file " << fp << std::endl;
        return 0;
    }

    int sampleChannels = fileHandle.channels();
    int fileSampleRate = fileHandle.samplerate();

    // Read the file into a temporary sample buffer
    std::vector<float> sample(sampleLength * sampleChannels);
    if (fileHandle.read(sample.data(), sampleLength * sampleChannels) != sampleLength * sampleChannels)
    {
        std::cerr << "Error: Failed to read audio data from file." << std::endl;
        return 0;
    }

    std::vector<float> sample_tmp;

    // Resample if necessary
    if (fileSampleRate != sampleRate)
    {
        size_t new_size = static_cast<size_t>(std::ceil(static_cast<double>(sampleLength) * sampleRate / fileSampleRate));
        sample_tmp.resize(new_size * sampleChannels);

        SRC_DATA src_data = {};
        src_data.data_in = sample.data();
        src_data.input_frames = sampleLength;
        src_data.data_out = sample_tmp.data();
        src_data.output_frames = new_size;
        src_data.src_ratio = static_cast<double>(sampleRate) / fileSampleRate;

        int result = src_simple(&src_data, RESAMPLER_TYPE, sampleChannels);
        if (result != 0)
        {
            std::cerr << "Failed to convert sample rate: " << src_strerror(result) << std::endl;
            return 0;
        }

        // Adjust the actual output length after resampling
        sample_tmp.resize(static_cast<size_t>(src_data.output_frames_gen * sampleChannels));
    }
    else
    {
        sample_tmp = std::move(sample);
    }

    // Resize the output buffer
    size_t outputLength = sample_tmp.size() / sampleChannels;
    buffer.resize(outputLength);

    // Mix to mono if necessary
    if (sampleChannels > 1)
    {
        for (size_t i = 0; i < outputLength; ++i)
        {
            buffer[i] = 0.0f;
            for (int c = 0; c < sampleChannels; ++c)
                buffer[i] += sample_tmp[i * sampleChannels + c];
            buffer[i] /= static_cast<float>(sampleChannels);
        }
    }
    else
        std::copy(sample_tmp.begin(), sample_tmp.end(), buffer.begin());

    return outputLength;
}

ImporterTask::ImporterTask(WAIVESampler *ws, ThreadsafeQueue<std::string> *queue) : Poco::Task("ImporterTask"), _ws(ws), _queue(queue) {};

void ImporterTask::runTask()
{
    while (true)
    {
        std::optional<std::string> fp;
        {
            // Stop while holding the lock, so setState() starts a new task
            // for anything queued after this point
            std::lock_guard<std::mutex> lock(_ws->importerMtx);
            if (!isCancelled())
                fp = _queue->pop();
            if (!fp.has_value())
            {
                _ws->importerRunning = false;
                return;
            }
        }

        // One bad file shouldn't stop the rest of the queue from importing
        try
        {
            ImporterTask::import(fp.value());
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to import " << fp.value() << ": " << e.what() << std::endl;
            continue;
        }

        _ws->pluginUpdate.notify(&_ws, WAIVESampler::PluginUpdate::kSampleAdded);
    }
}

void ImporterTask::import(const std::string &fp)
{
    std::string folder = Poco::Path(fp).makeParent().toString();

    // Always saved as WAV, whatever the format of the imported file
    Poco::Path wavName(Poco::Path(fp).getFileName());
    wavName.setExtension("wav");
    std::string name = _ws->sd.getNewSampleName(wavName.getFileName());

    std::cout << "     folder: " << folder << std::endl;
    std::cout << "       name: " << name << std::endl;

    std::vector<float> sampleCopy;
    int sampleLength = loadWaveform(fp.c_str(), sampleCopy, _ws->getSampleRate());

    if (sampleLength == 0)
    {
        std::cout << "sampleLength is 0\n";
        return;
    }

    // id is assigned when added to the database
    std::shared_ptr<SampleInfo> info(new SampleInfo(-1, name, "", false));
    info->adsr.attack = 0.0f;
    info->adsr.decay = 0.0f;
    info->adsr.sustain = 1.0f;
    info->adsr.release = 0.0f;
    // Sustain for the whole sample, so editing it doesn't shorten it
    info->sustainLength = std::min(static_cast<float>(1000.0 * sampleLength / _ws->getSampleRate()), MAX_SUSTAIN_LENGTH);
    info->sampleLength = sampleLength;
    info->percussiveBoost = 0.0f;
    info->filterCutoff = 0.999f;
    info->filterType = Filter::FILTER_LOWPASS;
    info->filterResonance = 0;
    info->pitch = 1.0f;
    info->volume = 1.0f;
    info->sourceStart = 0;
    info->saved = true;

    // TODO: render loaded sample...
    auto embedding = _ws->getEmbedding(&sampleCopy);
    info->embedX = embedding.first;
    info->embedY = embedding.second;

    // Save the file first, so the library never has a sample without one
    std::string samplePath = _ws->sd.getFullSamplePath(info);
    if (!saveWaveform(samplePath.c_str(), sampleCopy.data(), sampleLength, _ws->getSampleRate()))
        throw Poco::Exception("Failed to save waveform");

    // The library copy is the source, so the sample doesn't depend on the
    // imported file still being there
    info->sourceInfo->fp = samplePath;

    if (!_ws->sd.addToLibrary(info))
    {
        Poco::File(samplePath).remove();
        throw Poco::Exception("Failed to add sample to library");
    }

    std::cout << " - import done\n";
};

FeatureExtractorTask::FeatureExtractorTask(std::shared_ptr<Source> s, int sr)
    : Poco::Task("FeatureExtractorTask"),
      source(std::move(s)),
      sampleRate(sr) {}

void FeatureExtractorTask::runTask()
{
    std::cout << "FeatureExtractorTask::runTask()" << std::endl;
    if (source->length == 0)
        return;

    // Analyse a copy of the buffer, so the source isn't locked for the whole
    // analysis (renderSample() needs the lock, and can be called from the
    // host's audio thread via setParameterValue)
    std::vector<float> buffer;
    std::string fp;
    {
        std::lock_guard<std::mutex> lock(source->mtx);
        source->sourceFeatures.clear();
        source->sourceMeasurements.clear();
        buffer.assign(source->buffer.begin(), source->buffer.begin() + std::min(source->length, source->buffer.size()));
        fp = source->fp;
    }

    std::vector<WaveformFeature> features;
    std::vector<WaveformMeasurements> measurements;

    // The analysis depends on the buffer, so the key includes how it was
    // made (sample rate and resampler) and its length
    std::string cacheKey = SourceCache::makeKey(fp, "analysis:sr=" + std::to_string(sampleRate) +
                                                        ":resampler=" + std::to_string(RESAMPLER_TYPE) +
                                                        ":frames=" + std::to_string(buffer.size()));

    if (SourceCache::readAnalysis(cacheKey, features, measurements))
    {
        std::cout << "FeatureExtractorTask: loaded analysis from cache" << std::endl;
    }
    else
    {
        int frameIndex = 0;
        long frame = 0;
        long length = static_cast<long>(buffer.size());
        Gist<float> gist(128, sampleRate);
        const int frameSize = gist.getAudioFrameSize();

        measurements.reserve(length / frameSize + 1);

        float pStep = static_cast<float>(frameSize) / length;
        float p = 0.0f;

        long lastOnset = -5000;

        while (frame < length - frameSize && !isCancelled())
        {
            const float *frameData = &buffer[frame];
            gist.processAudioFrame(frameData, frameSize);

            // Only the measurements used for choosing sample start points are
            // calculated. RMS is computed here, as Gist's version uses pow(),
            // which is several times slower.
            float sumSquares = 0.0f;
            for (int i = 0; i < frameSize; i++)
                sumSquares += frameData[i] * frameData[i];

            WaveformMeasurements m{};
            m.frame = frame;
            m.start = frame * frameSize;
            m.rms = std::sqrt(sumSquares / frameSize);
            m.specFlat = gist.spectralFlatness();

            float onset = gist.complexSpectralDifference();

            if (onset > 100.f && frame > lastOnset + 4800)
            {
                features.push_back({FeatureType::Onset,
                                    "onset",
                                    onset,
                                    frame,
                                    frame,
                                    frameIndex});
                lastOnset = frame;
            }
            measurements.push_back(m);
            p += pStep;
            setProgress(p);

            frame += frameSize;
            frameIndex++;
        }

        if (!isCancelled())
            SourceCache::writeAnalysis(cacheKey, features, measurements);
    }

    if (isCancelled())
        return;

    {
        std::lock_guard<std::mutex> lock(source->mtx);

        // A different source was loaded during the analysis
        if (source->fp != fp || source->length != buffer.size())
            return;

        source->sourceFeatures = std::move(features);
        source->sourceMeasurements = std::move(measurements);
    }

    setProgress(1.0f);
}

WaveformLoaderTask::WaveformLoaderTask(
    const std::string &name,
    std::shared_ptr<std::vector<float>> buffer,
    std::shared_ptr<std::mutex> mutex,
    const std::string &fp,
    int sampleRate,
    bool useCache)
    : Poco::Task(name),
      buffer(buffer),
      mutex(mutex),
      fp(fp),
      sampleRate(sampleRate),
      useCache(useCache) {};

WaveformLoaderTask::~WaveformLoaderTask() {}

void WaveformLoaderTask::runTask()
{
    std::cout << "WaveformLoaderTask::runTask()" << std::endl;
    std::cout << " fp: " << fp << std::endl;

    // The file is read and resampled without holding the mutex (which, for
    // sample players, is also taken by the audio thread); it is only locked
    // to swap in the result.
    SndfileHandle fileHandle(fp, SFM_READ);
    if (fileHandle.error())
    {
        std::cerr << "Error: Unable to open input file " << fp << "\n  error: " << sf_error_number(fileHandle.error()) << std::endl;
        std::lock_guard<std::mutex> lock(*mutex);
        buffer->resize(0);

        throw std::runtime_error("Unable to open file " + fp);
    }

    size_t sampleLength = fileHandle.frames();

    if (sampleLength == 0)
    {
        std::cerr << "Error: Unable to open input file " << fp << "\n  error: " << sf_error_number(fileHandle.error()) << std::endl;
        std::lock_guard<std::mutex> lock(*mutex);
        buffer->resize(0);

        throw std::runtime_error("Unable to open file " + fp);
    }

    int sampleChannels = fileHandle.channels();
    int fileSampleRate = fileHandle.samplerate();

    // Only resampled audio is cached: a file already at the right sample
    // rate is as quick to read directly
    std::string cacheKey;
    if (useCache && fileSampleRate != sampleRate)
    {
        cacheKey = SourceCache::makeKey(fp, "audio:sr=" + std::to_string(sampleRate) +
                                                ":resampler=" + std::to_string(RESAMPLER_TYPE));

        std::vector<float> cached;
        if (SourceCache::readAudio(cacheKey, cached))
        {
            std::cout << "WaveformLoaderTask: loaded resampled audio from cache" << std::endl;
            std::lock_guard<std::mutex> lock(*mutex);
            buffer->swap(cached);
            return;
        }
    }

    std::vector<float> sample;
    sample.resize(sampleLength * sampleChannels);

    fileHandle.read(sample.data(), sampleLength * sampleChannels);

    std::vector<float> sample_tmp;
    size_t new_size = sampleLength;

    if (isCancelled())
        return;

    // resample data
    if (fileSampleRate == sampleRate)
    {
        sample_tmp.swap(sample);
    }
    else
    {
        double src_ratio = (double)sampleRate / fileSampleRate;
        sample_tmp.reserve(((size_t)(sampleLength * src_ratio) + 1) * sampleChannels);

        SRC_DATA src_data = {};
        src_data.src_ratio = src_ratio;

        std::cout << "RESAMPLING WAVEFORM:\n";
        std::cout << "  fileSampleRate: " << fileSampleRate << std::endl;
        std::cout << "      sampleRate: " << sampleRate << std::endl;
        std::cout << " sample_channels: " << sampleChannels << std::endl;
        std::cout << "       src_ratio: " << src_data.src_ratio << std::endl;

        const size_t chunk = 4096;
        std::vector<float> outputChunk(((size_t)(chunk * src_ratio) + 2) * sampleChannels);

        int error;

        // Use a smart pointer to ensure src_delete is called even if an exception is thrown.
        std::unique_ptr<SRC_STATE, decltype(&src_delete)> converter(src_new(RESAMPLER_TYPE, sampleChannels, &error), &src_delete);
        if (!converter)
        {
            std::cerr << "Could not init samplerate converter, reason: " << src_strerror(error) << std::endl;
            throw std::runtime_error("Could not init samplerate converter, reason: " + std::string(src_strerror(error)));
        }

        setProgress(0.0f);

        size_t inputFrame = 0;
        while (!isCancelled())
        {
            size_t remaining = sampleLength - inputFrame;
            size_t currentChunkSize = std::min(chunk, remaining);

            src_data.data_in = sample.data() + inputFrame * sampleChannels;
            src_data.input_frames = currentChunkSize;
            src_data.data_out = outputChunk.data();
            src_data.output_frames = outputChunk.size() / sampleChannels;
            // Tell the converter when the input ends, so it outputs its last samples
            src_data.end_of_input = currentChunkSize == remaining ? 1 : 0;

            error = src_process(converter.get(), &src_data);
            if (error)
            {
                std::cout << "Error during sample rate conversion: " << src_strerror(error) << std::endl;
                throw std::runtime_error("Error during sample rate conversion: " + std::string(src_strerror(error)));
            }

            sample_tmp.insert(sample_tmp.end(), src_data.data_out, src_data.data_out + src_data.output_frames_gen * sampleChannels);

            // The converter may not use all of the input if the output buffer fills
            inputFrame += src_data.input_frames_used;
            setProgress(static_cast<float>(inputFrame) / sampleLength);

            if (src_data.end_of_input && src_data.output_frames_gen == 0)
                break;
        }

        new_size = sample_tmp.size() / sampleChannels;
    }

    if (isCancelled())
        return;

    if (buffer == nullptr)
    {
        std::cerr << "WaveformLoaderTask::runTask() buffer is nullptr!" << std::endl;
        return;
    }

    std::vector<float> result;

    if (sampleChannels > 1)
    {
        result.resize(new_size);
        for (size_t i = 0; i < new_size; ++i)
        {
            float sum = 0.0f;
            for (int ch = 0; ch < sampleChannels; ++ch)
                sum += sample_tmp[i * sampleChannels + ch];
            result[i] = sum / sampleChannels;
        }
    }
    else
    {
        sample_tmp.resize(new_size);
        result.swap(sample_tmp);
    }

    SourceCache::writeAudio(cacheKey, result);

    {
        std::lock_guard<std::mutex> lock(*mutex);
        buffer->swap(result);
    }

    std::cout << "WaveformLoaderTask::runTask() finished" << std::endl;
}

END_NAMESPACE_DISTRHO