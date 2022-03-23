//
// Created by awsle on 22/03/2022.
//

#include "channel.h"

int main(int argc, char* argv[])
{

    int senderPort = 0;
    int recieverPort = 0;
    int sockAddrSize = sizeof(sockaddr_in);
    char* noiseFlag;
    int noiseSeed;
    int noiseLevel;
    char continueString[4];
    sockaddr_in ReceiverAddr;
    sockaddr_in SenderAddr;
    in_addr ChannelIpAddress;


    // Buffer size - will increase if file size is too large for current buffer
    char *messageBuffer = (char*)malloc(sizeof(char)*33);
    if (messageBuffer == NULL)
    {
        fprintf(stderr, "FAILURE TO ALLOCATE BUFFER\n");
        exit(1);
    }
    analyzeArguments(argc, argv);

    WSADATA wsadata;
    WinsockInit(&wsadata);


    while (TRUE)
    {
        SOCKET SenderListenSock = newSocket(&SenderAddr, &senderPort, TRUE);
        SOCKET RecieverListenSock = newSocket(&ReceiverAddr, &recieverPort, TRUE);

        getHostIp(&ChannelIpAddress); // getting the current host ip address -> will be set as server ip

        // printing connection info for the user
        fprintf(stdout,"%s", "sender socket: IP address = " + inet_ntoa(ChannelIpAddress));
        fprintf(stdout, "%s", "port = " + senderPort + "\n");
        fprintf(stdout,"%s", "Reciever socket: IP address = " + inet_ntoa(ChannelIpAddress));
        fprintf(stdout, "%s", "port = " + recieverPort = "\n");

        // accepting connection for data send/recieve
        SOCKET SenderDataSock = accept(SenderListenSock, (SOCKADDR*)&SenderAddr, &sockAddrSize);
        SOCKET RecieverDataSock = accept(RecieverListenSock, (SOCKADDR*)&ReceiverAddr, &sockAddrSize);
        fprintf(stdout, "%s", "Got connection for both clients!\n");


        // start getting message
        int bytesRecieved = recv(SenderDataSock, messageBuffer, 33, 0);
        messageBuffer[BUFFER_SIZE_BYTES] = '\0';                               // set the whole buffer as one string

        // Done recieving data from sender
        closesocket(SenderDataSock);

        char * str = "";
        uint32_t temp = 0;
        uint32_t vals[8];
        // convert to ints
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 4; ++j) {
                strcat(str, &messageBuffer[8*i + j]);
            }
            temp =(uint32_t) strtoul(str, NULL, 2);
            vals[i] = noiseMessage(temp);
        }
        // convert back
        for (int i = 0; i < 32; ++i) {
            messageBuffer[i] = (char ) &(temp & 0b11111111);
        }

        // send message with noise
        int bytesSent = send(RecieverDataSock, messageBuffer, bytesRecieved, 0);
        fprintf(stdout, "%s", "retransmitted: %d bytes, ", bytesSent);
        FlippedBits = 0;
        closesocket(RecieverDataSock);

        fprintf(stdout, "continue? (yes/no)\n");
        scanf("%s",continueString);
        if (!strcmp(continueString, "no"))
        {
            std::cout << "Quitting Program!\n";
            exit(0);
        }
    }

    return 0;

}