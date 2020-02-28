#pragma once
#include <thread>
#include <boost/filesystem.hpp>
#include "utensil.hpp"
#include "httplib.h"

#define P2P_PORT 9000
#define MAX_IP_BUFFER 16  // 255.255.255.255 + \0�ַ���������־������16�ֽ�
//#define MAX_INTERVAL (100 * 1024 * 1024)
#define MAX_INTERVAL (5)  // �������
#define SHARED_FOLDER "./SharedFile/"
#define DOWNLOAD_FOLDER "./DownloadFile/"

class Host  // ����
{
public:
	uint32_t _ip_address;  // ���������IP��ַ
	bool _pair;  // �����Խ�����ɹ�true��ʧ��false
};

class Client
{
public:
	bool Start()
	{
		// �ͻ���ѭ�����У��Ա�������
		while (1)
		{
			OnlineHost();
		}
		return true;
	}

	// ������Ե��߳���ں���
	void HostPair(Host* host)
	{
		host->_pair = false;
		char buf[MAX_IP_BUFFER] = { 0 };
		inet_ntop(AF_INET, &host->_ip_address, buf, MAX_IP_BUFFER);
		httplib::Client c(buf, P2P_PORT);  // ʵ����һ��httplib�ͻ��˶���
		auto response = c.Get("/hostpair");  // �����˷�������Ϊ/hostpair��GET����������ʧ��Get�᷵��NULL
		if (response && response->status == 200)  // �ж���Ӧ�����ȷ
			host->_pair = true;  // ����������Խ��
		return;
	}

	// ��ȡ��������
	bool OnlineHost()
	{
		char c = 'Y';
		if (!_online_host.empty())
		{
			cout << "�Ƿ����²鿴����������Y/N����";
			fflush(stdout);
			std::cin >> c;
		}
		if (c == 'Y')
		{
			cout << "����ƥ����..." << endl;
			std::vector<Adapter> v;
			AdapterClass::GetAllAdapter(&v);  // ��ȡ����������Ϣ��ŵ�v
			std::vector<Host> vHost;  // �������������IP��ַ
			for (int i = 0; i < v.size(); i++)  // ��ȡ����������IP��ַ�б�
			{
				uint32_t ip = v[i]._ip_address;
				uint32_t mask = v[i]._mask_address;
				uint32_t net = (ntohl(ip & mask));  // ��������ţ�ntol���޷��ų��������������ֽ���ת��Ϊ�����ֽ���
				uint32_t maxHost = (~ntohl(mask));  // �������������
				for (int j = 1; j < (int32_t)maxHost; j++)
				{
					uint32_t hostIP = net + j;  // �������IP�ļ���Ӧʹ�������ֽ���С���ֽ��򣩵������
					Host host;
					host._ip_address = htonl(hostIP);  // ����������ֽ���IP��ַת��Ϊ�����ֽ���
					host._pair = false;
					vHost.push_back(host);
				}
			}
			// ���vHost�е����������߳̽������
			std::vector<std::thread*> threadList(vHost.size());
			for (int i = 0; i < vHost.size(); i++)
				threadList[i] = new std::thread(&Client::HostPair, this, &vHost[i]);
			cout << "�������������..." << endl;
			// �ȴ������߳������ɣ��жϽ���������������_online_host
			for (int i = 0; i < vHost.size(); i++)
			{
				threadList[i]->join();  // �ȴ�һ���߳��˳�
				if (vHost[i]._pair == true)
					_online_host.push_back(vHost[i]);
				delete threadList[i];
			}
		}
		// ��ӡ������������IP���ṩ���û�ѡ��
		for (int i = 0; i < _online_host.size(); i++)
		{
			char buf[MAX_IP_BUFFER] = { 0 };
			inet_ntop(AF_INET, &_online_host[i]._ip_address, buf, MAX_IP_BUFFER);
			cout << "->    " << buf << endl;
		}
		cout << "��ѡ����������ȡ�����ļ��б�";
		fflush(stdout);
		std::string s;
		std::cin >> s;
		ShareList(s);
		return true;
	}

	// ��ȡ�����ļ��б�
	bool ShareList(const std::string& hostIP)
	{
		if (!boost::filesystem::exists(SHARED_FOLDER))  // �������ļ��в����ڣ��򴴽�һ��
			boost::filesystem::create_directory(SHARED_FOLDER);

		httplib::Client c(hostIP.c_str(), P2P_PORT);
		auto response = c.Get("/list");
		if (response == NULL || response->status != 200)
		{
			cout << "��ȡ�����ļ��б���Ӧ����" << endl;
			return false;
		}

		cout << response->body << endl;
		cout << "��ѡ����Ҫ���ص��ļ���";
		fflush(stdout);
		std::string name;
		std::cin >> name;
		IntervalDownLoad(hostIP, name);
		return true;
	}

	// �����ļ���С�ļ�ֱ������
	bool AllDownLoad(const std::string& hostIP, const std::string& name)
	{
		std::string requestPath = "/downloadfile/" + name;
		httplib::Client c(hostIP.c_str(), P2P_PORT);
		cout << "�����˷����ļ���������" << hostIP << ":" << requestPath << endl;
		auto response = c.Get(requestPath.c_str());
		if (response == NULL || response->status != 200)
		{
			cout << "�����ļ���ȡ��Ӧ��Ϣʧ�ܡ�" << endl;
			return false;
		}
		cout << "��ȡ�ļ�������Ӧ�ɹ���" << endl;

		if (!boost::filesystem::exists(DOWNLOAD_FOLDER))  // �������ļ��в������򴴽�һ��
			boost::filesystem::create_directory(DOWNLOAD_FOLDER);

		std::string realPath = DOWNLOAD_FOLDER + name;
		if (FileClass::Write(realPath, response->body) == false)
		{
			cout << "�ļ�����ʧ�ܡ�" << endl;
			return false;
		}
		cout << "�ļ����سɹ���" << endl;
		return true;
	}

	// �ֿ�����
	bool IntervalDownLoad(const std::string& hostIP, const std::string& name)
	{
		std::string requestPath = "/downloadfile/" + name;
		int64_t size = GetSize(hostIP, requestPath);
		if (size < MAX_INTERVAL)
		{
			cout << "�ļ���С��ֱ�����ء�" << endl;
			return AllDownLoad(hostIP, name);
		}
		cout << "�ļ����󣬷ֿ����ء�" << endl;

		// �ֿ�����ļ���
		int64_t count = 0;
		// �ļ���С���������С���ֿ���� = �ļ���С / �ֿ��С
		// �ļ���С�����������С���ֿ���� = �ļ���С / �ֿ��С + 1
		if ((size % MAX_INTERVAL) == 0)
			count = size / MAX_INTERVAL;
		else
			count = size / MAX_INTERVAL + 1;

		int64_t begin = 0, end = 0;
		for (int i = 0; i < count; i++)
		{
			begin = i * MAX_INTERVAL;
			if (i == (count - 1))  // ���һ���ֿ�
				end = size - 1;
			else
				end = ((i + 1) * MAX_INTERVAL) - 1;
			cout << "�ͻ�������ֿ飺" << begin << "-" << end << endl;
			_IntervalDownLoad(hostIP, name, begin, end);
		}
		cout << "�ļ��ֿ����سɹ���" << endl;
		return true;
	}
	
	int64_t GetSize(const std::string& hostIP, const std::string& requestPath)
	{
		httplib::Client c(hostIP.c_str(), P2P_PORT);
		auto response = c.Head(requestPath.c_str());
		if (response == NULL || response->status != 200)
		{
			cout << "��ȡ�ļ���Сʧ�ܡ�" << endl;
			if (response)
				cout << "��Ӧ״̬�룺" << response->status << endl;
			return false;
		}
		std::string length = response->get_header_value("Content-Length");
		int64_t size = StringClass::AtoI(length);
		return size;
	}

	bool _IntervalDownLoad(const std::string& hostIP, const std::string& name, int64_t begin, int64_t end)
	{
		std::string requestPath = "/downloadfile/" + name;
		std::string realPath = DOWNLOAD_FOLDER + name;
		httplib::Client c(hostIP.c_str(), P2P_PORT);

		httplib::Headers header;
		std::stringstream s;
		s << "bytes=" << begin << "-" << end;
		cout << "�ֿ�����������Ϣ��" << s.str() << endl;
		header.insert(std::make_pair("Range", s.str()));  // ���ͷ��Range�ֶ�
		auto response = c.Get(requestPath.c_str(), header);

		if (response == NULL || response->status != 206)
		{
			if (response == NULL)
				cout << "��ȡ��ӦΪNULL��" << endl;
			else
				cout << "��Ӧ״̬�룺" << response->status << endl;
			cout << "�ֿ������ļ�ʧ�ܡ�" << endl;
			return false;
		}
		cout << "�ͻ���д���ļ���[ " << response->body << " ]" << endl;
		FileClass::Write(realPath, response->body, begin);
		cout << "�ͻ��˷ֿ�д���ļ��ɹ���" << endl;
		return true;
	}
private:
	std::vector<Host> _online_host;
};

class Server
{
public:
	bool Start()
	{
		_server.Get("/hostpair", HostPair);
		_server.Get("/list", ShareList);
		_server.Get("/downloadfile/.*", DownLoad);

		_server.listen("0.0.0.0", P2P_PORT);  // ��������
		return true;
	}
private:
	// ���������Ӧ
	static void HostPair(const httplib::Request& request, httplib::Response& response)
	{
		response.status = 200;
		return;
	}

	// ��ȡ�����ļ��б���Ӧ
	static void ShareList(const httplib::Request& request, httplib::Response& response)
	{
		if (!boost::filesystem::exists(SHARED_FOLDER))  // �������ļ��в������򴴽�һ��
			boost::filesystem::create_directory(SHARED_FOLDER);

		boost::filesystem::directory_iterator it(SHARED_FOLDER);  // ʵ����Ŀ¼������ 
		boost::filesystem::directory_iterator end;  // ʵ����Ŀ¼��������ĩβ
		for (; it != end; it++)  // ����Ŀ¼
		{
			if (boost::filesystem::is_directory(it->status()))  // �����Ŀ¼������
				continue;
			
			std::string name = it->path().filename().string();  // ֻȡ�ļ���
			response.body += name + "\r\n";
		}
		response.status = 200;
		return;
	}

	// ��ȡ�ֿ���������
	static bool GetInterval(std::string& interval, int64_t& begin, int64_t& end)
	{
		size_t pos1, pos2;
		pos1 = interval.find("-");
		if (pos1 == std::string::npos) 
			return false;
		pos2 = interval.find("bytes=");
		if (pos2 == std::string::npos) 
			return false;

		std::stringstream s;
		s << interval.substr(pos2 + 6, pos1 - pos2 - 6);
		s >> begin;
		s.clear();
		s << interval.substr(pos1 + 1);
		s >> end;
		return true;
	}

	// �ļ�������Ӧ
	static void DownLoad(const httplib::Request& request, httplib::Response &response)
	{
		cout << "������յ��ļ���������" << request.path << endl;
		boost::filesystem::path requestPath(request.path);
		std::string name = requestPath.filename().string();  // ֻ��ȡ�ļ���
		cout << "������յ����ļ�����" << name << " ·��:" << SHARED_FOLDER << endl;
		std::string realPath = SHARED_FOLDER + name;  // ����Ŀ¼�µ�ʵ���ļ�·�� 
		cout << "������յ����ļ�����·����" << realPath << endl;
		if (!boost::filesystem::exists(realPath) || boost::filesystem::is_directory(realPath))  // �ļ������Ҳ�ΪĿ¼
		{
			response.status = 404;
			return;
		}
		if (request.method == "GET")
		{
			if (request.has_header("Range"))  // ����Range�ֶε�Ϊ�ֿ鴫��
			{
				std::string interval = request.get_header_value("Range");  // ��ȡRange�ֶ�  bytes=start-end

				int64_t begin, end, length;
				GetInterval(interval, begin, end);
				length = end - begin + 1;  // ���䳤��
				cout << "�ֿ����䣺" << realPath << ": range :" << begin << "-" << end << endl;

				std::string body;
				FileClass::ReadInterval(realPath, &body, length, begin);
				response.set_content(body.c_str(), length, "text/plain");  // �����������ݣ����ȣ���������
				response.set_header("Content-Length", std::to_string(response.body.size()).c_str());  // ����ͷ�������ĳ���
				response.status = 206;  // �ֿ鴫��״̬��
				cout << "����˻�ȡ�������ݣ�[" << response.body << "]" << endl;
			}
			else  // ������Range�ֶΣ�ֱ�����������ļ�
			{
				if (FileClass::Read(realPath, &response.body) == false)
				{
					response.status = 500;
					return;
				}
				response.status = 200;
			}
		}
		else  // ���head����ֻ��ȡͷ��
		{
			int64_t size = FileClass::GetFileSize(realPath);
			response.set_header("Content-Length", std::to_string(size).c_str());  // ������Ӧ��ͷ����Ϣ 
			response.status = 200;
		}
		cout << "������ļ�����������Ӧ��ϡ�" << endl;;
		return;
	}
private:
	httplib::Server _server;
};