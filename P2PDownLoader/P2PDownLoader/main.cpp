#define _CRT_SECURE_NO_WARNINGS 1

#include "utensil.hpp"
#include "client_server.hpp"

// 客户端发起连接三次握手过程需要时间，故服务端会先启动
void ClientRun()
{
	Client client;
	client.Start();
}

int main()
{
	// 创建线程运行客户端模块，主线程运行服务端模块
	std::thread threadClient(ClientRun);

	Server server;
	server.Start();

	return 0;
}