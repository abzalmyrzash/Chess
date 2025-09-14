#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <stdbool.h>

typedef enum {
	NO_THREAD,
	THREAD_STARTED,
	THREAD_FINISHED
} ThreadStatus;

typedef struct {
	SOCKET socket;
	char* recvbuf;
	int recvlen;
	ThreadStatus status;
	int result;
} ReceiveThreadData;

int startReceiveThread(ReceiveThreadData* data, int recvlen);
int sendData(SOCKET socket, char* sendbuf, int sendlen);
