#include "PageCache.h"

// ��ȡһ��span
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
			// �з�span
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

// ��ȡһ��span���м���
Span* PageCache::NewSpan(size_t numPage)
{
	_mutex.lock();
	Span* span = _NewSpan(numPage);
	_mutex.unlock();

	return span;
}

// ��ҳ�����ͷ��ڴ棬ͬʱҳ����ϲ������ͷŻ����Ŀռ�
void PageCache::ReleaseSpanToPageCache(Span* span)
{
	// ��ǰ�ϲ�
	while (1)
	{
		PAGE_ID prevPageId = span->_pageid - 1;
		auto it = _idSpanMap.find(prevPageId);
		// ������ǰ���ҳ
		if (it == _idSpanMap.end())
			break;

		// ǰһ������ʹ���У����ܺϲ�
		Span* prevSpan = it->second;
		if (prevSpan->_usecount != 0)
			break;

		// �ϲ���������ϲ�֮���span����128ҳ/���ҳ���򲻺ϲ�
		if (span->_pagesize + prevSpan->_pagesize >= MAX_PAGES)
			break;

		span->_pageid = prevSpan->_pageid;
		span->_pagesize += prevSpan->_pagesize;
		for (PAGE_ID i = 0; i < prevSpan->_pagesize; i++)
			_idSpanMap[prevSpan->_pageid + i] = span;

		_spanLists[prevSpan->_pagesize].Erase(prevSpan);
		delete prevSpan;
	}


	// ���ϲ�
	while (1)
	{
		PAGE_ID nextPageId = span->_pageid + span->_pagesize;
		auto nextIt = _idSpanMap.find(nextPageId);
		if (nextIt == _idSpanMap.end())
			break;

		Span* nextSpan = nextIt->second;
		if (nextSpan->_usecount != 0)
			break;

		// �ϲ���������ϲ�֮���span����128ҳ���򲻺ϲ�
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