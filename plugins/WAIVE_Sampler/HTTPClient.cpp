#include "HTTPClient.hpp"
#include <memory>

using namespace Poco::Net;

HTTPRequestTask::HTTPRequestTask(
    const std::string &name,
    const std::string &host,
    const std::string &path,
    std::function<void(const std::string &)> callback,
    std::function<void(const std::string &)> failCallback)
    : Poco::Task(name), host(host), port(0), path(path), callback(callback), failCallback(failCallback) {}

HTTPRequestTask::HTTPRequestTask(
    const std::string &name,
    const std::string &host,
    int port,
    const std::string &path,
    std::function<void(const std::string &)> callback,
    std::function<void(const std::string &)> failCallback)
    : Poco::Task(name), host(host), port(port), path(path), callback(callback), failCallback(failCallback) {}

void HTTPRequestTask::runTask()
{
    try
    {
        if (path.front() != '/')
            path = '/' + path;

        Poco::URI uri(host + path);

        std::string fullpath(uri.getPathAndQuery());
        if (fullpath.empty())
            fullpath = "/";

        std::unique_ptr<HTTPClientSession> session;

        if (uri.getScheme() == "https")
            session = std::make_unique<HTTPSClientSession>(uri.getHost(), uri.getPort());
        else if (uri.getScheme() == "http")
            session = std::make_unique<HTTPClientSession>(uri.getHost(), uri.getPort());
        else
        {
            failCallback(fmt::format("URI scheme neither 'http' nor 'https', is {:s}", uri.getScheme()));
            return;
        }

        session->setTimeout(Poco::Timespan(10, 0));

        HTTPRequest request(HTTPRequest::HTTP_GET, fullpath);

        std::cout << "HTTPRequestTask::runTask() Request for: " << uri.getScheme() << "://" << uri.getHost() << ":" << uri.getPort() << request.getURI() << std::endl;
        session->sendRequest(request);

        HTTPResponse response;

        std::istream &resStream = session->receiveResponse(response);
        std::cout << "HTTPRequestTask::runTask() Response: " << HTTPResponse::getReasonForStatus(response.getStatus()) << std::endl;

        if (response.getStatus() != 200)
        {
            // TODO: check for other status codes?
            failCallback(fmt::format("Response status not 200, received {:s}", HTTPResponse::getReasonForStatus(response.getStatus())));
            return;
        }

        std::stringstream responseString;
        Poco::StreamCopier::copyStream(resStream, responseString);

        std::cout << "HTTPRequestTask::runTask() Response length: " << responseString.str().length() << std::endl;

        callback(responseString.str());
    }
    catch (const Poco::Exception &ex)
    {
        std::cerr << "HTTP Request failed: " << ex.displayText() << std::endl;
        failCallback(fmt::format("HTTP Request failed: {:s}", ex.displayText()));
    }
}

HTTPClient::HTTPClient(Poco::TaskManager *tm) : taskManager(tm) {}
HTTPClient::~HTTPClient() {}

void HTTPClient::sendRequest(const std::string &name, const std::string &host, const std::string &path, std::function<void(const std::string &)> callback, std::function<void(const std::string &)> failCallback)
{
    HTTPRequestTask *task = new HTTPRequestTask(name, host, path, callback, failCallback);
    taskManager->start(task);
}

void HTTPClient::sendRequest(const std::string &name, const std::string &host, int port, const std::string &path, std::function<void(const std::string &)> callback, std::function<void(const std::string &)> failCallback)
{
    HTTPRequestTask *task = new HTTPRequestTask(name, host, port, path, callback, failCallback);
    taskManager->start(task);
}

void HTTPClient::onTaskFinished(Poco::TaskFinishedNotification *pNf) {}