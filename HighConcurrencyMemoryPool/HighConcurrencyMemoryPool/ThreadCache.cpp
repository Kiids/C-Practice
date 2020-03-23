#include "ThreadCache.h"
#include "CentralCache.h"

// ����ռ�
void* ThreadCache::Allocte(size_t size)
{
	size_t index = SizeClass::ListIndex(size);  // ���ݿ��ռ�Ĵ�С������������������±�
	FreeList& freeList = _freeLists[index];
	if (!freeList.Empty())
		return freeList.Pop();
	
	else
		return GetFromCentralCache(SizeClass::RoundUp(size));
}

// �ͷſռ�
void ThreadCache::Deallocte(void* ptr, size_t size)
{
	size_t index = SizeClass::ListIndex(size);  // ���ݴ�С��������
	FreeList& freeList = _freeLists[index];
	freeList.Push(ptr);

	// ��������������ƣ������Ļ������
	size_t num = SizeClass::NumMoveSize(size);
	if (freeList.Num() >= num)
		ListTooLong(freeList, num, size);
}

// �ж�������
void ThreadCache::ListTooLong(FreeList& freeList, size_t num, size_t size)
{
	void *start = nullptr, *end = nullptr;
	freeList.PopScope(start, end, num);

	Next(end) = nullptr;
	CentralCache::GetInstance().ReleaseListToSpans(start, size);
}

// ��������thread cache
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
