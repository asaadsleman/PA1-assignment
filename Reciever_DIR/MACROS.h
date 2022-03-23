//
// Created by awsle on 22/03/2022.
//

#ifndef PA1_ASSIGNMENT_MACROS_H
#define PA1_ASSIGNMENT_MACROS_H
#endif //PA1_ASSIGNMENT_MACROS_H
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



#define FILENAME_SIZE					500												// Define max file name length to 300 (includin extentions)


typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;


