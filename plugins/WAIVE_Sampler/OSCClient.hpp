#ifndef OSCCLIENT_H
#define OSCCLIENT_H

#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <string>
#include <vector>
#include <variant>

class OSCClient
{
public:
    using OSCArgument = std::variant<std::string, int>;

    OSCClient(const std::string &host, int port);
    bool setAddress(const std::string &host, int port);
    void sendMessage(const std::string &address, const std::vector<OSCArgument> &args);

    std::string getHost() const;
    int getPort() const;

private:
    std::string formatOSCMessage(const std::string &address, const std::vector<OSCArgument> &args);
    void appendPaddedString(std::string &message, const std::string &value);
    void appendPaddedInt(std::string &message, int value);

    Poco::Net::SocketAddress _socketAddress;
    Poco::Net::DatagramSocket _socket;
};

#endif