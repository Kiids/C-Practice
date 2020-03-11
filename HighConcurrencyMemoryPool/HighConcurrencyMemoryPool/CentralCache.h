// central cache本质哈希映射的span对象自由链表
#pragma once

#include "Universal.h"

// 单例模式
class CentralCache
{
private:
	SpanList _spanLists[FREE_LIST];

	CentralCache()
	{}

	CentralCache(const CentralCache&) = delete;

public:
	size_t GetRangeObj(void*& start, void*& end, size_t num, size_t size);  // 从中心缓存获取一定数量对象给thread cache
	void ReleaseListToSpans(void* start, size_t size);  // 将一定数量的对象释放到span
	Span* GetOneSpan(size_t size);// 从spanlist或page cache获取一个span

	static CentralCache& GetInstance()  // 获得一个实例
	{
		static CentralCache instance;
		return instance;
	}
};
