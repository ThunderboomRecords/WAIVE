#include "HTTPClient.hpp"

HTTPRequestTask::HTTPRequestTask(
    const std::string &name,
    const std::string &host,
    const std::string &path,
    std::function<void(const std::string &)> callback,
    std::function<void()> failCallback)
    : Poco::Task(name), _host(host), _port(0), _path(path), _callback(callback), _failCallback(failCallback) {}

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
        if (_path.front() != '/')
            _path = '/' + _path;

        Poco::URI uri(_host + _path);

        std::string path(uri.getPathAndQuery());
        if (path.empty())
            path = "/";

        Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());
        session.setTimeout(Poco::Timespan(10, 0));

        std::cout << "HTTPRequestTask::runTask() Request for: " << session.getHost() << path << std::endl;

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path);
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;

        std::istream &resStream = session.receiveResponse(response);
        std::cout << "HTTPRequestTask::runTask() Response: " << response.getStatus() << std::endl;

        if (response.getStatus() != 200)
        {
            // TODO: check for other status codes?
            _failCallback();
            return;
        }

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