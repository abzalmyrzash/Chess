#include "network.h"
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <iphlpapi.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <assert.h>

DWORD WINAPI receiveThread(void* data)
{
	ReceiveThreadData* tdata = data;
	int iResult;
	int recvlen = tdata->recvlen;
	int received = 0;
	while (recvlen > 0) {
		iResult = recv(tdata->socket, tdata->recvbuf + received, recvlen, 0);
		if (iResult > 0) {
			recvlen -= iResult;
			received += iResult;
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
			break;
		} else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(tdata->socket);
			WSACleanup();
			break;
		}
	}

	tdata->result = iResult;
	tdata->status = THREAD_FINISHED;
}

int sendData(SOCKET socket, char* sendbuf, int sendlen) {
	int iResult;
	int sent = 0;
	while (sendlen > 0) {
		iResult = send(socket, sendbuf + sent, sendlen, 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(socket);
			WSACleanup();
			return -1;
		}
		sendlen -= iResult;
		sent += iResult;
	}
	return 0;
}

int startReceiveThread(ReceiveThreadData* data, int recvlen) {
	assert(data->status == NO_THREAD);
	data->status = THREAD_STARTED;
	data->recvlen = recvlen;
	HANDLE thread = CreateThread(NULL, 0, receiveThread, data, 0, NULL);
	if (thread == NULL) return -1;
	return 0;
}
