#include "ThreadCache.h"
#include "CentralCache.h"

// 申请空间
void* ThreadCache::Allocte(size_t size)
{
	size_t index = SizeClass::ListIndex(size);  // 根据开空间的大小计算自由链表的索引下标
	FreeList& freeList = _freeLists[index];
	if (!freeList.Empty())
		return freeList.Pop();
	
	else
		return GetFromCentralCache(SizeClass::RoundUp(size));
}

// 释放空间
void ThreadCache::Deallocte(void* ptr, size_t size)
{
	size_t index = SizeClass::ListIndex(size);  // 根据大小计算索引
	FreeList& freeList = _freeLists[index];
	freeList.Push(ptr);

	// 对象个数超过限制，向中心缓存回收
	size_t num = SizeClass::NumMoveSize(size);
	if (freeList.Num() >= num)
		ListTooLong(freeList, num, size);
}

// 判断链表长度
void ThreadCache::ListTooLong(FreeList& freeList, size_t num, size_t size)
{
	void *start = nullptr, *end = nullptr;
	freeList.PopScope(start, end, num);

	Next(end) = nullptr;
	CentralCache::GetInstance().ReleaseListToSpans(start, size);
}

// 独立测试thread cache
void* ThreadCache::GetFromCentralCache(size_t size)
{
	size_t num = SizeClass::NumMoveSize(size);

	void *start = nullptr, *end = nullptr;
	size_t actualNum = CentralCache::GetInstance().GetRangeObj(start, end, num, size);

	if (actualNum == 1)
		return start;

	else
	{
		size_t index = SizeClass::ListIndex(size);
		FreeList& list = _freeLists[index];
		list.PushScope(Next(start), end, actualNum - 1);

		return start;
	}
}
