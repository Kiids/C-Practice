#define _CRT_SECURE_NO_WARNINGS 1

#include "utensil.hpp"
#include "client_server.hpp"

// �ͻ��˷��������������ֹ�����Ҫʱ�䣬�ʷ���˻�������
void ClientRun()
{
	Client client;
	client.Start();
}

int main()
{
	// �����߳����пͻ���ģ�飬���߳����з����ģ��
	std::thread threadClient(ClientRun);

	Server server;
	server.Start();

	return 0;
}