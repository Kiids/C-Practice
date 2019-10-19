#define _CRT_SECURE_NO_WARNINGS 1
#include <cstdlib>
#include <cassert>
#include "MyString.h"
#include "my_string.h"

// 模拟实现string，包含增删改查
namespace mine
{
	class string
	{
	public:
		// 迭代器、const迭代器
		typedef const char* const_iterator;
		typedef char* iterator;

		const_iterator begin() const 
		{
			return _str;
		}

		iterator begin()
		{
			return _str;
		}

		const_iterator end() const
		{
			return _str + _size;
		}

		iterator end()
		{
			return _str + _size;
		}

		// 构造
		string(const char* str = "")
			:_str(new char[strlen(str) + 1])
		{
			// 构造string类对象时，如果传递nullptr指针，则认为程序非法，断言
			if (nullptr == str)
			{
				assert(false);
				return;
			}

			// 已经拷贝'\0'
			strcpy(_str, str);  // while (*dst++ = *src++);
			_size = strlen(str);
			_capacity = _size;
		}

		// 析构
		~string()
		{
			delete[] _str;
			_str = nullptr;
			_size = _capacity = 0;
		}

		// 拷贝构造 string copy1(s1)
		string(const string& s)
			:_str(new char[s._size + 1])
			, _size(s._size)
			, _capacity(s._size)
		{
			strcpy(_str, s._str);
		}

		// 重载=运算符，两种情况  s1 = s2;  s1 = s1;
		string& operator=(const string& s)
		{
			if (this != &s)
			{
				delete[] _str;
				_str = new char[s._size + 1];
				strcpy(_str, s._str);
				_size = s._size;
				_capacity = s._capacity;
			}

			return *this;
		}

		// 按C形式字符串返回
		const char* c_str() const
		{
			return _str;
		}

		// 重载运算符[]
		char& operator[](size_t pos)
		{
			assert(pos < _size);
			return _str[pos];
		}

		const char& operator[](size_t pos) const
		{
			assert(pos < _size);
			return _str[pos];
		}

		size_t size() const
		{
			return _size;
		}

		size_t capacity()
		{
			return _capacity;
		}

		// 扩容reverse
		void reserve(size_t n)
		{
			if (n > _capacity)
			{
				char* tmp = new char[n+1];
				strcpy(tmp, _str);
				delete[] _str;
				_str = tmp;
				_capacity = n;
			}
		} 

		// 增加大小
		void resize(size_t n, char c = char())
		{
			if (n > _size)
			{
				// 如果newSize大于底层空间大小，则需要重新开辟空间
				if (n > _capacity)
				{
					reserve(n);
				}
				memset(_str + _size, c, n - _size);
			}
			_size = n;
			_str[n] = '\0';
		}

		// 尾插单个字符
		void push_back(char ch)
		{
			if (_size == _capacity)  // 扩容
			{
				if (_capacity == 0)
					reserve(8);
				else
					reserve(_capacity * 2);
			}

			_str[_size] = ch;
			++_size;
			_str[_size] = '\0';
			
			//insert(_size, ch);
		}

		// 尾插一个字符串 s1.append("11111");
		void append(const char* str)
		{
			size_t len = strlen(str);
			if (_size+len > _capacity)  // 扩容
			{
				reserve(_size + len);
			}

			strcpy(_str + _size, str);
			_size += len;

			//insert(_size, str);
		}

		// 重载运算符+=  s1 += ch
		const string& operator+=(char ch)
		{
			push_back(ch);
			return *this;
		}

		const string& operator+=(const char* str)
		{
			append(str);
			return *this;
		}

		const string& operator+=(const string& s)
		{
			append(s._str);
			return *this;
		}

		// 插入一个字符
		void insert(size_t pos, char ch)
		{
			assert(pos <= _size);
			if (_size == _capacity)
			{
				if (_capacity == 0)
					reserve(8);
				else
					reserve(_capacity * 2);
			}

			//int end = _size;
			//while (end >= (int)pos)
			//{
			//	_str[end + 1] = _str[end];
			//	--end;
			//}

			size_t end = _size+1;
			while (end >= pos + 1)
			{
				_str[end] = _str[end - 1];
				--end;
			}

			_str[pos] = ch;
			++_size;
		}

		// 插入字符串
		void insert(size_t pos, const char* str)
		{
			assert(pos <= _size);
			size_t len = strlen(str);
			if (_size + len > _capacity)
			{
				reserve(_size + len);
			}

			//int end = _size;
			//while (end >= (int)pos)
			//{
			//	_str[end + len] = _str[end];
			//	--end;
			//}

			size_t end = _size + len;
			while (end >= pos + len)
			{
				_str[end] = _str[end-len];
				--end;
			}

			while (*str)
			{
				_str[pos++] = *str++;
			}

			_size += len;
		}

		// 比较大小
		bool operator>(const string& s) const
		{
			const char* str1 = _str;
			const char* str2 = s._str;
			while (*str1 && *str2)
			{
				if (*str1 > *str2)
				{
					return true;
				}
				else if (*str1 < *str2)
				{
					return false;
				}
				else
				{
					++str1;
					++str2;
				}
			}

			if (*str1)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		bool operator==(const string& s) const
		{
			const char* str1 = _str;
			const char* str2 = s._str;
			while (*str1 && *str2)
			{
				if (*str1 != *str2)
				{
					return false;
				}
				else
				{
					++str1;
					++str2;
				}
			}

			if (*str1 || *str2)
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		bool operator>=(const string& s) const
		{
			return *this > s || *this == s;
		}

		size_t find(char ch)
		{
			for (size_t i = 0; i < _size; i++)
				if (_str[i] == ch)
					return i;

			return string::npos;
		}

		// 查找子串  strstr -> kmp   
		//size_t find(const char* str)
		//{
		//	const char* pos = strstr(_str, str);
		//	if (pos == nullptr)
		//		return npos;
		//	else
		//		return pos - _str;
		//}

		size_t find(const char* str) 
		{
			const char* src = _str;
			const char* dst = str;
			size_t srclen = _size;
			size_t dstlen = strlen(dst);

			size_t srcindex = 0;
			while (srcindex < srclen)
			{
				size_t i = srcindex;
				size_t j = 0;
				while (j < dstlen && src[i] == dst[j])
				{
					++i;
					++j;
				}

				if (j == dstlen)
				{
					return srcindex;
				}
				
			    srcindex++;
			}

			return npos;
		}

		// getline

	private:
		char* _str;
		size_t _size;
		size_t _capacity;

		static size_t npos;
	};

	size_t string::npos = -1;

	// 复用+=重载运算符+
	string operator+(const string& s1, const string& s2)
	{
		string ret = s1;
		ret += s2;
		return ret;
	}

	// 输出运算符重载
	ostream& operator<<(ostream& out, const string& s)
	{
		// out << s.c_str();  这样不可以，遇到\0会停止输出
		for (size_t i = 0; i < s.size(); i++)
		{
			out << s[i];
		}
		return out;
	}

	// 输入运算符重载
	istream& operator>>(istream& in, string& s)
	{
		char ch = NULL;
		while (ch != ' ' && ch != '\n')
		{
			ch = in.get();
			s += ch;
		}
		return in;
	}

}

void test1()
{
	mine::string s1("Hello");
	cout << s1.c_str() << endl;
	mine::string copy(s1);
	cout << copy.c_str() << endl;

	for (size_t i = 0; i < s1.size(); ++i)
	{
		// s1.operator[](i); -> s1.operator[](&s1, i);
		s1[i] = 'a';
		cout << s1[i] << " ";
	}
	cout << endl;

	mine::string::iterator it1 = s1.begin();
	while (it1 != s1.end())
	{
		cout << *it1 << " ";
		++it1;
	}
	cout << endl;

	for (auto e : s1)
	{
		cout << e << " ";
	}
	cout << endl;
}

void test2()
{
	mine::string s1("Hello");
	cout << s1.capacity() << endl;

	s1 += "World";
	cout << s1.capacity() << endl;

	s1 += '!';
	cout << s1.capacity() << endl;

	cout << s1.c_str() << endl;

	mine::string s2("HelloWorld!");
	s2.insert(5, ' ');
	cout << s2.c_str() << endl;
}

void test3()
{
	mine::string s1("Hello World worxxxy");
	cout << s1.find("wor") << endl;
	cout << s1.find("worx") << endl;
}

//#include <string>
//void test4()
//{
//	std::string s;
//	cin >> s;
//	cout << s << endl;
//}

void test5()
{
	mine::string s;
	cin >> s;
	cout << s << endl;
}

int main()
{
	//Test1();
	//Test2();

	//test1();
	//test2();
	//test3();

	//test4();
	test5();

	system("pause");
	return 0;
}