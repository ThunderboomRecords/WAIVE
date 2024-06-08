#ifndef HTTP_CLIENT_HPP_INCLUDED
#define HTTP_CLIENT_HPP_INCLUDED

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>
#include <Poco/Task.h>
#include <Poco/TaskManager.h>
#include <Poco/TaskNotification.h>
#include <Poco/Observer.h>
#include <iostream>
#include <sstream>
#include <functional>

class HTTPRequestTask : public Poco::Task
{
public:
    HTTPRequestTask(const std::string &host, const std::string &path, std::function<void(const std::string &)> callback);
    HTTPRequestTask(const std::string &host, int port, const std::string &path, std::function<void(const std::string &)> callback);
    void runTask() override;

private:
    std::string _host;
    std::string _path;
    int _port;
    std::function<void(const std::string &)> _callback;
};

class HTTPClient
{
public:
    HTTPClient();
    ~HTTPClient();

    void sendRequest(const std::string &host, const std::string &path, std::function<void(const std::string &)> callback);
    void sendRequest(const std::string &host, int port, const std::string &path, std::function<void(const std::string &)> callback);
    void onTaskFinished(Poco::TaskFinishedNotification *pNf);

private:
    Poco::TaskManager _taskManager;
};

#endif