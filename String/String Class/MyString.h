#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
using namespace std;

// 实现一个简单的string
// 传统写法
// 深浅拷贝
namespace my
{
	class string
	{
	public:
		string(const char* str = "")        // ""是一个空字符串，含有一个'\0'
			:_str(new char[strlen(str)+1])
		{
			strcpy(_str, str);              // 字符串复制，while (*dst++ = *src++); 拷贝了'\0'
		}

		~string()
		{
			delete[] _str;
			_str = nullptr;
		}

		string(const string& s)             // 拷贝构造函数的深拷贝
			:_str(new char[strlen(s._str)+1])
		{
			strcpy(_str, s._str);
		}

		string& operator=(const string& s)  // 重载=运算符的深拷贝 s1 = s2; s1 = s1;
		{
			if (this != &s)
			{
				delete[] _str;
				_str = new char[strlen(s._str) + 1];
				strcpy(_str, s._str);
			}

			return *this;
		}

		const char* c_str()
		{
			return _str;
		}

		char& operator[](size_t pos)
		{
			return _str[pos];
		}

	private:
		char* _str;
	};
}

void Test1()
{
	my::string s1("hello");
	cout << s1.c_str() << endl;

	s1[0] = 'a';
	cout << s1.c_str() << endl;

	my::string copy1(s1);
	cout << copy1.c_str() << endl;

	copy1[0] = 'h';
	cout << s1.c_str() << endl;
	cout << copy1.c_str() << endl;

	my::string s2 = "world";
	s1 = s2;
	cout << s1.c_str() << endl;
}
