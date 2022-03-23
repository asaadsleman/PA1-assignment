#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <math.h>
#include <stdint.h>

#pragma comment(lib, "Ws2_32.lib")

#include "MACROS.h"
#include "senderUtils.h"

static int TotalBytesRead = 0;

// initialize sender sockets
int start_sender(char* address, int port){
    SOCKET sender_s = INVALID_SOCKET;
    char f_name[FILENAME_SIZE] = "";

    // Initialize Winsock
    WSADATA wsa_data;
    int result;
    result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        printf("Error: WSAStartup failed: %d\n", result);
        return 1;
    }

    SOCKADDR_IN sender_sockaddr;
    sender_sockaddr.sin_family = AF_INET;
    sender_sockaddr.sin_port = htons(port);
    sender_sockaddr.sin_addr.s_addr = inet_addr(address);

    // start server communications
    while (1) {
        // create socket
        sender_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sender_s == INVALID_SOCKET) {
            printf("ERROR: Failed to create a socket\n");
            return 1;
        }
        // establish connection
        if (connect(sender_s, (SOCKADDR*)&sender_sockaddr, sizeof(sender_sockaddr)) == SOCKET_ERROR) {
            printf("Failed connecting to server on %s:%d\n", address, port);
            return 1;
        }
        // CONNECTION SUCCESS
        printf("enter file name:\n");
        if (scanf("%s", f_name) == NULL) {
            printf("Error: could not read file name\n");
            return 1;
        }
        // QUIT
        if (strcmp(f_name, "quit") == 0) {
            return 0;
        }

        if (send_file(f_name, &sender_s)) {
            return 1;
        }

        printf("file length: %u bytes\n", total_bytes_read);
        printf("sent: %u bytes\n", total_bytes_sent);

    }
}

// send file in packets through connection
int send_file(char* file_name, SOCKET *send_socket) {
    FILE* fp = NULL;
    if (fopen_s(&fp, file_name, "rb")) {
        fprintf(stderr, "Error: failed to read file");
        return 1;
    }

    int endFlag = 0;
    int TotalBytesRead = 0;
    int BytesReadOnce = 0;
    char buffer[26];

    while (TRUE) {

        // read 26 bytes from file
        BytesReadOnce = fread(buffer, sizeof(char), 26, fp);
        if (BytesReadOnce < 26){
            endFlag = 1;
            if(BytesReadOnce == 0){
                break;
            }
        }
        // convert, encode and send bytes
        convertReadBytes(buffer, send_socket);
        TotalBytesRead += BytesReadOnce;
        if (endFlag){
            break;
        }
    }

    if (fclose(fp)) {
        fprintf(stderr, "Error: failed to close file");
        return 1;
    }
    // COMMUNICATION OVER - CLOSE SOCKET
    if (closesocket(*p_socket) == INVALID_SOCKET){
        int error = WSAGetLastError();
        if (error == WSAENOTSOCK || error == WSAECONNABORTED)
            return 0;
        printf("ERROR: Failed to close socket.\n");
        return 1;
    }
    return 0;
}

// takes a 26 byte buffer, encodes it with hamming code as required and
// then sends to the channel
int convertReadBytes(char* buffer, SOCKET *send_socket){
    char bin[208];
    uint32_t vals[8];
    parse_buffer(buffer, bin);
    char *temp = "";
    int ind = 0;
    // convert binary string into 8 unsigned integers
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 26; j++) {
            strcat(temp, &bin[8*i + j]);
        }
        vals[i] = (uint32_t) strtoul(temp, NULL, 2);
        temp = "";
    }

    // encode and send bytes
    hamming8Bytes(vals, send_socket);

    return 0;
}


int hamming8Bytes(uint32_t *vals, SOCKET *send_socket){
    uint32_t temp = 0;
    // encode each number separately
    for (int i = 0; i < 8; ++i) {
        temp = vals[i];
        vals[i] = encode(temp);
    }
    send_encoded(vals, send_socket);
    return 0;
}

int send_encoded(uint32_t *msg, SOCKET *send_socket){
    int sent = 0;
    int err_no = 0;

    sent = send((*connected_sock), msg, sizeof(msg) * 8, 0);
    // PROBLEMS IN SEND
    if (sent == SOCKET_ERROR)
    {
        // CAN WE SEND?
        err_no = WSAGetLastError();
        if (err_no == WSAESHUTDOWN || err_no == WSAENOTSOCK || err_no == WSAEINTR) {
            fprintf(stderr, "FAILED! WSA ERROR NUMBER: %d.\n", err_no);
            return 1;
        }
        // FORCE CLOSE SOCKET
        closesocket(*send_socket);
        *send_socket = INVALID_SOCKET;
        return 1;
    }
    return 0;
}

// converts char array to binary string
int parse_buffer(char* buffer, char *bin){
    char *binstr;
    for (int i = 0; i < 26; i++) {
        binstr = _itoa(buffer[i], binstr, 2);
        for (int j = 0; j < 4; j++) {
            bin[i + j] = binstr[j];
        }
    }
    return 0;
}

int parity(uint32_t v)
{ //implementation from https://gist.github.com/qsxcv/b2f9976763d52bf1e7fc255f52f05f5b#file-secded_31_26-c-L10
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
}

uint32_t computeParityBits(uint32_t h) {
    if(parity(h & 0b10101010101010101010101010101010)) { h|=1<<1;}
    if(parity(h & 0b11001100110011001100110011001100)) { h|=1<<2;}
    if(parity(h & 0b11110000111100001111000011110000)) { h|=1<<4;}
    if(parity(h & 0b11111111000000001111111100000000)) { h|=1<<8;}
    if(parity(h & 0b11111111111111110000000000000000)) { h|=1<<16;}
    return h;
}

uint32_t encode(uint32_t d)
{
    uint32_t first =d&1;
    uint32_t second =d&0b1110;
    uint32_t third =d&0b11111110000;
    uint32_t forth =d&0b11111111111111100000000000;
    uint32_t h =forth<<6|third<<5|second<<4|first<<3; //padding with zeros

    return computeParityBits(h);
}
