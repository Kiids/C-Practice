#pragma once
#include <iostream>

namespace mine
{
	// ����ڵ�
	template<class T>
	struct __list_node
	{
		__list_node<T>* _next;  // ָ��ǰ��
		__list_node<T>* _prev;  // ָ����
		T _data;

		__list_node(const T& x = T())
			: _data(x)
			, _next(nullptr)
			, _prev(nullptr)
		{

		}
	};

	// ������
	template<class T, class Ref, class Ptr>
	struct __list_iterator
	{
		typedef __list_node<T> node;
		typedef __list_iterator<T, Ref, Ptr> Self;
		node* _node;

		__list_iterator(node* node)
			: _node(node)
		{

		}

		// ��������
		Ref operator*()
		{
			return _node->_data;
		}

		// �Զ�������
		Ptr operator->()
		{
			return &_node->_data; // it->data; --> it.opterator->() _data(�Զ��������ڵı���) ʵ��ӦΪit->->_data,�˴����������⴦��ʡ�Ե�һ��->
		}

		// ǰ��++
		Self& operator++()
		{
			_node = _node->_next;
			return *this;
		}

		// ����++
		Self operator++(int)
		{
			__list_iterator<T> tmp(*this);
			_node = _node->_next;
			return tmp;
		}

		// ǰ��--
		Self& operator++()
		{
			_node = _node->_prev;
			return *this;
		}

		// ����--
		Self operator++(int)
		{
			__list_iterator<T> tmp(*this);
			_node = _node->_prev;
			return tmp;
		}

		bool operator!= (const Self& it)
		{
			return _node != it._node;
		}

		bool operator== (const Self& it)
		{
			return _node == it._node;
		}
	};

	template<class T>
	class list
	{
		typedef __list_node<T> node;
	public:
		typedef __list_iterator<T, T&, T*> iterator;
		typedef __list_iterator<T, const T&, const T*> const_iterator;

		const_iterator begin() const
		{
			return const_iterator(_head->_next);
		}

		const_iterator end() const
		{
			return const_iterator(_head);
		}

		iterator begin()
		{
			return iterator(_head->_next);
		}

		iterator end()
		{
			return iterator(_head);
		}

		// �������� copy(l)
		list(const list<T>& l)
		{
			_head = new node;
			_head->next = _head;
			_head->prev = _head;

			const_iterator it = l.begin();
			//auto it = l.begin();
			while (it != l.end())
			{
				push_back(*it);
				++it;
			}
		}

		// l1 = l2 ��ͳд��
		//list<T>& operator=(const list<T>& l)
		//{
		//	this->clear();

		//	auto it = l.begin();
		//	while (it != l.end())
		//	{
		//		push_back(*it);
		//		++it;
		//	}
		//}

		// l1 = l2 �ִ�д��
		list<T>& operator=(list<T> l)
		{
			swap(_head, l.head);
			return *this;
		}

		list()
		{
			_head = new node(T());
			_head->next = _head;
			_head->prev = _head;
		}

		~list()
		{
			clear();

			delete _head;
			_head = nullptr;
		}

		void clear()
		{
			iterator it = begin();
			while (it != end())
				it = erase(it);
		}

		void push_back(const T& x)
		{
			//node* newnode = new node(x);

			//_head->_prev->next = newnode;
			//newnode->_prev = _head->_prev;
			//newnode->_next = _head;
			//_head->_prev = newnode;

			insert(end(), x);
		}

		void push_front(const T& x)
		{
			insert(begin(), x);
		}

		void pop_back()
		{
			erase(--end());
		}

		void pop_front()
		{
			erase(begin());
		}

		void insert(iterator pos, const T& x)
		{
			node* cur = pos._node;
			node* newnode = new node(x);

			cur->_prev->next = newnode;
			newnode->_prev = cur->_prev;
			newnode->_next = cur;
			cur->_prev = newnode;
		}

		// ����ɾ���ڵ����һ���ڵ������ڵ������ı���
		iterator erase(iterator pos)
		{
			node* cur = pos._node;
			assert(cur != _head);  // ��ֹɾ��ͷ�ڵ�

			cur->_prev->_next = cur->_next;
			cur->_next->_prev = cur->_prev;

			delete cur;
			reuurn iterator(next);
		}

	private:
		node* _head;
	};

	void print(const list<int>& l)
	{
		list<int>::const_iterator it = l.begin();
		while (it != l.end())
		{
			//*it = 10;  ->  it.operator*() = 10;
			std::cout << *it << " ";
			++it;
		}
		std::cout << std::endl;
	}


	void test_list1()
	{
		list<int> l;
		l.push_back(1);
		l.push_back(2);
		l.push_back(3);
		l.push_back(4);
		print(l);

		list<int>::iterator it = l.begin();
		while (it != l.end())
		{
			if (*it % 2 == 0)
			{
				it = l.erase(it);
			}
			else
			{
				++it;
			}
		}

		print(l);
	}

	struct Date
	{
		int _year = 1900;
		int _month = 1;
		int _day = 1;
	};

	void test_list2()
	{
		list<Date> l;
		l.push_back(Date());
		l.push_back(Date());

		//list<Date>::iterator it = l.begin();
		auto it = l.begin();
		while (it != l.end())
		{
			//cout << *it << endl;
			std::cout << it->_year << "-" << it->_month << "-" << it->_day << std::endl;
			++it;
		}

		//int* p *p
		//Date* p p->_year;
	}

	void test_list3()
	{
		list<int> l;
		l.push_back(1);
		l.push_back(2);
		l.push_back(3);
		l.push_back(4);
		//print_list(l);

		list<int> copy = l;
		l.push_back(5);

		print(copy);
		print(l);
	}
}
