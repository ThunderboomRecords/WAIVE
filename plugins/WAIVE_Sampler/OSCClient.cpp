#include "OSCClient.hpp"
#include <Poco/Exception.h>
#include <iostream>
#include <cstring>

OSCClient::OSCClient(const std::string &host, int port)
    : _socketAddress(host, port), _socket(Poco::Net::DatagramSocket())
{
    _socket.connect(_socketAddress);
}

bool OSCClient::setAddress(const std::string &host, int port)
{
    try
    {
        _socket.close();
        _socketAddress = Poco::Net::SocketAddress(host, port);
        _socket.connect(_socketAddress);
    }
    catch (Poco::Exception &e)
    {
        std::cerr << "Error initialising socket " << e.message() << std::endl;
        return false;
    }

    return true;
}

std::string OSCClient::getHost() const
{
    return _socketAddress.host().toString();
}

int OSCClient::getPort() const
{
    return _socketAddress.port();
}

void OSCClient::sendMessage(const std::string &address, const std::vector<OSCArgument> &args)
{
    try
    {
        std::string message = formatOSCMessage(address, args);
        _socket.sendBytes(message.data(), static_cast<int>(message.size()));
    }
    catch (const Poco::Exception &ex)
    {
        std::cerr << "Error sending OSC message: " << ex.displayText() << std::endl;
    }
}

std::string OSCClient::formatOSCMessage(const std::string &address, const std::vector<OSCArgument> &args)
{
    std::string message;

    appendPaddedString(message, address);

    std::string typeTags = ",";
    for (const auto &arg : args)
    {
        if (std::holds_alternative<std::string>(arg))
            typeTags += "s";
        else if (std::holds_alternative<int>(arg))
            typeTags += "i";
    }
    appendPaddedString(message, typeTags);

    // Add arguments
    for (const auto &arg : args)
    {
        if (std::holds_alternative<std::string>(arg))
            appendPaddedString(message, std::get<std::string>(arg));
        else if (std::holds_alternative<int>(arg))
            appendPaddedInt(message, std::get<int>(arg));
    }

    return message;
}

void OSCClient::appendPaddedString(std::string &message, const std::string &value)
{
    message.append(value);
    message.append(4 - (value.size() % 4), '\0'); // Pad with null characters to 4-byte boundary
}

void OSCClient::appendPaddedInt(std::string &message, int value)
{
    int networkValue = htonl(value); // Convert to network byte order TODO: check on windows!
    message.append(reinterpret_cast<const char *>(&networkValue), sizeof(networkValue));
    message.append(4 - (sizeof(networkValue) % 4), '\0'); // Pad with null characters to 4-byte boundary
}