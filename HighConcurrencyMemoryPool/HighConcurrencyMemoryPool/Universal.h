// ��������ͷ�ļ���ȫ�ֳ���������������
#pragma once

#include <iostream>
#include <assert.h>
#include <map>
#include <unordered_map>
#include <thread>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#endif  // _WIN32����

using std::endl;
using std::cout;

const size_t MAX_SIZE = 64 * 1024;
const size_t FREE_LIST = MAX_SIZE / 8;
const size_t MAX_PAGES = 129;
const size_t PAGE_SHIFT = 12;  // 4kҳ��λ

// ��ȡ��һ���ڵ�ָ��
inline void*& Next(void* obj)
{
	return *((void**)obj);
}

// ����������
class FreeList
{
private:
	void* _freelist = nullptr;  // ͷָ��
	size_t _num = 0;  // �ܸ���

public:
	void Push(void* obj)  // ͷ��
	{
		Next(obj) = _freelist;
		_freelist = obj;
		++_num;
	}

	void* Pop()  // ͷɾ
	{
		void* obj = _freelist;
		_freelist = Next(obj);
		--_num;
		return obj;
	}

	void PushScope(void* head, void* tail, size_t num)  // ����һ�η�Χ�Ľڵ�
	{
		Next(tail) = _freelist;
		_freelist = head;
		_num += num;
	}

	size_t PopScope(void*& start, void*& end, size_t num)  // ɾ��һ�η�Χ�Ľڵ�
	{
		size_t actualNum = 0;  // ʵ��ɾ���ڵ�����
		void* prev = nullptr;
		void* cur = _freelist;
		for (; actualNum < num && cur != nullptr; actualNum++)
		{
			prev = cur;
			cur = Next(cur);
		}

		start = _freelist;
		end = prev;
		_freelist = cur;

		_num -= actualNum;

		return actualNum;
	}

	size_t Num()  // ��ȡ���������ȣ����ڵ�����
	{
		return _num;
	}

	bool Empty()  // �ж����������Ƿ�Ϊ��
	{
		return _freelist == nullptr;
	}

	void Clear()  // �ÿ���������
	{
		_freelist = nullptr;
		_num = 0;
	}
};

// �����ڴ���룬���ڼ�������С����
class SizeClass
{
public:
	// ��8���ж��룬����ӳ������������ľ���λ�ã�����Ч�ʵ�
	//static size_t ListIndex_No(size_t size)
	//{
	//	if (size % 8 == 0)
	//		return size / 8 - 1;
	//	else
	//		return size / 8;
	//}

	//// ��8���ж��룬��������Ĵ�С������Ч�ʵ�
	//static size_t RoundUp_No(size_t size)
	//{
	//	if (size % 8 != 0)
	//		return (size / 8 + 1) * 8;
	//	else
	//		return size;
	//}

	// ����[1%��10%]��������Ƭ�˷�
	// [1,128]  8�ֽڶ���  ��������[0,16)
	// [129,1024]  16�ֽڶ���  ��������[16,72)
	// [1025,8*1024]  128�ֽڶ���  ��������[72,128)
	// [8*1024+1,64*1024]  1024�ֽڶ���  ��������[128,184)

	// ͨ����Ӧ�������ʹ����Сֵ��������ʵ�ʴ�С
	static size_t _RoundUp(size_t size, size_t alignment)
	{
		return (size + alignment - 1)&(~(alignment - 1));
	}

	// [9-16] + 7 = [16-23] -> 16 8 4 2 1
	// [17-32] + 15 = [32,47] ->32 16 8 4 2 1
	// ��ͬ�����������ͬ
	static inline size_t RoundUp(size_t size)
	{
		assert(size <= MAX_SIZE);  // ���Դ�С�Ƿ�С�����ֵ

		if (size <= 128)
			return _RoundUp(size, 8);
		
		else if (size <= 1024)
			return _RoundUp(size, 16);
		
		else if (size <= 8192)
			return _RoundUp(size, 128);
		
		else if (size <= 65536)
			return _RoundUp(size, 1024);
		
		return -1;
	}

	// [9-16] + 7 = [16-23]
	// ʹ�ö�������2��align_shift���ݺʹ���Ĵ�С������ӳ������������ľ���λ��
	static size_t _ListIndex(size_t size, size_t align_shift)
	{
		return ((size + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	// ��ͬ�����������ͬ
	static size_t ListIndex(size_t size)
	{
		assert(size <= MAX_SIZE);

		static int group_array[4] = { 16, 56, 56, 56 };  // ÿ�������������������

		if (size <= 128)
			return _ListIndex(size, 3);
		
		else if (size <= 1024)
			return _ListIndex(size - 128, 4) + group_array[0];
		
		else if (size <= 8192)
			return _ListIndex(size - 1024, 7) + group_array[1] + group_array[0];
		
		else if (size <= 65536)
			return _ListIndex(size - 8192, 10) + group_array[2] + group_array[1] + group_array[0];

		return -1;
	}

	// [2,512]��֮�䣬���ƻ�ȡ�ڴ�������
	static size_t NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;

		int num = MAX_SIZE / size;
		if (num < 2)
			num = 2;

		if (num > 512)
			num = 512;

		return num;
	}

	// ����һ����ϵͳ���뼸��ҳ
	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);
		size_t nPage = num * size;

		nPage >>= 12;
		if (nPage == 0)
			nPage = 1;

		return nPage;
	}
};

// span  ��ȣ�������ҳΪ��λ���ڴ���󣬱����Ƿ���ϲ�������ڴ���Ƭ����
// 2^64 / 2^12 == 2^52
// central cache��������һ����ϣӳ���span��������������
// ÿ��ӳ���С�Ŀ�span����һ�������У��ǿ�span������һ��������
// Ϊ�˱�֤ȫ��ֻ��Ψһ��central cache���������Ƴɵ���ģʽ

#ifdef _WIN32
typedef unsigned int PAGE_ID;
#else
typedef unsigned long long PAGE_ID;
#endif // _WIN32����

// ʵ�ֿ�ȵĽṹ��
struct Span
{
	PAGE_ID _pageid = 0;    // ҳ��
	PAGE_ID _pagesize = 0;  // ҳ������
	FreeList _freeList;     // ��������
	size_t _objSize = 0;    // ��������Ķ����С
	int _usecount = 0;      // �ڴ�����ʹ�ü���

	Span* _next = nullptr;
	Span* _prev = nullptr;
};

// span�����࣬��ͷ˫��ѭ��
class SpanList
{
private:
	Span* _head;      // ͷָ��
	std::mutex _mutex;  // ������

public:
	SpanList()  // ���캯������ʼ������
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	Span* Begin()  // ���ص�һ��
	{
		return _head->_next;
	}

	Span* End()  // �������һ��
	{
		return _head;
	}

	void PushFront(Span* newSpan)  // ͷ��
	{
		Insert(_head->_next, newSpan);
	}

	void PopFront()  // ͷɾ
	{
		Erase(_head->_next);
	}

	void PushBack(Span* newSpan)  // β��
	{
		Insert(_head, newSpan);
	}

	void PopBack()  // βɾ
	{
		Erase(_head->_prev);
	}

	void Insert(Span* pos, Span* newSpan)  // ����λ�ò���
	{
		Span* prev = pos->_prev;

		prev->_next = newSpan;
		newSpan->_next = pos;
		pos->_prev = newSpan;
		newSpan->_prev = prev;
	}

	void Erase(Span* pos)  // ����λ��ɾ��
	{
		assert(pos != _head);

		Span* prev = pos->_prev;
		Span* next = pos->_next;

		prev->_next = next;
		next->_prev = prev;
	}

	bool Empty()  // �п�
	{
		return Begin() == End();
	}

	void Lock()  // ����
	{
		_mutex.lock();
	}

	void Unlock()  // ����
	{
		_mutex.unlock();
	}
};

// ��ϵͳ�����ڴ�ռ�
inline static void* SystemAlloc(size_t num_page)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, num_page*(1 << PAGE_SHIFT), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	// brk mmap��
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
}

// �ͷ�ϵͳ�ڴ�ռ�
inline static void SystemFree(void* ptr)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
#endif
}
