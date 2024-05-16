#ifndef SIMPLE_UDP_SERVER_INCLUDED
#define SIMPLE_UDP_SERVER_INCLUDED

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

// Include platform-specific headers for sockets
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#define PORT 8080             // Port number to send messages
#define SERVER_IP "127.0.0.1" // Server IP address

class SimpleUDPServer
{
public:
    SimpleUDPServer(char *addr, int port);
    ~SimpleUDPServer();

    bool sendMessage(char *msg, int len);

    bool open;

private:
    struct sockaddr_in serverAddr;

    int sockfd;
};

#endif