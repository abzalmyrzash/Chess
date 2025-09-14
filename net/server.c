#include "server.h"
#include <stdio.h>
#include <iphlpapi.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"

int startServerAndWaitForClient(SOCKET* ListenSocket, SOCKET* ClientSocket) {
	WSADATA wsaData;
	int iResult;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed %d\n", iResult);
		return -1;
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof (hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return -1;
	}

	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
		fprintf(stderr, "Error getting hostname: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Local Host Name: %s\n", hostname);

	struct hostent *phe = gethostbyname(hostname);
	if (phe == NULL) {
		fprintf(stderr, "Bad host lookup: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		printf("IP Address %d: %s\n", i + 1, inet_ntoa(addr));
	}

	*ListenSocket = INVALID_SOCKET;
	// Create a SOCKET for the server to listen for client connections

	*ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (*ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return -1;
	}

	// Setup the TCP listening socket
	iResult = bind(*ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(*ListenSocket);
		WSACleanup();
		return -1;
	}
	freeaddrinfo(result);

	if ( listen(*ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
		printf( "Listen failed with error: %ld\n", WSAGetLastError() );
		closesocket(*ListenSocket);
		WSACleanup();
		return -1;
	}
	
	*ClientSocket = INVALID_SOCKET;
	
	// Accept a client socket
	printf("Waiting for client...\n");
	*ClientSocket = accept(*ListenSocket, NULL, NULL);
	if (*ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(*ListenSocket);
		WSACleanup();
		return -1;
	}
}

int closeServer(SOCKET ListenSocket, SOCKET ClientSocket) {
	// No longer need server socket
	closesocket(ListenSocket);

	// shutdown the send half of the connection since no more data will be sent
	int iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return -1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
	
	return 0;
}
