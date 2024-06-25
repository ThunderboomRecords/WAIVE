#include "HTTPClient.hpp"

HTTPRequestTask::HTTPRequestTask(
    const std::string &name,
    const std::string &host,
    const std::string &path,
    std::function<void(const std::string &)> callback,
    std::function<void()> failCallback)
    : Poco::Task(name), _host(host), _port(-1), _path(path), _callback(callback), _failCallback(failCallback) {}

HTTPRequestTask::HTTPRequestTask(
    const std::string &name,
    const std::string &host,
    int port,
    const std::string &path,
    std::function<void(const std::string &)> callback,
    std::function<void()> failCallback)
    : Poco::Task(name), _host(host), _port(port), _path(path), _callback(callback), _failCallback(failCallback) {}

void HTTPRequestTask::runTask()
{
    try
    {
        Poco::Net::HTTPClientSession session(_host);
        if (_port > 0)
            session.setPort(_port);

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, _path);
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &resStream = session.receiveResponse(response);

        std::stringstream responseString;
        Poco::StreamCopier::copyStream(resStream, responseString);

        _callback(responseString.str());
    }
    catch (const Poco::Exception &ex)
    {
        std::cerr << "HTTP Request failed: " << ex.displayText() << std::endl;
        _failCallback();
    }
}

HTTPClient::HTTPClient(Poco::TaskManager *tm) : _taskManager(tm) {}
HTTPClient::~HTTPClient() {}

void HTTPClient::sendRequest(const std::string &name, const std::string &host, const std::string &path, std::function<void(const std::string &)> callback, std::function<void()> failCallback)
{
    HTTPRequestTask *task = new HTTPRequestTask(name, host, path, callback, failCallback);
    _taskManager->start(task);
}

void HTTPClient::sendRequest(const std::string &name, const std::string &host, int port, const std::string &path, std::function<void(const std::string &)> callback, std::function<void()> failCallback)
{
    HTTPRequestTask *task = new HTTPRequestTask(name, host, port, path, callback, failCallback);
    _taskManager->start(task);
}

void HTTPClient::onTaskFinished(Poco::TaskFinishedNotification *pNf) {}