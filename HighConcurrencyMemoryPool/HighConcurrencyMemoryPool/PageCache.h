// page cache��ҳΪ��λ��span��������
#pragma once

#include "Universal.h"

 // ����ģʽ
class PageCache
{
private:
	SpanList _spanLists[MAX_PAGES];  // ����
	std::unordered_map<PAGE_ID, Span*> _idSpanMap;  // ʹ��mapӳ��ҳ��span��id
	std::mutex _mutex;  // ������

	PageCache()
	{}

	PageCache(const PageCache&) = delete;

public:
	Span* _NewSpan(size_t numPage);  // ��ֹ��ȡ��spanʱ�����ݹ���
	Span* NewSpan(size_t numPage);  // ��ȡһ����span
	void ReleaseSpanToPageCache(Span* span);  // ��span�ͷŻ�page cache
	Span* GetIdToSpan(PAGE_ID id);  // ͨ��span��id�ҵ���span

	static PageCache& GetInstance()
	{
		static PageCache instance;
		return instance;
	}
};
