#define _CRT_SECURE_NO_WARNINGS 1
#include <cstdlib>
#include <cassert>
#include "MyString.h"
#include "my_string.h"
//using std::setw;

// 模拟实现string，包含增删改查
namespace mine
{
	class string
	{
	public:
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

		string(const char* str = "")
			:_str(new char[strlen(str) + 1])
		{
			// 构造string类对象时，如果传递nullptr指针，认为程序非法，此处断言下
			if (nullptr == str)
			{
				assert(false);
				return;
			}

			// 已经拷贝'\0'
			strcpy(_str, str);// while (*dst++ = *src++);
			_size = strlen(str);
			_capacity = _size;
		}

		~string()
		{
			delete[] _str;
			_str = nullptr;
			_size = _capacity = 0;
		}

		//string copy1(s1)
		string(const string& s)
			:_str(new char[s._size + 1])
			, _size(s._size)
			, _capacity(s._size)
		{
			strcpy(_str, s._str);
		}

		// s1 = s2;
		// s1 = s1;
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

		const char* c_str() const
		{
			return _str;
		}

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

		//reverse
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

		void push_back(char ch)
		{
			//if (_size == _capacity)
			//{
			//	// 扩容
			//	reserve(_capacity * 2);
			//}

			//_str[_size] = ch;
			//++_size;
			//_str[_size] = '\0';
			insert(_size, ch);
		}

		// s1.append("11111");
		void append(const char* str)
		{
			//size_t len = strlen(str);
			//if (_size+len > _capacity)
			//{
			//	// 扩容
			//	reserve(_size + len);
			//}

			//strcpy(_str + _size, str);
			//_size += len;

			insert(_size, str);
		}

		//s1 += ch
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

		void insert(size_t pos, char ch)
		{
			assert(pos <= _size);
			if (_size == _capacity)
			{
				reserve(_capacity * 2);
			}

			/*int end = _size;
			while (end >= (int)pos)
			{
				_str[end + 1] = _str[end];
				--end;
			}*/

			size_t end = _size+1;
			while (end >= pos + 1)
			{
				_str[end] = _str[end - 1];
				--end;
			}

			_str[pos] = ch;
			++_size;
		}

		void insert(size_t pos, const char* str)
		{
			assert(pos <= _size);
			size_t len = strlen(str);
			if (_size + len > _capacity)
			{
				reserve(_size + len);
			}

			/*	int end = _size;
				while (end >= (int)pos)
				{
				_str[end + len] = _str[end];
				--end;
				}*/

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

		// s1 > s2
		// hello  hello!
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

			/*
			else if (*str2)
			{
			return false;
			}
			else
			{
			return false;
			}
			*/
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

		//size_t find(const char* str) // strstr -> kmp   
		//{
		//	const char* pos = strstr(_str, str);
		//	if (pos == nullptr)
		//		return npos;
		//	else
		//		return pos - _str;
		//}

		size_t find(const char* str) // strstr -> kmp   
		{
			const char* src = _str;
			const char* dst = str;
			size_t srclen = _size;
			size_t dstlen = strlen(dst);

			size_t srcindex = 0;
			while (srcindex < srclen)
			{
				/*if (src[srcindex] == dst[0])
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
					else
					{
						++srcindex;
					}
				}
				else
				{
					srcindex++;
				}*/

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

	// s1 + s2
	string operator+(const string& s1, const string& s2)
	{
		string ret = s1;
		ret += s2;
		return ret;
	}

	ostream& operator<<(ostream& out, const string& s)
	{
		// out << s.c_str();  这样不可以，遇到\0会停止输出
		for (size_t i = 0; i < s.size(); i++)
		{
			out << s[i];
		}

		return out;
	}

   // 以下输入运算符重载还需要调试
	istream& operator>>(istream& in, string& s)
	{
		//char ch;
		//while (in.get(ch))
		//{
		//	if (ch == ' ' || ch == '\n')
		//	{
		//		in.clear();

		//		return in;
		//	}
		//	else
		//		s += ch;
		//}

		//return in;

//		char temp[100];
//		in >> setw(100) >> temp; 
		/*
		见
		在C++中，setw(int n)用来控制输出间隔。
		例如:
		cout<<'s'<<setw(8)<<'a'<<endl;
		则在屏幕显示
		s a
		s与a之间有7个空格，setw()只对其后面紧跟的输出产生作用，如上例中，表示'a'共占8个位置，不足的用空格填充。若输入的内容超过setw()设置的长度，则按实际长度输出。
		setw()默认填充的内容为空格，可以setfill()配合使用设置其他字符填充。
		*/
//		s = temp;
//		return in;

		char ch;
		while (scanf("[^\n]", &ch))
		{
			if (ch == ' ' || ch == '\n')
			{
				in.clear();

				return in;
			}
			else
				s += ch;
		}

		return in;
	}

}



void test1()
{
	mine::string s1("hello");
	cout << s1.c_str() << endl;
	mine::string copy1(s1);
	cout << copy1.c_str() << endl;

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
	mine::string s1("hello");
	cout << s1.capacity() << endl;

	s1 += "world";
	cout << s1.capacity() << endl;

	s1 += '!';
	cout << s1.capacity() << endl;

	cout << s1.c_str() << endl;

	mine::string s2("helloworld!");
	s2.insert(5, ' ');
	cout << s2.c_str() << endl;
	s2.insert(0, '$');
	cout << s2.c_str() << endl;
	s2.insert(0, "bit");
	cout << s2.c_str() << endl;
}

void test3()
{
	mine::string s1("hello world worxxxy");
	cout << s1.find("wor") << endl;
	cout << s1.find("worx") << endl;
}

int main()
{
	Test1();
	Test2();

	system("pause");
	return 0;
}