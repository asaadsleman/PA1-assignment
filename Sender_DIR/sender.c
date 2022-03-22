#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>

#include "senderUtils.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("ERROR: incorrect args num\n");
        exit(1);
    }

    int port = atoi(argv[2]);

    if (port < 0 || port > 65535) {
        printf("ERROR: incorrect port number\n");
        exit(1);
    }

    int result = start_sender(argv[1], port);

    return result;
}