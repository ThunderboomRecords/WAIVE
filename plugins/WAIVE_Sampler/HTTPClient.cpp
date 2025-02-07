#include "HTTPClient.hpp"
#include <memory>

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

        std::unique_ptr<Poco::Net::HTTPClientSession> session;

        if (uri.getScheme() == "https")
            session = std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort());
        else if (uri.getScheme() == "http")
            session = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
        else
        {
            _failCallback();
            return;
        }

        session->setTimeout(Poco::Timespan(10, 0));

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path);

        std::cout << "HTTPRequestTask::runTask() Request for: " << uri.getScheme() << "://" << uri.getHost() << ":" << uri.getPort() << request.getURI() << std::endl;
        session->sendRequest(request);

        Poco::Net::HTTPResponse response;

        std::istream &resStream = session->receiveResponse(response);
        std::cout << "HTTPRequestTask::runTask() Response: " << response.getStatus() << std::endl;

        if (response.getStatus() != 200)
        {
            // TODO: check for other status codes?
            _failCallback();
            return;
        }

        std::stringstream responseString;
        Poco::StreamCopier::copyStream(resStream, responseString);

        std::cout << "HTTPRequestTask::runTask() Response length: " << responseString.str().length() << std::endl;

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