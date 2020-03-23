// 包含常用头文件、全局常量、函数等内容
#pragma once

#include <iostream>
#include <assert.h>
#include <map>
#include <unordered_map>
#include <thread>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#endif  // _WIN32环境

using std::endl;
using std::cout;

const size_t MAX_SIZE = 64 * 1024;
const size_t FREE_LIST = MAX_SIZE / 8;
const size_t MAX_PAGES = 129;
const size_t PAGE_SHIFT = 12;  // 4k页移位

// 获取下一个节点指针
inline void*& Next(void* obj)
{
	return *((void**)obj);
}

// 自由链表类
class FreeList
{
private:
	void* _freelist = nullptr;  // 头指针
	size_t _num = 0;  // 总个数

public:
	void Push(void* obj)  // 头插
	{
		Next(obj) = _freelist;
		_freelist = obj;
		++_num;
	}

	void* Pop()  // 头删
	{
		void* obj = _freelist;
		_freelist = Next(obj);
		--_num;
		return obj;
	}

	void PushScope(void* head, void* tail, size_t num)  // 插入一段范围的节点
	{
		Next(tail) = _freelist;
		_freelist = head;
		_num += num;
	}

	size_t PopScope(void*& start, void*& end, size_t num)  // 删除一段范围的节点
	{
		size_t actualNum = 0;  // 实际删除节点数量
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

	size_t Num()  // 获取自由链表长度，即节点数量
	{
		return _num;
	}

	bool Empty()  // 判断自由链表是否为空
	{
		return _freelist == nullptr;
	}

	void Clear()  // 置空自由链表
	{
		_freelist = nullptr;
		_num = 0;
	}
};

// 考虑内存对齐，用于计算具体大小的类
class SizeClass
{
public:
	// 以8进行对齐，计算映射在自由链表的具体位置，计算效率低
	//static size_t ListIndex_No(size_t size)
	//{
	//	if (size % 8 == 0)
	//		return size / 8 - 1;
	//	else
	//		return size / 8;
	//}

	//// 以8进行对齐，计算对齐后的大小，计算效率低
	//static size_t RoundUp_No(size_t size)
	//{
	//	if (size % 8 != 0)
	//		return (size / 8 + 1) * 8;
	//	else
	//		return size;
	//}

	// 控制[1%，10%]左右内碎片浪费
	// [1,128]  8字节对齐  自由链表[0,16)
	// [129,1024]  16字节对齐  自由链表[16,72)
	// [1025,8*1024]  128字节对齐  自由链表[72,128)
	// [8*1024+1,64*1024]  1024字节对齐  自由链表[128,184)

	// 通过相应对齐数和传入大小值计算对齐后实际大小
	static size_t _RoundUp(size_t size, size_t alignment)
	{
		return (size + alignment - 1)&(~(alignment - 1));
	}

	// [9-16] + 7 = [16-23] -> 16 8 4 2 1
	// [17-32] + 15 = [32,47] ->32 16 8 4 2 1
	// 不同区间对齐数不同
	static inline size_t RoundUp(size_t size)
	{
		assert(size <= MAX_SIZE);  // 断言大小是否小于最大值

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
	// 使用对齐数的2的align_shift次幂和传入的大小，计算映射在自由链表的具体位置
	static size_t _ListIndex(size_t size, size_t align_shift)
	{
		return ((size + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	// 不同区间对齐数不同
	static size_t ListIndex(size_t size)
	{
		assert(size <= MAX_SIZE);

		static int group_array[4] = { 16, 56, 56, 56 };  // 每个区间自由链表的数量

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

	// [2,512]个之间，控制获取内存块的数量
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

	// 计算一次向系统申请几个页
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

// span  跨度，管理以页为单位的内存对象，本质是方便合并，解决内存碎片问题
// 2^64 / 2^12 == 2^52
// central cache本质是由一个哈希映射的span对象自由链表构成
// 每个映射大小的空span挂在一个链表中，非空span挂在另一个链表中
// 为了保证全局只有唯一的central cache，这个类设计成单例模式

#ifdef _WIN32
typedef unsigned int PAGE_ID;
#else
typedef unsigned long long PAGE_ID;
#endif // _WIN32环境

// 实现跨度的结构体
struct Span
{
	PAGE_ID _pageid = 0;    // 页号
	PAGE_ID _pagesize = 0;  // 页的数量
	FreeList _freeList;     // 自由链表
	size_t _objSize = 0;    // 自由链表的对象大小
	int _usecount = 0;      // 内存块对象使用计数

	Span* _next = nullptr;
	Span* _prev = nullptr;
};

// span链表类，带头双向循环
class SpanList
{
private:
	Span* _head;      // 头指针
	std::mutex _mutex;  // 互斥锁

public:
	SpanList()  // 构造函数，初始化变量
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	Span* Begin()  // 返回第一个
	{
		return _head->_next;
	}

	Span* End()  // 返回最后一个
	{
		return _head;
	}

	void PushFront(Span* newSpan)  // 头插
	{
		Insert(_head->_next, newSpan);
	}

	void PopFront()  // 头删
	{
		Erase(_head->_next);
	}

	void PushBack(Span* newSpan)  // 尾插
	{
		Insert(_head, newSpan);
	}

	void PopBack()  // 尾删
	{
		Erase(_head->_prev);
	}

	void Insert(Span* pos, Span* newSpan)  // 任意位置插入
	{
		Span* prev = pos->_prev;

		prev->_next = newSpan;
		newSpan->_next = pos;
		pos->_prev = newSpan;
		newSpan->_prev = prev;
	}

	void Erase(Span* pos)  // 任意位置删除
	{
		assert(pos != _head);

		Span* prev = pos->_prev;
		Span* next = pos->_next;

		prev->_next = next;
		next->_prev = prev;
	}

	bool Empty()  // 判空
	{
		return Begin() == End();
	}

	void Lock()  // 加锁
	{
		_mutex.lock();
	}

	void Unlock()  // 解锁
	{
		_mutex.unlock();
	}
};

// 向系统申请内存空间
inline static void* SystemAlloc(size_t num_page)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, num_page*(1 << PAGE_SHIFT), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	// brk mmap等
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
}

// 释放系统内存空间
inline static void SystemFree(void* ptr)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
#endif
}
