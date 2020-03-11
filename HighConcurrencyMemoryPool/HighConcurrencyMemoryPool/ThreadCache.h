// thread cache 本质由一个哈希映射的对象自由链表构成
#pragma once

#include "Universal.h"

class ThreadCache
{
private:
	FreeList _freeLists[FREE_LIST];  // 线程获取内存的自由链表

	//ThreadCache* _next;
	//int threadid;

public:
	void* Allocte(size_t size);              // 申请内存
	void Deallocte(void* ptr, size_t size);  // 释放内存

	void* GetFromCentralCache(size_t index);                        // 从中心缓存获取对象
	void ListTooLong(FreeList& freeList, size_t num, size_t size);  // 如果自由链表中对象数量超过一定长度限制则释放给中心缓存回收
};

// ThreadCache* tclist = nullptr;
// 线程TLS Thread Local Storage 保证效率

_declspec (thread) static ThreadCache* pThreaCache = nullptr;

// 线程局部存储（Thread Local Storage，TLS）用来将数据与一个正在执行的指定线程关联起来。
// 主要是为了避免多个线程同时访存同一全局变量或者静态变量时所导致的冲突，尤其是多个线程同时需要修改这一变量时。
// 通过TLS机制，为每一个使用该全局变量的线程都提供一个变量值的副本，每一个线程均可以独立地改变自己的副本，而不会和其它线程的副本冲突。
// 从线程的角度看，就好像每一个线程都完全拥有该变量。
// 从全局变量的角度上来看，就好像一个全局变量被克隆成了多份副本，而每一份副本都可以被一个线程独立地改变。