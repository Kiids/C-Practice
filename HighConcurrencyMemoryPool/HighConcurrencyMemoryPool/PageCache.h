// page cache以页为单位的span自由链表
#pragma once

#include "Universal.h"

 // 单例模式
class PageCache
{
private:
	SpanList _spanLists[MAX_PAGES];  // 链表
	std::unordered_map<PAGE_ID, Span*> _idSpanMap;  // 使用map映射页与span的id
	std::mutex _mutex;  // 互斥锁

	PageCache()
	{}

	PageCache(const PageCache&) = delete;

public:
	Span* _NewSpan(size_t numPage);  // 防止获取新span时发生递归锁
	Span* NewSpan(size_t numPage);  // 获取一个新span
	void ReleaseSpanToPageCache(Span* span);  // 将span释放回page cache
	Span* GetIdToSpan(PAGE_ID id);  // 通过span的id找到该span

	static PageCache& GetInstance()
	{
		static PageCache instance;
		return instance;
	}
};
