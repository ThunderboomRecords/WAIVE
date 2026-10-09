#include "HTTPClient.hpp"
#include <memory>
#include <stdexcept>

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
    std::string body;
    std::string error;

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
            throw std::runtime_error(fmt::format("URI scheme neither 'http' nor 'https', is {:s}", uri.getScheme()));

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
            throw std::runtime_error(fmt::format("Response status not 200, received {:s}", HTTPResponse::getReasonForStatus(response.getStatus())));
        }

        std::stringstream responseString;
        Poco::StreamCopier::copyStream(resStream, responseString);

        body = responseString.str();
        std::cout << "HTTPRequestTask::runTask() Response length: " << body.length() << std::endl;

        // A dropped connection can end the stream early without throwing,
        // so check the body against the advertised length
        if (response.getContentLength64() != HTTPMessage::UNKNOWN_CONTENT_LENGTH &&
            static_cast<Poco::Int64>(body.length()) != response.getContentLength64())
            throw std::runtime_error(fmt::format("Incomplete response, received {:d} of {:d} bytes", body.length(), response.getContentLength64()));
    }
    catch (const Poco::Exception &ex)
    {
        error = fmt::format("HTTP Request failed: {:s}", ex.displayText());
    }
    catch (const std::exception &ex)
    {
        error = fmt::format("HTTP Request failed: {:s}", ex.what());
    }

    // Exactly one of the callbacks is called, outside the request's try
    // block, so an exception thrown by a callback is not reported as a
    // failed request (and the fail callback is never called twice)
    try
    {
        if (error.empty())
            callback(body);
        else
        {
            std::cerr << error << std::endl;
            failCallback(error);
        }
    }
    catch (const Poco::Exception &ex)
    {
        std::cerr << "HTTP Request callback failed: " << ex.displayText() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "HTTP Request callback failed: " << ex.what() << std::endl;
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