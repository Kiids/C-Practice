// central cache���ʹ�ϣӳ���span������������
#pragma once

#include "Universal.h"

// ����ģʽ
class CentralCache
{
private:
	SpanList _spanLists[FREE_LIST];

	CentralCache()
	{}

	CentralCache(const CentralCache&) = delete;

public:
	size_t GetRangeObj(void*& start, void*& end, size_t num, size_t size);  // �����Ļ����ȡһ�����������thread cache
	void ReleaseListToSpans(void* start, size_t size);  // ��һ�������Ķ����ͷŵ�span
	Span* GetOneSpan(size_t size);// ��spanlist��page cache��ȡһ��span

	static CentralCache& GetInstance()  // ���һ��ʵ��
	{
		static CentralCache instance;
		return instance;
	}
};
