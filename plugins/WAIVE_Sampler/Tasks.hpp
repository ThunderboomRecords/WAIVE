#ifndef TASKS_HPP_INCLUDED
#define TASKS_HPP_INCLUDED

#include <iostream>

#include <Poco/Task.h>

#include "DistrhoPlugin.hpp"
#include "ThreadsafeQueue.hpp"

START_NAMESPACE_DISTRHO

class ImporterTask : public Poco::Task
{
public:
    ImporterTask(WAIVESampler *ws, ThreadsafeQueue<std::string> *queue);
    void runTask() override;

private:
    WAIVESampler *_ws;
    ThreadsafeQueue<std::string> *_queue;

    void import(const std::string &fp);
};

class FeatureExtractorTask : public Poco::Task
{
public:
    FeatureExtractorTask(WAIVESampler *ws);
    void runTask() override;

private:
    WAIVESampler *_ws;
};

class WaveformLoaderTask : public Poco::Task
{
public:
    WaveformLoaderTask(std::shared_ptr<std::vector<float>> _buffer, std::mutex *_mutex, const std::string &_fp, int sampleRate);
    ~WaveformLoaderTask();
    void runTask() override;
    std::string fp;

private:
    std::shared_ptr<std::vector<float>> buffer;
    std::mutex *mutex;
    int sampleRate, flags;
};

class TestTask : public Poco::Task
{
public:
    TestTask(std::string name) : Poco::Task(name)
    {
        std::cout << "TestTask init" << std::endl;
    };
    void runTask() override
    {
        std::cout << "TestTask runTask" << std::endl;
        sleep(1000);
        std::cout << "TestTask done" << std::endl;
    };
};

END_NAMESPACE_DISTRHO

#endif