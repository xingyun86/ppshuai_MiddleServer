// MiddleServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <header.h>
#define socket_type int32_t

class c_info {
	uint16_t code;//消息代码
	int32_t sfd;
	int32_t* cfd;
	uint32_t** send;
	uint32_t** recv;
};

class t_package {
	uint32_t size;//数据大小
	uint16_t code;//消息代码
	uint8_t type;//是否压缩
	int32_t fd;//转发目标
	uint8_t* data;
}t_package;

int main(int argc, char ** argv)
{
	log_test_init();
	iocp_test_main();
    std::cout << "Hello World!\n";

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
