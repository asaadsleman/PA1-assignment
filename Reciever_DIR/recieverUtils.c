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
#pragma comment(lib, "Ws2_32.lib")

#include "MACROS.h"
#include "recieverUtils.h"


static int g_port;
static char* g_ip;
static unsigned char write_mask = 0;
static unsigned int total_bytes_written = 0;
static unsigned int total_bytes_received = 0;
static unsigned int next_bit_index = 0;         // next bit index in a byte, max val = 7;
static unsigned char write_byte = 0;            // next byte to write to file
static unsigned int errors_corrected = 0;

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
        write_mask = 0;
        write_byte = 0;
        errors_corrected = 0;

        comm_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (comm_sock == INVALID_SOCKET) {
            printf("ERROR: SOCKET CREATION FAILED!\n");
            return 1;
        }
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

        printf("received: %u bytes\n", total_bytes_received);
        printf("wrote: %u bytes\n", total_bytes_written);
        printf("corrected: %u errors\n", errors_corrected);
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
    int parsed_message[MAX_BITS_IN_PACKET] = { 0 };     // packet message parsed to bits
    char message[MAX_BYTES_IN_PACKET];


    // each iteration is per packet
    while (TRUE) {

        recv_result = recv_packet(message, MAX_BYTES_IN_PACKET, p_socket, &packet_size);

        if (recv_result == TRNS_FAILED) {
            if (fclose(fp))
                printf("Error: unable to close file\n");
            return 1;
        }


        get_bits(message, parsed_message, packet_size);
        if (parse_packet(fp, parsed_message, packet_size)) {
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


int parse_packet(FILE* p_file, int* source, int packet_size) {

    int total_bits = packet_size * BITS_IN_BYTE;
    int frames = total_bits / BITS_IN_FRAME;
    int parsed_frame[DATA_BITS_IN_FRAME];
    int temp = 0;

    for (int i = 0; i < frames; i++) {
        decode_hamming(&(source[i * BITS_IN_FRAME]), parsed_frame);
        for (int j = 0; j < DATA_BITS_IN_FRAME; j++) {
            temp = parsed_frame[j];
            temp *= pow(2, (double)BITS_IN_BYTE - 1 - next_bit_index);
            write_byte += temp;
            if (next_bit_index == 7) {
                if (file_write_byte(p_file))
                    return 1;
                next_bit_index = 0;
                write_byte = 0;
            }
            else
                next_bit_index++;
        }
    }
    return 0;
}


void decode_hamming(int* encoded_buffer, int* decoded_buffer) {
    int error_index = 0;
    for (int i = 0; i < BITS_IN_FRAME; i++) {
        if (encoded_buffer[i] == 1)
            error_index ^= (i + 1);
    }

    if (error_index != 0) {
        encoded_buffer[error_index - 1] = !encoded_buffer[error_index - 1];
        errors_corrected++;
    }

    int decoded_buffer_index = 0;
    for (int i = 2; i < BITS_IN_FRAME; i++) {
        if (ceil(log2((double)i + 1)) == floor(log2((double)i + 1)))
            continue;
        decoded_buffer[decoded_buffer_index] = encoded_buffer[i];
        decoded_buffer_index++;
    }
}


int file_write_byte(FILE* p_file) {
    if (fputc(write_byte, p_file) != write_byte) {
        printf("Error: could not write to file");
        return 1;
    }
    total_bytes_written++;
    write_byte = 0;
    return 0;
}


void get_bits(char* read_target, int* data_buffer, int packet_size) {

    int read_mask = 0;
    int temp;
    int ref_index = 0;

    for (int i = 0; i < packet_size; i++) {

        read_mask = 1 << (BITS_IN_BYTE - 1);
        ref_index = i * BITS_IN_BYTE;

        for (int j = 0; j < BITS_IN_BYTE; j++) {
            temp = read_mask & read_target[i];
            if (temp > 0) {
                data_buffer[ref_index + j] = 1;
            }
            read_mask >>= 1;
        }
    }
}


TransferResult_t recv_packet(char* buffer, const int packet_length, SOCKET* p_connection_socket, int* bytes_received) {
    char* p_current_place = buffer;
    int bytes_transferred, bytes_left = packet_length, ret_val = 0, error = 0;

    // recieve bytes of data until done
    while (bytes_left > 0) {

        bytes_transferred = recv(*p_connection_socket, p_current_place, bytes_left, 0);

        if (bytes_transferred == 0 || bytes_transferred == SOCKET_ERROR) {

            if (bytes_transferred == SOCKET_ERROR) {
                error = WSAGetLastError();
                // Assuming connection termination is only because the server completed sending the data
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

        bytes_received += bytes_transferred;
        total_bytes_received += bytes_transferred;
        bytes_left -= bytes_transferred;
        p_current_place += bytes_transferred;
    }

    return TRNS_SUCCEEDED;
}