//
// Created by asaad sleman
//

#ifndef PA1_ASSIGNMENT_RECIEVERUTILS_H
#define PA1_ASSIGNMENT_RECIEVERUTILS_H
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <math.h>
#pragma comment(lib, "Ws2_32.lib")

#include "MACROS.h"


int start_reciever(char* address, int port);
int communicate_server(char* file_name, SOCKET* p_socket);
TransferResult_t recv_packet(char* buffer, const int packet_length, SOCKET* p_connection_socket, int* bytes_received);
int parse_packet(FILE* p_file, char * source);
int parity(uint32_t v);
uint32_t findErrorLocation(uint32_t h);
uint32_t removeParityBits(uint32_t h);
uint32_t decode(uint32_t h);