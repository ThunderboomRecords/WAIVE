#ifndef HTTP_CLIENT_HPP_INCLUDED
#define HTTP_CLIENT_HPP_INCLUDED

#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>
#include <Poco/Timespan.h>
#include <Poco/Task.h>
#include <Poco/URI.h>
#include <Poco/TaskManager.h>
#include <Poco/TaskNotification.h>
#include <Poco/Observer.h>
#include <iostream>
#include <sstream>
#include <functional>

class HTTPRequestTask : public Poco::Task
{
public:
    HTTPRequestTask(const std::string &name, const std::string &host, const std::string &path, std::function<void(const std::string &)> callback, std::function<void()> failCallback);
    HTTPRequestTask(const std::string &name, const std::string &host, int port, const std::string &path, std::function<void(const std::string &)> callback, std::function<void()> failCallback);
    void runTask() override;

private:
    std::string _host;
    std::string _path;
    int _port;
    std::function<void(const std::string &)> _callback;
    std::function<void()> _failCallback;
};

class HTTPClient
{
public:
    HTTPClient(Poco::TaskManager *tm);
    ~HTTPClient();

    void sendRequest(const std::string &name, const std::string &host, const std::string &path, std::function<void(const std::string &)> callback, std::function<void()> failCallback);
    void sendRequest(const std::string &name, const std::string &host, int port, const std::string &path, std::function<void(const std::string &)> callback, std::function<void()> failCallback);
    void onTaskFinished(Poco::TaskFinishedNotification *pNf);

private:
    Poco::TaskManager *_taskManager;
};

#endif