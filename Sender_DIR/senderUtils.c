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

#pragma comment(lib, "Ws2_32.lib")

#include "MACROS.h"
#include "senderUtils.h"

// GLOBAL VARIABLES
static int g_port;
static char* g_ip;
static unsigned char read_mask = 0;
static char read_byte;
static unsigned int total_bytes_read = 0;
static unsigned int total_bytes_sent = 0;

// initialize sender sockets
int start_sender(char* address, int port){
    SOCKET sender_s = INVALID_SOCKET;
    g_ip = address;
    g_port = port;
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
    sender_sockaddr.sin_port = htons(port); //The htons function converts a u_short from host to TCP/IP network byte order (which is big-endian).
    sender_sockaddr.sin_addr.s_addr = inet_addr(address);

    // start server communications
    while (TRUE) {

        total_bytes_read = 0;
        total_bytes_sent = 0;
        read_mask = 0;

        sender_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sender_s == INVALID_SOCKET) {
            printf("ERROR: Failed to create a socket\n");
            return 1;
        }

        if (connect(sender_s, (SOCKADDR*)&sender_sockaddr, sizeof(sender_sockaddr)) == SOCKET_ERROR) {
            printf("Failed connecting to server on %s:%d\n", address, port);
            return 1;
        }

        // CONNECTION SUCCESS
        printf("enter file name:\n");
        if (scanf("%s", f_name) == EOF) {
            printf("Error: could not read file name\n");
            return 1;
        }
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
int send_file(char* file_name, SOCKET *p_socket) {
    FILE* fp = NULL;
    if (fopen_s(&fp, file_name, "rb")) {
        fprintf(stderr, "Error: failed to read file");
        return 1;
    }

    int end_of_file = 0;

    int packet_buffer[BYTES_IN_PACKET * BITS_IN_BYTE] = { 0 };
    int bits_in_packet_buffer = 0;

    int frame_data_buffer[DATA_BYTES_IN_FRAME] = { 0 };
    int frame_buffer[BYTES_IN_FRAME] = { 0 };

    int bits_read = 0;
    int bits_in_frame_buffer = 0;

    char message[BYTES_IN_PACKET] = { 0 };


    while (TRUE) {

        while (bits_in_packet_buffer < BYTES_IN_PACKET * BITS_IN_BYTE) {

            // read data for single frame. If the read_file_bits returns -1 and bits_read is zero there is no frame to generate.
            if (read_file_bits(fp, frame_data_buffer, &bits_read) == -1) {
                end_of_file = 1;
                if (bits_read == 0)
                    break;
            }
            bits_in_frame_buffer = bits_read + PARITY_BITS;
            simple_hamming(frame_data_buffer, frame_buffer);
            concatenate_array(packet_buffer, bits_in_packet_buffer, frame_buffer, bits_in_frame_buffer);

            bits_in_packet_buffer += bits_in_frame_buffer;

            bits_read = 0;
            memset(frame_data_buffer, 0, sizeof(frame_data_buffer));
            memset(frame_buffer, 0, sizeof(frame_buffer));

            if (end_of_file)
                break;
        }

        // empty packet
        if (bits_in_packet_buffer == 0)
            break;


        int_to_char(packet_buffer, message, bits_in_packet_buffer / BITS_IN_BYTE);

        if (send_packet(message, bits_in_packet_buffer / BITS_IN_BYTE, p_socket)) {
            if (fclose(fp))
                fprintf(stderr, "Error: failed to close file");
            return 1;
        }

        memset(packet_buffer, 0, sizeof(packet_buffer));
        bits_in_packet_buffer = 0;

        if (end_of_file)
            break;
    }

    if (fclose(fp)) {
        fprintf(stderr, "Error: failed to close file");
        return 1;
    }

    if (closesocket(*p_socket) == INVALID_SOCKET){
        int error = WSAGetLastError();
        if (error == WSAENOTSOCK || error == WSAECONNABORTED)
            return 0;
        printf("ERROR: Failed to close socket.\n");
        return 1;
    }
    return 0;
}

// read file in sizes that fit hamming encoding
int read_file_bits(FILE* p_file, int *data_buffer, int *bits_read){
    while (*bits_read < FDATA_BITS) {
        read_mask >>= 1;
        if (read_mask == 0) {
            // CAN'T READ
            if (fread(&read_byte, 1, 1, p_file) <= 0)
                return -1;
            total_bytes_read++;
            read_mask = 1 << 7;
        }
        // check read bit
        unsigned int temp = read_mask & read_byte;
        if (temp > 0)
            data_buffer[*bits_read] = 1;
        else
            data_buffer[*bits_read] = 0;

        (*bits_read)++;
    }
    return 0;
}

// encode raw data with hamming 31.26
void simple_hamming(int* raw_data, int* encoded) {
    // placeholders for parity bits
    int b1, b2, b4, b8, b16;
    // organize data in encoded buffer
    encoded[2] = raw_data[0]; encoded[4] = raw_data[1];
    encoded[5] = raw_data[2]; encoded[6] = raw_data[3];
    encoded[8] = raw_data[4]; encoded[9] = raw_data[5];
    encoded[10] = raw_data[6]; encoded[11] = raw_data[7];
    encoded[12] = raw_data[8]; encoded[13] = raw_data[9];
    encoded[14] = raw_data[10]; encoded[16] = raw_data[11];
    encoded[17] = raw_data[12]; encoded[18] = raw_data[13];
    encoded[19] = raw_data[14]; encoded[20] = raw_data[15];
    encoded[21] = raw_data[16]; encoded[22] = raw_data[17];
    encoded[23] = raw_data[18]; encoded[24] = raw_data[19];
    encoded[25] = raw_data[20]; encoded[26] = raw_data[21];
    encoded[27] = raw_data[22]; encoded[28] = raw_data[23];
    encoded[29] = raw_data[24]; encoded[30] = raw_data[25];


    //xor and substitute relevant bits
    b1 = encoded[2] ^ encoded[4] ^ encoded[6] ^ encoded[8] ^ encoded[10] ^ encoded[12] ^ encoded[14] ^ encoded[16] ^ encoded[18] ^ encoded[20] ^ encoded[22] ^ encoded[24] ^ encoded[26] ^ encoded[28] ^ encoded[30];
    encoded[0] = b1;

    b2 = encoded[2] ^ encoded[5] ^ encoded[6] ^ encoded[9] ^ encoded[10] ^ encoded[13] ^ encoded[14] ^ encoded[17] ^ encoded[18] ^ encoded[21] ^ encoded[22] ^ encoded[25] ^ encoded[26] ^ encoded[29] ^ encoded[30];
    encoded[1] = b2;

    b4 = encoded[4] ^ encoded[5] ^ encoded[6] ^ encoded[11] ^ encoded[12] ^ encoded[13] ^ encoded[14] ^ encoded[19] ^ encoded[20] ^ encoded[21] ^ encoded[22] ^ encoded[27] ^ encoded[28] ^ encoded[29] ^ encoded[30];
    encoded[3] = b4;

    b8 = encoded[8] ^ encoded[9] ^ encoded[10] ^ encoded[11] ^ encoded[12] ^ encoded[13] ^ encoded[14] ^ encoded[23] ^ encoded[24] ^ encoded[25] ^ encoded[26] ^ encoded[27] ^ encoded[28] ^ encoded[29] ^ encoded[30];
    encoded[7] = b8;

    b16 = encoded[16] ^ encoded[17] ^ encoded[18] ^ encoded[19] ^ encoded[20] ^ encoded[21] ^ encoded[22] ^ encoded[23] ^ encoded[24] ^ encoded[25] ^ encoded[26] ^ encoded[27] ^ encoded[28] ^ encoded[29] ^ encoded[30];
    encoded[15] = b16;
}

// parse integer array to char array (parse every 8 bits)
void int_to_char(int* source, char* dest, int characters){
    int temp = 0;
    int curr = 0;
    int helper_index = 0;

    for (int i = 0; i < characters; i++) {
        curr = 0;
        helper_index = 8 * i;
        for (int j = 0; j < 8; j++) {
            temp = source[j + helper_index];
            temp *= pow(2, (double)(7 - j));
            curr += temp;
        }
        dest[i] = curr;
    }
}

// send fixed length message (packet)
int send_packet(char* buffer, const int length, SOCKET* connected_sock) {
    char* curr_In = buffer; // current index
    int sent, error_No; // error_No = error number
    int empty = length;

    while (empty > 0) {
        sent = send((*connected_sock), curr_Ind, empty, 0);
        if (sent == SOCKET_ERROR)
        {
            // CAN WE SEND?
            error_No = WSAGetLastError();
            if (error_No == WSAESHUTDOWN || error_No == WSAENOTSOCK || error_No == WSAEINTR) {
                fprintf(stderr, "FAILED! WSA ERROR NUMBER: %d.\n", error_No);
                return 1;
            }
            // FORCE CLOSE SOCKET
            closesocket(*connected_sock);
            *connected_sock = INVALID_SOCKET;
            return 1;
        }
        // UPDATE COUNTERS
        total_bytes_sent += sent;
        empty -= sent;
        curr_Ind += sent;
    }
    return 0;
}

// concat addOn to stream, from index pos
void add_to_buffer(int* stream, int pos, int* addOn, int size) {
    for (int i = 0; i < size; i++) {
        stream[pos + i] = addOn[i];
    }
}