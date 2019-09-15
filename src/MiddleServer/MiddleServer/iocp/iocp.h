// iocp.h : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef __IOCP_H__
#define __IOCP_H__

#include <winsock2.h>
#pragma comment( lib, "ws2_32.lib" )

#define DefaultPort		20000
#define DataBuffSize	1024 * 1024

typedef struct PER_IO_OPERATEION_DATA
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	CHAR buffer[DataBuffSize];
	DWORD bytesSend;
	DWORD bytesRecv;
}PER_IO_OPERATEION_DATA, * LPPER_IO_OPERATION_DATA;

typedef struct PER_HANDLE_DATA
{
	SOCKET socket;
}PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

void iocp_test_main();

#endif