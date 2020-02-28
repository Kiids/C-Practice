#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <boost/filesystem.hpp>

using std::cout;
using std::endl;

#ifdef _WIN32
// Windows��ͷ�ļ�
#include <WS2tcpip.h>
#include <IPHlpApi.h>  // ��ȡ������Ϣ�ӿ�
#pragma comment(lib, "Iphlpapi.lib")  // ��ȡ������Ϣ�ӿڵĿ��ļ���������ʾ���������
#pragma comment(lib, "ws2_32.lib")  // Windows�µ�socket��
#else
// Linux��ͷ�ļ�
#endif

#define DOWNLOAD_FOLDER "./DownloadFile"  // ����·���������ļ���

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

	static bool Write(const std::string& name, const std::string& body, int64_t offset = 0)  // nameд����ļ�����body���ģ�д������ݣ�offsetƫ������д���λ��
	{
		cout << "д����ļ�����" << name << "д�����ݴ�С�����ݣ�" << body.size() << " : [ " << body << " ]" << endl;

		if (!boost::filesystem::exists(DOWNLOAD_FOLDER))  // �������ļ��в������򴴽�һ��
			boost::filesystem::create_directory(DOWNLOAD_FOLDER);

		std::ofstream of;
		if (!boost::filesystem::exists(name))  // �ж����ص��ļ��Ƿ���ڣ������ļ�ֱ�����غͷֿ鴫���һ����Ҫ�����ļ����ֿ鴫�����һ��֮�����ƫ��������д��
		{
			std::ofstream o(name);
			if (o)
				cout << "�½��ļ��ɹ����ļ�����" << name << endl;
		}

		// ios::in ������/����ʽ���ļ�
		// ios::out �����/д��ʽ���ļ�
		// ios::ate �ļ��򿪺�λ���ļ�β
		// ios::app ׷�ӵķ�ʽ�򿪣�׷��д
		// ios::trunc ����ļ��Ѵ�������ɾ���ļ�
		// ios::binary �����Ʒ�ʽ��
		of.open(name, std::ios::in | std::ios::out | std::ios::binary);
		if (of.is_open() == false)
		{
			cout << "���ļ�ʧ�ܣ��ļ�����" << name << endl;
			return false;
		}

		of.seekp(offset, std::ios::beg);  // ��дλ����ת��������ļ���ʼλ�ÿ�ʼƫ����offset
		of.write(&body[0], body.size());
		if (of.good() == false)  // �ж���һ���ļ������Ƿ�ɹ�
		{
			cout << "���ļ�д������ʧ�ܡ�" << endl;
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
			cout << "���ļ�ʧ�ܣ��ļ�����" << name << endl;
			return false;
		}

		f.read(&(*body)[0], size);
		if (f.good() == false)
		{
			cout << "��ȡ�ļ�����ʧ�ܣ��������ݣ�" << *body << endl;
			f.close();
			return false;
		}
		f.close();
		return true;
	}

	static bool ReadInterval(const std::string& name, std::string* body, int64_t length, int64_t offset)  // ��ȡ���䣬�ֿ鴫��
	{
		body->resize(length);
		std::fstream f(name, std::ios::in | std::ios::out | std::ios::binary);
		if (f.is_open() == false)
		{
			cout << "���ļ�ʧ�ܣ��ļ�����" << name << endl;
			return false;
		}

		f.seekg(offset, std::ios::beg);
		f.read(&(*body)[0], length);

		if (f.good() == false)
		{
			cout << "��ȡ�ļ�����ʧ�ܣ��������ݣ�" << *body << endl;
			f.close();
			return false;
		}
		f.close();
		return true;
	}
};

class Adapter  // ����������������
{
public:
	uint32_t _ip_address;  // ������IP��ַ
	uint32_t _mask_address;  // ��������������
};
class AdapterClass
{
public:
#ifdef _WIN32
	// Windows�»�ȡ������Ϣ
	static bool GetAllAdapter(std::vector<Adapter>* list)
	{
		// �ṹ��IP_ADAPTER_INFO�������ؼ����ĳһ����������������Ϣ����ֻ�ܻ�ȡIPv4��Ϣ
		PIP_ADAPTER_INFO pAdapters = new IP_ADAPTER_INFO();
		uint64_t allAdapterSize = sizeof(IP_ADAPTER_INFO);
		// GetAdaptersInfo Windows�º�ȥ������Ϣ�Ľӿ�
		// �����ж��������Ϣ���ʴ���ָ��
		// ���ڿռ䲻���ԭ�򣬻�ȡ������Ϣ���ܻ�ʧ�ܣ������allAdapterSize������������������Ϣʵ�ʿռ�ռ�ô�С
		// ULONG  unsigned long
		// PULONG��ָ�룬 ����Ϊָ��ULONG���͵�ָ����������PULONG�������ָ��ULONG�����͵�ָ��
		int ret = GetAdaptersInfo(pAdapters, (PULONG)&allAdapterSize);
		if (ret == ERROR_BUFFER_OVERFLOW) // �������ռ䲻��
		{
			// ��������ָ��ռ�
			delete pAdapters;
			// BYTE  unsigned char
			pAdapters = (PIP_ADAPTER_INFO)new BYTE[allAdapterSize];
			GetAdaptersInfo(pAdapters, (PULONG)&allAdapterSize);  // ���»�ȡ������Ϣ
		}

		while (pAdapters)
		{
			Adapter adapter;
			// inet_pton(int family, char *string, void *buf)  ��һ���ַ������ʮ����IP��ַת��Ϊ�����ֽ���IP��ַ
			// AF_INET ʹ��IPv4
			inet_pton(AF_INET, pAdapters->IpAddressList.IpAddress.String, &adapter._ip_address);
			inet_pton(AF_INET, pAdapters->IpAddressList.IpMask.String, &adapter._mask_address);
			if (adapter._ip_address != 0)  // ��ȥδ���õ�������û������������IP��ַ��Ϊ0
			{
				list->push_back(adapter);  // ��������Ϣ�����vector���ظ��û�
				cout << "�������ƣ�" << pAdapters->AdapterName << endl;
				cout << "����������" << pAdapters->Description << endl;
				cout << "IP��ַ��" << pAdapters->IpAddressList.IpAddress.String << endl;
				cout << "�������룺" << pAdapters->IpAddressList.IpAddress.String << endl;
				cout << endl;
			}
			pAdapters = pAdapters->Next;
		}
		delete pAdapters;
		return true;
	}
#else
	// Linux�»�ȡ������Ϣ
	static bool GetAllAdapter(std::vector<Adapter>* list)
#endif
};