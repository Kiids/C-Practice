#include "CentralCache.h"
#include "PageCache.h"

// ��ȡһ��span����
Span* CentralCache::GetOneSpan(size_t size)
{
	size_t index = SizeClass::ListIndex(size);
	SpanList& spanList = _spanLists[index];
	Span* it = spanList.Begin();
	while (it != spanList.End())
	{
		if (!it->_freeList.Empty())
			return it;
		else
			it = it->_next;
	}

	// ��page cache��ȡһ��span
	size_t numPage = SizeClass::NumMovePage(size);
	Span* span = PageCache::GetInstance().NewSpan(numPage);
	// ��span�����г���Ӧ��С�ҵ�span������������
	char* start = (char*)(span->_pageid << 12);
	char* end = start + (span->_pagesize << 12);
	while (start < end)
	{
		char* obj = start;
		start += size;

		span->_freeList.Push(obj);
	}
	span->_objSize = size;
	spanList.PushFront(span);

	return span;
}

// �зִ�С
size_t CentralCache::GetRangeObj(void*& start, void*& end, size_t num, size_t size)
{
	size_t index = SizeClass::ListIndex(size);
	SpanList& spanList = _spanLists[index];
	spanList.Lock();  // ����

	Span* span = GetOneSpan(size);
	FreeList& freeList = span->_freeList;
	size_t actualNum = freeList.PopScope(start, end, num);
	span->_usecount += actualNum;

	spanList.Unlock();  // ����

	return actualNum;
}

// �ͷ�һ�οռ��span
void CentralCache::ReleaseListToSpans(void* start, size_t size)
{
	size_t index = SizeClass::ListIndex(size);
	SpanList& spanList = _spanLists[index];
	spanList.Lock();  // ����

	while (start)
	{
		void* next = Next(start);
		PAGE_ID id = (PAGE_ID)start >> PAGE_SHIFT;
		Span* span = PageCache::GetInstance().GetIdToSpan(id);
		span->_freeList.Push(start);
		span->_usecount--;

		// ��ǰspan�����ȥ�Ķ���ȫ�����أ����Խ���ǰspan����page cache�ϲ��������ڴ���Ƭ
		if (span->_usecount == 0)
		{
			size_t index = SizeClass::ListIndex(span->_objSize);
			_spanLists[index].Erase(span);
			span->_freeList.Clear();

			PageCache::GetInstance().ReleaseSpanToPageCache(span);
		}

		start = next;
	}

	spanList.Unlock();  // ����
}