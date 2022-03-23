//
// Created by awsle on 22/03/2022.
//

#include "channel.h"


int noise, prob, messagesInCycle,bitToFlip;
int currentMessageInCycle=0;
int flipped=0;


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
        fprintf(stderr, "ERROR IN HOST LOCAL NAME\n");
        exit(1);
    }

    hostIpAddr = gethostbyname(hostName);
    if (hostIpAddr == NULL)
    {
        fprintf(stderr, "ERROR IN HOST IPv4 ADDRESS\n");
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
        fprintf(stderr, "ERROR IN CREATING TCP SOCKET %d\n",WSAGetLastError());
        exit(1);
    }

    // if socket is for listening: will handle it correctly
    if (aIsListen)
    {
        // bind
        int bindRes = bind(s, (SOCKADDR*)aClientAddr, sizeof(*aClientAddr));
        if (bindRes)
        {
            fprintf(stderr, "ERROR IN SOCKET BIND() %d\n" ,WSAGetLastError());
            exit(1);
        }

        //listen
        int listenRes = listen(s, 1);
        if (listenRes)
        {
            fprintf(stderr, "ERROR LISTENING TO SOCKET: %d\n", WSAGetLastError());
            exit(1);
        }
    }

    int addrSize = sizeof(*aClientAddr);
    getsockname(s, (SOCKADDR*)aClientAddr, &addrSize);
    *aAutoPort = ntohs(aClientAddr->sin_port);
}

void setNoiseType(int noiseCMD, int probCMD, int seedCMD, int spanCMD) {
    noise = noiseCMD;
    prob = probCMD / (2 ^ -16);
    srand(seedCMD);
    if(noiseCMD==0){ //deterministic
        messagesInCycle=(int)floorf(spanCMD/31.0)+1;
        bitToFlip=spanCMD%31;
    }

}

void analyzeArguments(int argc, char**argv) {
    char* end;
    if(argc==3){
        if(strcmp(argv[1], "-d")==0){
            setNoiseType(0,0,0,strtol(argv[2], & end, 10));
        }
    }
    else if(argc==4){
        if(strcmp(argv[1], "-r")==0){
            setNoiseType(1,strtol(argv[2], &end, 10),
                         strtol(argv[3],&end,10),0);
        }
    }
    else{
        printf("Invalid number of arguments.\n");
    }
}
int getRandom(){ //returns 1 with probability of n/2^-16
    return rand()/RAND_MAX<=prob;
}

uint32_t noiseMessage(uint32_t message){
    int i;
    if(noise){//random
        for(i=0; i<32; i++){
            if (getRandom()){
                message^=1<<i;
                flipped++;
            }
        }return message;

    }
    else{ //deterministic
        currentMessageInCycle++;
        if(currentMessageInCycle!=messagesInCycle){
            return message;
        }
        else{
            message^=1<<bitToFlip;
            flipped++;
            currentMessageInCycle=0;
            return message;
        }
    }

}