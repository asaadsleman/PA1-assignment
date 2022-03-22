//
// Created by awsle on 22/03/2022.
//

#ifndef PA1_ASSIGNMENT_MACROS_H
#define PA1_ASSIGNMENT_MACROS_H

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

//#include <WinSock2.h>
#include <Windows.h>
//#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")


// Bounds (when possible)
#define BITS_IN_BYTE			8
#define DATA_BITS_IN_FRAME		26												// Total number of data bits in frame
#define BITS_IN_FRAME			31												// Total number of bits in frame, including the check bits
#define DEFAULT_HAMMING_BITS	5												// Default number of Hamming check bits
#define FRAMES_IN_PACKET		32												// Max number of Hamming intervals in a packet
#define MAX_BYTES_IN_PACKET		FRAMES_IN_PACKET		* BITS_IN_FRAME			// Total number of bytes in packet
#define DATA_BYTES_IN_PACKET	FRAMES_IN_PACKET		* DATA_BITS_IN_FRAME	// Total number of data bytes in packet
#define MAX_BITS_IN_PACKET		MAX_BYTES_IN_PACKET		* BITS_IN_BYTE			// Max number of bits in packet
#define DATA_BITS_IN_PACKET		DATA_BYTES_IN_PACKET	* BITS_IN_BYTE			// Total number of bits in packet
#define FILENAME_SIZE					500												// Define max file name length to 300 (includin extentions)


typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;


#endif //PA1_ASSIGNMENT_MACROS_H
