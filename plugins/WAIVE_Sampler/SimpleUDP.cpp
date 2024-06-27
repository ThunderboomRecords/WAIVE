#include "SimpleUDP.hpp"

SimpleUDPServer::SimpleUDPServer(char *addr, int port)
{

// Initialize Winsock on Windows
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        open = false;
        return;
    }
#endif

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        std::cerr << "Socket creation failed\n";
        open = false;
        return;
    }

    // Server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(addr);

    open = true;
}

SimpleUDPServer::~SimpleUDPServer()
{
// Close the socket
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
}

bool SimpleUDPServer::sendMessage(char *msg, int len)
{
    if (!open)
        return false;

    if (sendto(sockfd, msg, len, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Failed to send message\n";
        return false;
    }

    return true;
}
