#include "CentralCache.h"
#include "PageCache.h"

// 获取一个span对象
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

	// 从page cache获取一个span
	size_t numPage = SizeClass::NumMovePage(size);
	Span* span = PageCache::GetInstance().NewSpan(numPage);
	// 把span对象切成相应大小挂到span的自由链表中
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

// 切分大小
size_t CentralCache::GetRangeObj(void*& start, void*& end, size_t num, size_t size)
{
	size_t index = SizeClass::ListIndex(size);
	SpanList& spanList = _spanLists[index];
	spanList.Lock();  // 加锁

	Span* span = GetOneSpan(size);
	FreeList& freeList = span->_freeList;
	size_t actualNum = freeList.PopScope(start, end, num);
	span->_usecount += actualNum;

	spanList.Unlock();  // 解锁

	return actualNum;
}

// 释放一段空间给span
void CentralCache::ReleaseListToSpans(void* start, size_t size)
{
	size_t index = SizeClass::ListIndex(size);
	SpanList& spanList = _spanLists[index];
	spanList.Lock();  // 加锁

	while (start)
	{
		void* next = Next(start);
		PAGE_ID id = (PAGE_ID)start >> PAGE_SHIFT;
		Span* span = PageCache::GetInstance().GetIdToSpan(id);
		span->_freeList.Push(start);
		span->_usecount--;

		// 当前span分离出去的对象全部返回，可以将当前span还给page cache合并，减少内存碎片
		if (span->_usecount == 0)
		{
			size_t index = SizeClass::ListIndex(span->_objSize);
			_spanLists[index].Erase(span);
			span->_freeList.Clear();

			PageCache::GetInstance().ReleaseSpanToPageCache(span);
		}

		start = next;
	}

	spanList.Unlock();  // 解锁
}