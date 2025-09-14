#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>

int startClientAndConnect(char* servername, SOCKET* ConnectSocket);
int closeClient(SOCKET ConnectSocket);
