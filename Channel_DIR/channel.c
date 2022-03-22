//
// Created by awsle on 22/03/2022.
//

#include "channel.h"

void InitSock(WSADATA *wsaData)
{
    const auto api_ver = MAKEWORD(2, 2);
    if (WSAStartup(api_ver, wsaData) != NO_ERROR)
    {
        fprintf(stderr, "%s", "ERROR INITIALIZING WINSOCK\n" + WSAGetLastError());
        exit(1);
    }
}

void getHostIp(in_addr *aHostAddr)
{
    char hostName[HOSTNAME_MAX_LEN + 1] = {0};
    struct hostent* hostIpAddr;

    // With the Assist of Ido Barak
    if (gethostname(hostName, HOSTNAME_MAX_LEN - 1) != 0)
    {
        fprintf(stderr, "%s", "ERROR IN HOST LOCAL NAME\n");
        exit(1);
    }

    hostIpAddr = gethostbyname(hostName);
    if (hostIpAddr == NULL)
    {
        fprintf(stderr, "%s", "ERROR IN HOST IPv4 ADDRESS\n");
        exit(1);
    }

    // set the host ip address for printing
    memcpy(aHostAddr, hostIpAddr->h_addr_list[0], sizeof(in_addr));
}

SOCKET newSocket(sockaddr_in *aClientAddr, int* aAutoPort, BOOL aIsListen)
{
    SOCKET s;

    // set socket parameters
    aClientAddr->sin_family = AF_INET;
    aClientAddr->sin_port = RANDOM_PORT;
    aClientAddr->sin_addr.s_addr = INADDR_ANY;


    // create the new socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        fprintf(stderr, "%s", "ERROR IN CREATING TCP SOCKET\n" + WSAGetLastError());
        exit(1);
    }

    // if socket is for listening: will handle it correctly
    if (aIsListen)
    {
        // bind
        int bindRes = bind(s, (SOCKADDR*)aClientAddr, sizeof(*aClientAddr));
        if (bindRes)
        {
            fprintf(stderr, "%s", "ERROR IN SOCKET BIND()\n" + WSAGetLastError());
            exit(1);
        }

        //listen
        int listenRes = listen(s, 1);
        if (listenRes)
        {
            fprintf(stderr, "%s", "ERROR LISTENING TO SOCKET\n" + WSAGetLastError());
            exit(1);
        }
    }

    int addrSize = sizeof(*aClientAddr);
    getsockname(s, (SOCKADDR*)aClientAddr, &addrSize);


    *aAutoPort = ntohs(aClientAddr->sin_port);
}
