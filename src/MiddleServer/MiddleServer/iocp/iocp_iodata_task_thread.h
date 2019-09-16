/**
 * @file iocp_iodata_task_thread.h
 * @brief iocp_iodata_task_thread defines
 */

#pragma once

#include <header.h>
#include <task_thread.h>

class CIocpIodataTaskThread: public CTaskThread {
public:
	CIocpIodataTaskThread(const std::string& name, const SOCKET nListenSocket = INVALID_SOCKET, const HANDLE hCompletionPort = INVALID_HANDLE_VALUE) :CTaskThread(name) {
		this->nListenSocket(nListenSocket);
		this->hCompletionPort(hCompletionPort);
	}
	CIocpIodataTaskThread() :CTaskThread() {
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
		CIocpIodataTaskThread* thiz = reinterpret_cast<CIocpIodataTaskThread*>(p);
		if(thiz != nullptr)
		{
			HANDLE hComplationPort = (HANDLE)(thiz->hCompletionPort());
			DWORD nBytesTransferred = 0;
			LPPER_HANDLE_DATA lpHandleData = NULL;
			LPPER_IO_OPERATION_DATA lpIoData = NULL;
			DWORD nSendBytes = 0;
			DWORD nRecvBytes = 0;
			DWORD nFlags = 0;

			while (thiz->IsRunning())
			{
				if (GetQueuedCompletionStatus(hComplationPort, &nBytesTransferred, (PULONG_PTR)& lpHandleData, (LPOVERLAPPED*)& lpIoData, 0) == 0)
				{
					switch (GetLastError())
					{
					case WAIT_TIMEOUT:
					{
						continue;
					}
						break;
					default:
						break;
					}
					LOG(main, LOG_ERROR, "tid=%d GetQueuedCompletionStatus failed. Error:%d\n", std::this_thread::get_id(), GetLastError());
					return;
				}

				// 检查数据是否已经传输完了
				if (nBytesTransferred == 0)
				{
					LOG(main, LOG_ERROR, "tid=%d Start closing socket... Error:%d\n", std::this_thread::get_id(), GetLastError());
					if (CloseHandle((HANDLE)lpHandleData->socket) == SOCKET_ERROR)
					{
						LOG(main, LOG_ERROR, "tid=%d Close socket failed. Error:%d\n", std::this_thread::get_id(), GetLastError());
						return;
					}

					GlobalFree(lpHandleData);
					GlobalFree(lpIoData);
					continue;
				}

				// 检查管道里是否有数据
				if (lpIoData->bytesRecv == 0)
				{
					lpIoData->bytesRecv = nBytesTransferred;
					lpIoData->bytesSend = 0;
				}
				else
				{
					lpIoData->bytesSend += nBytesTransferred;
				}

				// 数据没有发完，继续发送
				if (lpIoData->bytesRecv > lpIoData->bytesSend)
				{
					memset(&(lpIoData->overlapped), 0, sizeof(OVERLAPPED));
					lpIoData->databuff.buf = lpIoData->buffer + lpIoData->bytesSend;
					lpIoData->databuff.len = lpIoData->bytesRecv - lpIoData->bytesSend;

					// 发送数据出去
					if (WSASend(lpHandleData->socket, &(lpIoData->databuff), 1, &nSendBytes, 0, &(lpIoData->overlapped), NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != ERROR_IO_PENDING)
						{
							LOG(main, LOG_ERROR, "tid=%d WSASend() failed. Error:%d\n", std::this_thread::get_id(), GetLastError());
							return;
						}
						else
						{
							LOG(main, LOG_ERROR, "tid=%d  WSASend() failed. io pending. Error:%d\n", std::this_thread::get_id(), GetLastError());
							return;
						}
					}

					LOG(main, LOG_INFO, "tid=%d  Send %.*s\n", std::this_thread::get_id(), nSendBytes - 12, lpIoData->buffer + 12);
				}
				else
				{
					lpIoData->bytesRecv = 0;
					nFlags = 0;

					memset(&(lpIoData->overlapped), 0, sizeof(OVERLAPPED));
					lpIoData->databuff.len = DataBuffSize;
					lpIoData->databuff.buf = lpIoData->buffer;

					if (WSARecv(lpHandleData->socket, &(lpIoData->databuff), 1, &nRecvBytes, &nFlags, &(lpIoData->overlapped), NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != ERROR_IO_PENDING)
						{
							LOG(main, LOG_ERROR, "tid=%d WSARecv() failed. Error:%d\n", std::this_thread::get_id(), GetLastError());
							return;
						}
						else
						{
							LOG(main, LOG_ERROR, "tid=%d WSARecv() io pending. Error:%d\n", std::this_thread::get_id(), GetLastError());
							continue;
						}
					}
				}
			}
		}
	}
};
