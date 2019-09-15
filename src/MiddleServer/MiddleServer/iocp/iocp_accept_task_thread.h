/**
 * @file iocp_accept_task_thread.h
 * @brief iocp_accept_task_thread defines
 */

#pragma once

#include <header.h>
#include <task_thread.h>

class CIocpAcceptTaskThread: public CTaskThread {
public:
	CIocpAcceptTaskThread(const std::string & name, const SOCKET nListenSocket = INVALID_SOCKET, const HANDLE hCompletionPort = INVALID_HANDLE_VALUE):CTaskThread(name) {
		this->nListenSocket(nListenSocket);
		this->hCompletionPort(hCompletionPort);
	}
	CIocpAcceptTaskThread():CTaskThread() {
		this->nListenSocket(INVALID_SOCKET);
		this->hCompletionPort(nullptr);
	}
private:
	SOCKET m_nListenSocket;
	HANDLE m_hCompletionPort;
public:
	const SOCKET nListenSocket()
	{
		return (const SOCKET)m_nListenSocket;
	}
	void nListenSocket(const SOCKET nListenSocket)
	{
		m_nListenSocket = (SOCKET)nListenSocket;
	}
	const void* hCompletionPort()
	{
		return (const void*)m_hCompletionPort;
	}
	void hCompletionPort(const void* hCompletionPort)
	{
		m_hCompletionPort = (void*)hCompletionPort;
	}
public:
	virtual void Handler(void* p)
	{
		CIocpAcceptTaskThread* thiz = reinterpret_cast<CIocpAcceptTaskThread*>(p);
		if(thiz != nullptr)
		{
			SOCKET nListenSocket = (SOCKET)(thiz->nListenSocket());
			HANDLE hCompletionPort = (HANDLE)(thiz->hCompletionPort());
			DWORD nBytesTransferred = 0;
			LPPER_HANDLE_DATA lpHandleData = NULL;
			LPPER_IO_OPERATION_DATA lpIoData = NULL;
			DWORD nSendBytes = 0;
			DWORD nRecvBytes = 0;
			DWORD nFlags = 0;
			SOCKET nAcceptSocket = INVALID_SOCKET;

			// 开始死循环，处理数据
			while (thiz->IsRunning())
				{
					nAcceptSocket = WSAAccept(nListenSocket, NULL, NULL, NULL, 0);
					if (nAcceptSocket == SOCKET_ERROR)
					{
						LOG(main, LOG_ERROR, "tid=%d WSAAccept failed. Error:%d\n", std::this_thread::get_id(), GetLastError());
						//std::cout << "tid=" << std::this_thread::get_id() << " WSAAccept failed. Error:" << GetLastError() << std::endl;
						return;
					}

					lpHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
					if (lpHandleData == NULL)
					{
						LOG(main, LOG_ERROR, "tid=%d GlobalAlloc( HandleData ) failed. Error:%d\n", std::this_thread::get_id(), GetLastError());
						//std::cout << "tid=" << std::this_thread::get_id() << " GlobalAlloc( HandleData ) failed. Error:" << GetLastError() << std::endl;
						return;
					}
					memset(lpHandleData, 0, sizeof(PER_HANDLE_DATA));

					lpHandleData->socket = nAcceptSocket;
					if (CreateIoCompletionPort((HANDLE)nAcceptSocket, hCompletionPort, (ULONG_PTR)lpHandleData, 0) == NULL)
					{
						LOG(main, LOG_ERROR, "tid=%d CreateIoCompletionPort failed. Error:%d\n", std::this_thread::get_id(), GetLastError());
						//std::cout << "tid=" << std::this_thread::get_id() << " CreateIoCompletionPort failed. Error:" << GetLastError() << std::endl;
						return;
					}

					lpIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
					if (lpIoData == NULL)
					{
						LOG(main, LOG_ERROR, "tid=%d GlobalAlloc( IoData ) failed. Error:%d\n", std::this_thread::get_id(), GetLastError());
						//std::cout << "tid=" << std::this_thread::get_id() << " GlobalAlloc( IoData ) failed. Error:" << GetLastError() << std::endl;
						return;
					}
					memset(lpIoData, 0, sizeof(PER_IO_OPERATEION_DATA));

					lpIoData->bytesSend = 0;
					lpIoData->bytesRecv = 0;
					lpIoData->databuff.len = DataBuffSize;
					lpIoData->databuff.buf = lpIoData->buffer;

					nFlags = 0;
					if (WSARecv(nAcceptSocket, &(lpIoData->databuff), 1, &nRecvBytes, &nFlags, &(lpIoData->overlapped), NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != ERROR_IO_PENDING)
						{
							LOG(main, LOG_ERROR, "tid=%d WSARecv() failed. Error:%d\n", std::this_thread::get_id(), GetLastError());
							//std::cout << "tid=" << std::this_thread::get_id() << " WSARecv() failed. Error:" << GetLastError() << std::endl;
							return;
						}
						else
						{
							LOG(main, LOG_ERROR, "tid=%d WSARecv() io pending. Error:%d\n", std::this_thread::get_id(), GetLastError());
							//std::cout << "tid=" << std::this_thread::get_id() << " WSARecv() io pending" << std::endl;
							continue;
						}
					}
				}
		}
	}
};
