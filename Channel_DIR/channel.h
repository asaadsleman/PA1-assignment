//
// Created by awsle on 22/03/2022.
//

#pragma once
#pragma comment(lib, "Ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2def.h>
#include <cstdint>
#include <iostream>
#include <stdlib.h>
#include <random>
#include <ws2tcpip.h>

#define RANDOM_PORT         0
#define SINGLE_BIT_MASK		1
#define BITS_IN_BYTE        8
#define FRAME_SIZE_BYTES	31
#define FRAME_SIZE_BITS     248
#define FRAMES_PER_PACKET   20
#define BUFFER_SIZE_BYTES	620
#define BUFFER_SIZE_BITS    4960
#define TWO_POWER_FIFTEEN   32768
#define TWO_POWER_SIXTEEN   65536
#define HOSTNAME_MAX_LEN    350


// initiate winsock instance with wsadata
void InitSock(WSADATA* wsaData);
// channel ip address to be used for sender and reciever
void getHostIp(in_addr* aHostAddr);
// open new socket
SOCKET newSocket(sockaddr_in* aClientAddr, int* aAutoPort, BOOL aIsListen);
void setNoiseType(int noiseCMD, int probCMD, int seedCMD, int spanCMD);
void analyzeArguments(int argc, char**argv);
int getRandom();
uint32_t noiseMessage(uint32_t message);