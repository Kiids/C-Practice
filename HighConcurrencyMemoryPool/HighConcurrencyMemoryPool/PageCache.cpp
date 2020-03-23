#include "PageCache.h"

// 获取一个span
Span* PageCache::_NewSpan(size_t numPage)
{
	if (!_spanLists[numPage].Empty())
	{
		Span* span = _spanLists[numPage].Begin();
		_spanLists[numPage].PopFront();
		return span;
	}

	for (size_t i = numPage + 1; i < MAX_PAGES; i++)
	{
		if (!_spanLists[i].Empty())
		{
			// 切分span
			Span* span = _spanLists[i].Begin();
			_spanLists[i].PopFront();

			Span* splitSpan = new Span;
			splitSpan->_pageid = span->_pageid + span->_pagesize - numPage;
			splitSpan->_pagesize = numPage;
			for (PAGE_ID i = 0; i < numPage; i++)
				_idSpanMap[splitSpan->_pageid + i] = splitSpan;

			span->_pagesize -= numPage;

			_spanLists[span->_pagesize].PushFront(span);

			return splitSpan;
		}
	}

	void* p = SystemAlloc(MAX_PAGES - 1);

	Span* bigSpan = new Span;
	bigSpan->_pageid = (PAGE_ID)p >> PAGE_SHIFT;
	bigSpan->_pagesize = MAX_PAGES - 1;

	for (PAGE_ID i = 0; i < bigSpan->_pagesize; i++)
		_idSpanMap[bigSpan->_pageid + i] = bigSpan;

	_spanLists[bigSpan->_pagesize].PushFront(bigSpan);

	return _NewSpan(numPage);
}

// 获取一个span进行加锁
Span* PageCache::NewSpan(size_t numPage)
{
	_mutex.lock();
	Span* span = _NewSpan(numPage);
	_mutex.unlock();

	return span;
}

// 向页缓存释放内存，同时页缓存合并相邻释放回来的空间
void PageCache::ReleaseSpanToPageCache(Span* span)
{
	// 向前合并
	while (1)
	{
		PAGE_ID prevPageId = span->_pageid - 1;
		auto it = _idSpanMap.find(prevPageId);
		// 不存在前面的页
		if (it == _idSpanMap.end())
			break;

		// 前一个还在使用中，不能合并
		Span* prevSpan = it->second;
		if (prevSpan->_usecount != 0)
			break;

		// 合并，但如果合并之后的span超过128页/最大页，则不合并
		if (span->_pagesize + prevSpan->_pagesize >= MAX_PAGES)
			break;

		span->_pageid = prevSpan->_pageid;
		span->_pagesize += prevSpan->_pagesize;
		for (PAGE_ID i = 0; i < prevSpan->_pagesize; i++)
			_idSpanMap[prevSpan->_pageid + i] = span;

		_spanLists[prevSpan->_pagesize].Erase(prevSpan);
		delete prevSpan;
	}


	// 向后合并
	while (1)
	{
		PAGE_ID nextPageId = span->_pageid + span->_pagesize;
		auto nextIt = _idSpanMap.find(nextPageId);
		if (nextIt == _idSpanMap.end())
			break;

		Span* nextSpan = nextIt->second;
		if (nextSpan->_usecount != 0)
			break;

		// 合并，但如果合并之后的span超过128页，则不合并
		if (span->_pagesize + nextSpan->_pagesize >= MAX_PAGES)
			break;

		span->_pagesize += nextSpan->_pagesize;
		for (PAGE_ID i = 0; i < nextSpan->_pagesize; ++i)
			_idSpanMap[nextSpan->_pageid + i] = span;

		_spanLists[nextSpan->_pagesize].Erase(nextSpan);
		delete nextSpan;
	}

	_spanLists[span->_pagesize].PushFront(span);
}

Span* PageCache::GetIdToSpan(PAGE_ID id)
{
	//std::map<PAGE_ID, Span*>::iterator it = _idSpanMap.find(id);
	auto it = _idSpanMap.find(id);
	if (it != _idSpanMap.end())
		return it->second;
	else
		return nullptr;
}