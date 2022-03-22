#ifndef __client_functions_H__
#define __client_functions_H__
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <math.h>
#pragma comment(lib, "Ws2_32.lib")
#include "MACROS.h"

int start_sender(char* address, int port);
int send_file(char* file_name, SOCKET* p_socket);
int convertReadBytes(char* buffer, SOCKET *send_socket);
int send_encoded(uint32_t *msg, SOCKET *send_socket);
int hamming8Bytes(uint32_t *vals, SOCKET *send_socket);
int parse_buffer(char* buffer, char *bin);
int parity(uint32_t v);
uint32_t computeParityBits(uint32_t h);
uint32_t encode(uint32_t d);