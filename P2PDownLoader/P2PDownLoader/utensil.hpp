#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <boost/filesystem.hpp>

using std::cout;
using std::endl;

#ifdef _WIN32
// Windows的头文件
#include <WS2tcpip.h>
#include <IPHlpApi.h>  // 获取网卡信息接口
#pragma comment(lib, "Iphlpapi.lib")  // 获取网卡信息接口的库文件包含，表示链接这个库
#pragma comment(lib, "ws2_32.lib")  // Windows下的socket库
#else
// Linux的头文件
#endif

#define DOWNLOAD_FOLDER "./DownloadFile"  // 下载路径，下载文件夹

class StringClass
{
public:
	static int64_t AtoI(const std::string &n)
	{
		std::stringstream t;
		int64_t ret;
		t << n;
		t >> ret;
		return ret;
	}
};

class FileClass
{
public:
	static int64_t GetFileSize(const std::string& name)
	{
		return boost::filesystem::file_size(name);
	}

	static bool Write(const std::string& name, const std::string& body, int64_t offset = 0)  // name写入的文件名；body正文，写入的数据；offset偏移量，写入的位置
	{
		cout << "写入的文件名：" << name << "写入数据大小及内容：" << body.size() << " : [ " << body << " ]" << endl;

		if (!boost::filesystem::exists(DOWNLOAD_FOLDER))  // 若下载文件夹不存在则创建一个
			boost::filesystem::create_directory(DOWNLOAD_FOLDER);

		std::ofstream of;
		if (!boost::filesystem::exists(name))  // 判断下载的文件是否存在，整个文件直接下载和分块传输第一块需要创建文件，分块传输除第一块之后根据偏移量附加写入
		{
			std::ofstream o(name);
			if (o)
				cout << "新建文件成功，文件名：" << name << endl;
		}

		// ios::in 以输入/读方式打开文件
		// ios::out 以输出/写方式打开文件
		// ios::ate 文件打开后定位到文件尾
		// ios::app 追加的方式打开，追加写
		// ios::trunc 如果文件已存在则先删除文件
		// ios::binary 二进制方式打开
		of.open(name, std::ios::in | std::ios::out | std::ios::binary);
		if (of.is_open() == false)
		{
			cout << "打开文件失败，文件名：" << name << endl;
			return false;
		}

		of.seekp(offset, std::ios::beg);  // 读写位置跳转至相对于文件起始位置开始偏移量offset
		of.write(&body[0], body.size());
		if (of.good() == false)  // 判断上一步文件操作是否成功
		{
			cout << "向文件写入数据失败。" << endl;
			of.close();
			return false;
		}
		of.close();
		return true;
	}

	static bool Read(const std::string& name, std::string* body)
	{
		int64_t size = GetFileSize(name);
		body->resize(size);

		std::fstream f(name, std::ios::in | std::ios::out | std::ios::binary);
		if (f.is_open() == false)
		{
			cout << "打开文件失败，文件名：" << name << endl;
			return false;
		}

		f.read(&(*body)[0], size);
		if (f.good() == false)
		{
			cout << "读取文件数据失败，正文内容：" << *body << endl;
			f.close();
			return false;
		}
		f.close();
		return true;
	}

	static bool ReadInterval(const std::string& name, std::string* body, int64_t length, int64_t offset)  // 读取区间，分块传输
	{
		body->resize(length);
		std::fstream f(name, std::ios::in | std::ios::out | std::ios::binary);
		if (f.is_open() == false)
		{
			cout << "打开文件失败，文件名：" << name << endl;
			return false;
		}

		f.seekg(offset, std::ios::beg);
		f.read(&(*body)[0], length);

		if (f.good() == false)
		{
			cout << "读取文件数据失败，正文内容：" << *body << endl;
			f.close();
			return false;
		}
		f.close();
		return true;
	}
};

class Adapter  // 适配器，网络适配
{
public:
	uint32_t _ip_address;  // 网卡上IP地址
	uint32_t _mask_address;  // 网卡上子网掩码
};
class AdapterClass
{
public:
#ifdef _WIN32
	// Windows下获取网卡信息
	static bool GetAllAdapter(std::vector<Adapter>* list)
	{
		// 结构体IP_ADAPTER_INFO包含本地计算机某一个网络适配器的信息，但只能获取IPv4信息
		PIP_ADAPTER_INFO pAdapters = new IP_ADAPTER_INFO();
		uint64_t allAdapterSize = sizeof(IP_ADAPTER_INFO);
		// GetAdaptersInfo Windows下后去网卡信息的接口
		// 可能有多个网卡信息，故传入指针
		// 由于空间不足等原因，获取网卡信息可能会失败，输出型allAdapterSize参数返回所有网卡信息实际空间占用大小
		// ULONG  unsigned long
		// PULONG是指针， 它作为指向ULONG类型的指针的替代，即PULONG代表的是指向ULONG的类型的指针
		int ret = GetAdaptersInfo(pAdapters, (PULONG)&allAdapterSize);
		if (ret == ERROR_BUFFER_OVERFLOW) // 缓冲区空间不足
		{
			// 重新申请指针空间
			delete pAdapters;
			// BYTE  unsigned char
			pAdapters = (PIP_ADAPTER_INFO)new BYTE[allAdapterSize];
			GetAdaptersInfo(pAdapters, (PULONG)&allAdapterSize);  // 重新获取网卡信息
		}

		while (pAdapters)
		{
			Adapter adapter;
			// inet_pton(int family, char *string, void *buf)  将一个字符串点分十进制IP地址转换为网络字节序IP地址
			// AF_INET 使用IPv4
			inet_pton(AF_INET, pAdapters->IpAddressList.IpAddress.String, &adapter._ip_address);
			inet_pton(AF_INET, pAdapters->IpAddressList.IpMask.String, &adapter._mask_address);
			if (adapter._ip_address != 0)  // 出去未启用的网卡，没有启用网卡的IP地址即为0
			{
				list->push_back(adapter);  // 将网卡信息添加至vector返回给用户
				cout << "网卡名称：" << pAdapters->AdapterName << endl;
				cout << "网卡描述：" << pAdapters->Description << endl;
				cout << "IP地址：" << pAdapters->IpAddressList.IpAddress.String << endl;
				cout << "子网掩码：" << pAdapters->IpAddressList.IpAddress.String << endl;
				cout << endl;
			}
			pAdapters = pAdapters->Next;
		}
		delete pAdapters;
		return true;
	}
#else
	// Linux下获取网卡信息
	static bool GetAllAdapter(std::vector<Adapter>* list)
#endif
};