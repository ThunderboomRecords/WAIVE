#include "WAIVESampler.hpp"
#include "Tasks.hpp"

START_NAMESPACE_DISTRHO

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
        src_data.src_ratio = static_cast<float>(sampleRate) / fileSampleRate;

        int result = src_simple(&src_data, SRC_SINC_BEST_QUALITY, sampleChannels);
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
        std::optional<std::string> fp = _queue->pop();
        if (fp.has_value())
        {
            ImporterTask::import(fp.value());
            _ws->pluginUpdate.notify(&_ws, WAIVESampler::PluginUpdate::kSampleAdded);
        }
        else
            break;
    }
}

void ImporterTask::import(const std::string &fp)
{
    std::string folder = Poco::Path(fp).makeParent().toString();
    std::string name = Poco::Path(fp).getFileName();

    name.assign(_ws->sd.getNewSampleName(name));

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
    info->sustainLength = 1000.0f;
    info->percussiveBoost = 0.0f;
    info->filterCutoff = 0.999f;
    info->filterType = Filter::FILTER_LOWPASS;
    info->filterResonance = 0;
    info->pitch = 1.0f;
    info->volume = 1.0f;
    info->sourceInfo.fp = fp;
    info->sourceStart = 0;
    info->saved = true;

    // TODO: render loaded sample...
    auto embedding = _ws->getEmbedding(&sampleCopy);
    info->embedX = embedding.first;
    info->embedY = embedding.second;

    _ws->sd.addToLibrary(info);

    bool result = saveWaveform(_ws->sd.getFullSamplePath(info).c_str(), &(sampleCopy.at(0)), sampleLength, _ws->getSampleRate());
    if (!result)
    {
        throw Poco::Exception("Failed to save waveform");
    }

    std::cout << " - import done\n";
};

FeatureExtractorTask::FeatureExtractorTask(Source *s, int sr)
    : Poco::Task("FeatureExtractorTask"),
      source(s),
      sampleRate(sr) {}

void FeatureExtractorTask::runTask()
{
    std::cout << "FeatureExtractorTask::runTask()" << std::endl;
    if (source->length == 0)
        return;

    // TODO: check and load cached features
    // try
    // {
    //     Poco::File cachedir(Poco::Path(Poco::Path::cacheHome()).append("WAIVE").append("SourceAnalysis"));
    //     if (!cachedir.exists())
    //     {
    //         cachedir.createDirectories();
    //     }
    // }
    // catch (const Poco::Exception &e)
    // {
    //     std::cerr << "Error creating cache directories: " << e.displayText() << std::endl;
    //     return;
    // }

    int frameIndex = 0;
    long frame = 0;
    long length = source->length;
    Gist<float> gist(128, sampleRate);
    const int frameSize = gist.getAudioFrameSize();

    std::lock_guard<std::mutex> lock(source->mtx);
    source->sourceFeatures.clear();
    source->sourceMeasurements.clear();

    float pStep = static_cast<float>(gist.getAudioFrameSize()) / length;
    float p = 0.0f;

    long lastOnset = -5000;

    while (frame < length - frameSize && !isCancelled())
    {
        WaveformMeasurements m;
        gist.processAudioFrame(&source->buffer.at(frame), frameSize);

        m.frame = frame;
        m.start = frame * frameSize;
        m.rms = gist.rootMeanSquare();
        m.peakEnergy = gist.peakEnergy();
        m.specCentroid = gist.spectralCentroid();
        m.specCrest = gist.spectralCrest();
        m.specFlat = gist.spectralFlatness();
        m.specKurtosis = gist.spectralKurtosis();
        m.specRolloff = gist.spectralRolloff();
        m.zcr = gist.zeroCrossingRate();
        m.highfrequencyContent = gist.highFrequencyContent();

        // printf("  RMS: %6.2f PE: %6.2f SCent: %6.2f SCre: %6.2f SF: %6.2f SK: %6.2f SR: %6.2f ZCR: %6.2f\n",
        //        m.rms, m.peakEnergy, m.specCentroid, m.specCrest, m.specFlat, m.specKurtosis, m.specRolloff, m.zcr);

        float onset = gist.complexSpectralDifference();

        if (onset > 100.f && frame > lastOnset + 4800)
        {
            source->sourceFeatures.push_back({FeatureType::Onset,
                                              "onset",
                                              onset,
                                              frame,
                                              frame,
                                              frameIndex});
            lastOnset = frame;
        }
        source->sourceMeasurements.push_back(m);
        p += pStep;
        setProgress(p);

        frame += gist.getAudioFrameSize();
        frameIndex++;
    }

    // TODO: cache features here (if not cancelled)

    setProgress(1.0f);
}

WaveformLoaderTask::WaveformLoaderTask(
    const std::string &name,
    std::shared_ptr<std::vector<float>> buffer,
    std::shared_ptr<std::mutex> mutex,
    const std::string &fp,
    int sampleRate)
    : Poco::Task(name),
      buffer(buffer),
      mutex(mutex),
      fp(fp),
      sampleRate(sampleRate) {};

WaveformLoaderTask::~WaveformLoaderTask() {}

void WaveformLoaderTask::runTask()
{
    std::cout << "WaveformLoaderTask::runTask()" << std::endl;
    std::cout << " fp: " << fp << std::endl;

    std::lock_guard<std::mutex> lock(*mutex);

    SndfileHandle fileHandle(fp, SFM_READ);
    if (fileHandle.error())
    {
        std::cerr << "Error: Unable to open input file " << fp << "\n  error: " << sf_error_number(fileHandle.error()) << std::endl;
        buffer->resize(0);

        throw std::runtime_error("Unable to open file " + fp);
    }

    size_t sampleLength = fileHandle.frames();

    if (sampleLength == 0)
    {
        std::cerr << "Error: Unable to open input file " << fp << "\n  error: " << sf_error_number(fileHandle.error()) << std::endl;
        buffer->resize(0);

        throw std::runtime_error("Unable to open file " + fp);
    }

    int sampleChannels = fileHandle.channels();
    int fileSampleRate = fileHandle.samplerate();

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
        new_size = (size_t)(sampleLength * src_ratio + 1);
        sample_tmp.reserve(new_size * sampleChannels + sampleChannels);

        SRC_DATA src_data = {};
        src_data.src_ratio = src_ratio;

        std::cout << "RESAMPLING WAVEFORM:\n";
        std::cout << "  fileSampleRate: " << fileSampleRate << std::endl;
        std::cout << "      sampleRate: " << sampleRate << std::endl;
        std::cout << " sample_channels: " << sampleChannels << std::endl;
        std::cout << "       src_ratio: " << src_data.src_ratio << std::endl;
        std::cout << "        new_size: " << new_size << std::endl;

        size_t chunk = 512;
        std::vector<float> outputChunk((size_t)(sampleChannels * chunk * src_data.src_ratio) + sampleChannels);

        double progress = 0.0f;
        int error;

        // Use a smart pointer to ensure src_delete is called even if an exception is thrown.
        std::unique_ptr<SRC_STATE, decltype(&src_delete)> converter(src_new(SRC_SINC_BEST_QUALITY, sampleChannels, &error), &src_delete);
        if (!converter)
        {
            std::cerr << "Could not init samplerate converter, reason: " << sf_error_number(error) << std::endl;
            throw std::runtime_error("Could not init samplerate converter, reason: " + std::string(sf_error_number(error)));
        }

        setProgress(progress);

        src_data.end_of_input = 0;
        src_data.input_frames = chunk;

        size_t inputPos = 0;
        double pStep = (double)(chunk * sampleChannels) / sample.size();
        while (inputPos < sample.size() && !isCancelled())
        {
            size_t currentChunkSize = std::min(chunk, (sample.size() - inputPos) / sampleChannels);

            src_data.data_in = sample.data() + inputPos;
            src_data.input_frames = currentChunkSize;
            src_data.data_out = outputChunk.data();
            src_data.output_frames = outputChunk.size() / sampleChannels;

            error = src_process(converter.get(), &src_data);
            if (error)
            {
                std::cout << "Error during sample rate conversion: " << src_strerror(error) << std::endl;
                throw std::runtime_error("Error during sample rate conversion: " + std::string(src_strerror(error)));
            }

            sample_tmp.insert(sample_tmp.end(), src_data.data_out, src_data.data_out + src_data.output_frames_gen * sampleChannels);

            progress += pStep;
            setProgress(progress);

            inputPos += currentChunkSize * sampleChannels;
        }
    }

    if (isCancelled())
        return;

    if (buffer == nullptr)
    {
        std::cerr << "WaveformLoaderTask::runTask() buffer is nullptr!" << std::endl;
        return;
    }

    buffer->resize(new_size);

    if (sampleChannels > 1)
    {
        for (size_t i = 0; i < new_size; ++i)
        {
            float sum = 0.0f;
            for (int ch = 0; ch < sampleChannels; ++ch)
                sum += sample_tmp[i * sampleChannels + ch];
            buffer->at(i) = sum / sampleChannels;
        }
    }
    else
    {
        std::copy(sample_tmp.begin(), sample_tmp.begin() + new_size, buffer->begin());
    }

    std::cout << "WaveformLoaderTask::runTask() finished" << std::endl;
}

END_NAMESPACE_DISTRHO