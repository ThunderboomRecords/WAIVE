#include "HTTPClient.hpp"

HTTPRequestTask::HTTPRequestTask(const std::string &host, const std::string &path, std::function<void(const std::string &)> callback)
    : Poco::Task("HTTPRequestTask"), _host(host), _port(-1), _path(path), _callback(callback) {}

HTTPRequestTask::HTTPRequestTask(const std::string &host, int port, const std::string &path, std::function<void(const std::string &)> callback)
    : Poco::Task("HTTPRequestTask"), _host(host), _port(port), _path(path), _callback(callback) {}

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
    }
}

HTTPClient::HTTPClient(Poco::TaskManager *tm) : _taskManager(tm)
{
    // _taskManager->addObserver(Poco::Observer<HTTPClient, Poco::TaskFinishedNotification>(*this, &HTTPClient::onTaskFinished));
}

HTTPClient::~HTTPClient() {}

void HTTPClient::sendRequest(const std::string &host, const std::string &path, std::function<void(const std::string &)> callback)
{
    HTTPRequestTask *task = new HTTPRequestTask(host, path, callback);
    _taskManager->start(task);
}

void HTTPClient::sendRequest(const std::string &host, int port, const std::string &path, std::function<void(const std::string &)> callback)
{
    HTTPRequestTask *task = new HTTPRequestTask(host, port, path, callback);
    _taskManager->start(task);
}

void HTTPClient::onTaskFinished(Poco::TaskFinishedNotification *pNf)
{
    // Poco::Task *pTask = pNf->task();
    // std::cout << "Task finished: " << pTask->name() << std::endl;
    // pTask->release();
}