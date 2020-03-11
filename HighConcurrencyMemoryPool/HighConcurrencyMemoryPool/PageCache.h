// page cache��ҳΪ��λ��span��������
#pragma once

#include "Universal.h"
 // ����ģʽ
class PageCache
{
private:
	SpanList _spanLists[MAX_PAGES];  // ��������
	std::map<PAGE_ID, Span*> _idSpanMap;  // ʹ��mapӳ��ҳ��span��id
	std::mutex _mutex;  // ������

	PageCache()
	{}

	PageCache(const PageCache&) = delete;

public:
	Span* _NewSpan(size_t numpage);  // ��ֹ��ȡ��spanʱ�����ݹ���
	Span* NewSpan(size_t numpage);  // ��ȡһ����span
	void ReleaseSpanToPageCache(Span* span);  // ��span�ͷŻ�page cache
	Span* GetIdToSpan(PAGE_ID id);  // ��ȡspan��id

	static PageCache& GetInstance()
	{
		static PageCache instance;
		return instance;
	}
};
