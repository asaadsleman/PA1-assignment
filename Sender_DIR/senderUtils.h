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
#include "HardCodedData.h"

int start_sender(char* address, int port);

int send_file(char* file_name, SOCKET* p_socket);

int read_file_bits(FILE* p_file, int* data_buffer, int* bits_read);

void simple_hamming(int* data_buffer, int* frame_buffer);

int send_packet(char* buffer, const int message_len, SOCKET* p_connection_socket);

void add_to_buffer(int* stream, int pos, int* addOn, int size);

void int_to_char(int* source, char* dest, int num_of_bytes);