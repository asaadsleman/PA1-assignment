//
// Created by awsle on 22/03/2022.
//

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
#include <WS2tcpip.h>
#include <math.h>
#include <stdint.h>
#pragma comment(lib, "Ws2_32.lib")

#include "MACROS.h"
#include "recieverUtils.h"


static int g_port;
static char* g_ip;
static unsigned int total_bytes_written = 0;
static unsigned int total_bytes_received = 0;
static int flipped=0;

int start_reciever(char* address, int port){
    SOCKET comm_sock = INVALID_SOCKET;
    g_ip = address;
    g_port = port;
    char filename[FILENAME_SIZE] = "";

    WSADATA wsa_data;
    int result;
    result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        printf("ERROR: FAILED WSAStartup number %d\n", result);
        return 1;
    }

    SOCKADDR_IN receiver_s;
    receiver_s.sin_family = AF_INET;
    receiver_s.sin_port = htons(port);
    receiver_s.sin_addr.s_addr = inet_addr(address);

    // talk to server
    while (1) {

        total_bytes_received = 0;
        total_bytes_written = 0;

        // OPEN SOCKET
        comm_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (comm_sock == INVALID_SOCKET) {
            printf("ERROR: SOCKET CREATION FAILED!\n");
            return 1;
        }
        // ESTABLISH CONNECTION
        if (connect(comm_sock, (SOCKADDR*)&receiver_s, sizeof(receiver_s)) == SOCKET_ERROR) {
            printf("CONNECTION TO SERVER FAILED ON %s:%d\n", address, port);
            return 1;
        }
        // CONNECTION SUCCESS
        printf("enter file name:\n");
        if (scanf("%s", filename) == NULL) {
            printf("ERROR READING OUT FILE\n");
            return 1;
        }
        // ON QUIT
        if (strcmp(filename, "quit") == 0) {
            return 0;
        }

        if (communicate_server(filename, &comm_sock)) {
            return 1;
        }
    }
}
int communicate_server(char* file_name, SOCKET* p_socket) {
    FILE* fp;
    if (fopen_s(&fp, file_name, "w")) {
        fprintf(stderr, "ERROR OPENING FILE\n");
        return 1;
    }

    TransferResult_t recv_result;                       // recv_packet result
    int packet_size = 0;                                // number of bytes in received packet
    uint32_t parsed_message[8] = { 0 };     // packet message parsed to bits
    char message[32];


    // each iteration is per packet
    while (TRUE) {

        recv_result = recv_packet(message,p_socket);

        if (recv_result == TRNS_FAILED) {
            if (fclose(fp))
                printf("Error: unable to close file\n");
            return 1;
        }

        if (parse_packet(fp, message)) {
            if (fclose(fp))
                printf("Error: unable to close file\n");
            return 1;
        }

        if (recv_result == TRNS_DISCONNECTED) {
            if (fclose(fp)) {
                printf("Error: unable to close file\n");
                return 1;
            }
            return 0;
        }
    }
}
// parse 32 byte chars into 8 uint32_t vals, decode them
int parse_packet(FILE* p_file, char * source) {
    uint32_t temp = 0;
    uint32_t vals[8];
    char * str = "";

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; ++j) {
            strcat(str, &source[8*i + j]);
        }
        temp =(uint32_t) strtoul(str, NULL, 2);
        vals[i] = temp;
    }
    for (int i = 0; i < 8; ++i) {
        temp = decode(vals[i]);
        str = (char *) &(temp & 0b00000011111111111111111111111111);
        if (fputs(str, p_file) == EOF) {
            fprintf(stderr, "ERROR IN WRITING TO FILE!\n");
            return 1;
        }
    }
    return 0;
}
// receive 32 bit buffer
TransferResult_t recv_packet(char* buffer, SOCKET* p_connection_socket) {
    char* p_current_place = buffer;
    int bytes_transferred, bytes_left = 32, ret_val = 0, error = 0;

    // recieve bytes of data until done
    while (bytes_left > 0) {

        bytes_transferred = recv(*p_connection_socket, p_current_place, bytes_left, 0);

        if (bytes_transferred == 0 || bytes_transferred == SOCKET_ERROR) {

            if (bytes_transferred == SOCKET_ERROR) {
                error = WSAGetLastError();
                // TERMINATED CONNECTION
                if (error == WSAENOTSOCK || error == WSAEINTR)
                    return TRNS_DISCONNECTED;
                printf("Error: receive failed because of %d.\n", error);
                ret_val = TRNS_FAILED;
            }

            else
                ret_val = TRNS_DISCONNECTED;

            // close socket
            if (*p_connection_socket != INVALID_SOCKET) {

                if (closesocket(*p_connection_socket) == INVALID_SOCKET) {
                    error = WSAGetLastError();
                    if (error != WSAENOTSOCK && error != WSAEINTR)
                        printf("ERROR: Failed to close socket. Error number %d.\n", WSAGetLastError());
                }
                else
                    *p_connection_socket = INVALID_SOCKET;

            }

            return ret_val;

        }

        total_bytes_received += bytes_transferred;
        bytes_left -= bytes_transferred;
        p_current_place += bytes_transferred;
    }

    return TRNS_SUCCEEDED;
}
int parity(uint32_t v)
{
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
}
uint32_t findErrorLocation(uint32_t h) {
    //computing the parity check according to the algorithm
    uint32_t i =
            parity(h & 0b10101010101010101010101010101010) |
            parity(h & 0b11001100110011001100110011001100) << 1 |
            parity(h & 0b11110000111100001111000011110000) << 2 |
            parity(h & 0b11111111000000001111111100000000) << 3 |
            parity(h & 0b11111111111111110000000000000000) << 4;
    return i;
}
uint32_t removeParityBits(uint32_t h){
    uint32_t result=((h >> 3) & 1) | //bit at index 3
                    ((h >> 4) & 0b1110| //bits at indices 5-7
                     ((h >> 5) & 0b11111110000) |//bits at indices 9-15
                     ((h >> 6) & 0b11111111111111100000000000)); //bits at indices 17-31
    return result;
}
uint32_t decode(uint32_t h)
{
    uint32_t i = findErrorLocation(h);
    if(h%2==1){ //handling the case that bit in index 0 was flipped by channel,
        // this will not be detected by the hamming code algorithm because it ignores bit 0.
        h^=1;
        flipped++;
    }
    if (i != 0) {
        h ^= (1 << i);
        flipped++;
    }
    return removeParityBits(h);
}
