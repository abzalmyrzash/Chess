#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>

int startServerAndWaitForClient(SOCKET* ListenSocket, SOCKET* ClientSocket);
int closeServer(SOCKET ListenSocket, SOCKET ClientSocket);
