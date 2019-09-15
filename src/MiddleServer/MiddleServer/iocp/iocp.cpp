// iocp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <header.h>
#include <iocp.h>
#include <iocp_accept_task_thread.h>
#include <iocp_iodata_task_thread.h>

#define NETWORK_STARTUP	[]()->int32_t{WSADATA wsadata = {0};return WSAStartup(MAKEWORD(2,2), &wsadata);}
#define NETWORK_CLEANUP	[]()->int32_t{return WSACleanup();}
DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);

void iocp_test_main()
{
	SOCKET nListenSocket = INVALID_SOCKET;
	HANDLE hCompletionPort = INVALID_HANDLE_VALUE;
	int32_t ret = 0;
	ret = NETWORK_STARTUP();
	if (ret != 0)
	{
		std::cout << "WSAStartup failed. Error:" << ret << std::endl;
		return;
	}

	uint32_t nNumberOfProcessor = std::thread::hardware_concurrency();
	std::unordered_map<std::string, std::shared_ptr<CBaseThread>> task_list;

	std::cout << "nNumberOfProcessor=" << nNumberOfProcessor << std::endl;

	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hCompletionPort == NULL)
	{
		std::cout << "CreateIoCompletionPort failed. Error:" << GetLastError() << std::endl;
		return;
	}
	
	// 启动一个监听socket
	nListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (nListenSocket == INVALID_SOCKET)
	{
		std::cout << " WSASocket( listenSocket ) failed. Error:" << GetLastError() << std::endl;
		return;
	}

	sockaddr_in internetAddr = { 0 };
	internetAddr.sin_family = AF_INET;
	internetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	internetAddr.sin_port = htons(DefaultPort);

	// 绑定监听端口
	if (bind(nListenSocket, (const sockaddr *)& internetAddr, sizeof(internetAddr)) == SOCKET_ERROR)
	{
		std::cout << "Bind failed. Error:" << GetLastError() << std::endl;
		return;
	}

	if (listen(nListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "listen failed. Error:" << GetLastError() << std::endl;
		return;
	}
	std::cout << "listen success on:" << "0.0.0.0" << ":" << DefaultPort << std::endl;

	// 创建 2 * CPU核数 + 1 个线程
	for (uint32_t i = 0; i < nNumberOfProcessor; i++)
	{
		task_list.insert({
			{"iodata" + std::to_string(i),
			std::make_shared<CIocpIodataTaskThread>("iocp_iodata_task", nListenSocket, hCompletionPort)
			},
			{"accept" + std::to_string(i),
			std::make_shared<CIocpAcceptTaskThread>("iocp_accept_task", nListenSocket, hCompletionPort)
			},

			});
	}
	for (auto& it : task_list)
	{
		if (it.second != nullptr)
		{
			it.second->Start();
		}
	}
	
	getchar();

	for (auto& it : task_list)
	{
		if (it.second != nullptr)
		{
			it.second->Stop();
		}
	}
	std::cout << "service stopped!" << std::endl;
}
