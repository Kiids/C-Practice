// thread cache ������һ����ϣӳ��Ķ�������������
#pragma once

#include "Universal.h"

class ThreadCache
{
private:
	FreeList _freeLists[FREE_LIST];  // �̻߳�ȡ�ڴ����������

	//ThreadCache* _next;
	//int threadid;

public:
	void* Allocte(size_t size);              // �����ڴ�
	void Deallocte(void* ptr, size_t size);  // �ͷ��ڴ�

	void* GetFromCentralCache(size_t index);                        // �����Ļ����ȡ����
	void ListTooLong(FreeList& freeList, size_t num, size_t size);  // ������������ж�����������һ�������������ͷŸ����Ļ������
};

// ThreadCache* tclist = nullptr;
// �߳�TLS Thread Local Storage ��֤Ч��

_declspec (thread) static ThreadCache* pThreaCache = nullptr;

// �ֲ߳̾��洢��Thread Local Storage��TLS��������������һ������ִ�е�ָ���̹߳���������
// ��Ҫ��Ϊ�˱������߳�ͬʱ�ô�ͬһȫ�ֱ������߾�̬����ʱ�����µĳ�ͻ�������Ƕ���߳�ͬʱ��Ҫ�޸���һ����ʱ��
// ͨ��TLS���ƣ�Ϊÿһ��ʹ�ø�ȫ�ֱ������̶߳��ṩһ������ֵ�ĸ�����ÿһ���߳̾����Զ����ظı��Լ��ĸ�����������������̵߳ĸ�����ͻ��
// ���̵߳ĽǶȿ����ͺ���ÿһ���̶߳���ȫӵ�иñ�����
// ��ȫ�ֱ����ĽǶ����������ͺ���һ��ȫ�ֱ�������¡���˶�ݸ�������ÿһ�ݸ��������Ա�һ���̶߳����ظı䡣