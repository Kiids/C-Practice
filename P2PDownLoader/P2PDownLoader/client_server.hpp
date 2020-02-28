#pragma once
#include <thread>
#include <boost/filesystem.hpp>
#include "utensil.hpp"
#include "httplib.h"

#define P2P_PORT 9000
#define MAX_IP_BUFFER 16  // 255.255.255.255 + \0字符串结束标志，共需16字节
//#define MAX_INTERVAL (100 * 1024 * 1024)
#define MAX_INTERVAL (5)  // 方便测试
#define SHARED_FOLDER "./SharedFile/"
#define DOWNLOAD_FOLDER "./DownloadFile/"

class Host  // 主机
{
public:
	uint32_t _ip_address;  // 配对主机的IP地址
	bool _pair;  // 存放配对结果，成功true，失败false
};

class Client
{
public:
	bool Start()
	{
		// 客户端循环运行，以便多次下载
		while (1)
		{
			OnlineHost();
		}
		return true;
	}

	// 主机配对的线程入口函数
	void HostPair(Host* host)
	{
		host->_pair = false;
		char buf[MAX_IP_BUFFER] = { 0 };
		inet_ntop(AF_INET, &host->_ip_address, buf, MAX_IP_BUFFER);
		httplib::Client c(buf, P2P_PORT);  // 实例化一个httplib客户端对象
		auto response = c.Get("/hostpair");  // 向服务端发送数据为/hostpair的GET请求，若连接失败Get会返回NULL
		if (response && response->status == 200)  // 判断响应结果正确
			host->_pair = true;  // 设置主机配对结果
		return;
	}

	// 获取在线主机
	bool OnlineHost()
	{
		char c = 'Y';
		if (!_online_host.empty())
		{
			cout << "是否重新查看在线主机（Y/N）：";
			fflush(stdout);
			std::cin >> c;
		}
		if (c == 'Y')
		{
			cout << "主机匹配中..." << endl;
			std::vector<Adapter> v;
			AdapterClass::GetAllAdapter(&v);  // 获取所有网卡信息存放到v
			std::vector<Host> vHost;  // 存放所有主机的IP地址
			for (int i = 0; i < v.size(); i++)  // 获取所有主机的IP地址列表
			{
				uint32_t ip = v[i]._ip_address;
				uint32_t mask = v[i]._mask_address;
				uint32_t net = (ntohl(ip & mask));  // 计算网络号，ntol将无符号长整形数从网络字节序转换为主机字节序
				uint32_t maxHost = (~ntohl(mask));  // 计算最大主机号
				for (int j = 1; j < (int32_t)maxHost; j++)
				{
					uint32_t hostIP = net + j;  // 这个主机IP的计算应使用主机字节序（小端字节序）的网络号
					Host host;
					host._ip_address = htonl(hostIP);  // 将这个主机字节序IP地址转换为网络字节序
					host._pair = false;
					vHost.push_back(host);
				}
			}
			// 配对vHost中的主机，多线程进行配对
			std::vector<std::thread*> threadList(vHost.size());
			for (int i = 0; i < vHost.size(); i++)
				threadList[i] = new std::thread(&Client::HostPair, this, &vHost[i]);
			cout << "主机配对请求中..." << endl;
			// 等待所有线程配对完成，判断结果添加在线主机到_online_host
			for (int i = 0; i < vHost.size(); i++)
			{
				threadList[i]->join();  // 等待一个线程退出
				if (vHost[i]._pair == true)
					_online_host.push_back(vHost[i]);
				delete threadList[i];
			}
		}
		// 打印所有在线主机IP，提供给用户选择
		for (int i = 0; i < _online_host.size(); i++)
		{
			char buf[MAX_IP_BUFFER] = { 0 };
			inet_ntop(AF_INET, &_online_host[i]._ip_address, buf, MAX_IP_BUFFER);
			cout << "->    " << buf << endl;
		}
		cout << "请选择主机，获取共享文件列表：";
		fflush(stdout);
		std::string s;
		std::cin >> s;
		ShareList(s);
		return true;
	}

	// 获取共享文件列表
	bool ShareList(const std::string& hostIP)
	{
		if (!boost::filesystem::exists(SHARED_FOLDER))  // 若共享文件夹不存在，则创建一个
			boost::filesystem::create_directory(SHARED_FOLDER);

		httplib::Client c(hostIP.c_str(), P2P_PORT);
		auto response = c.Get("/list");
		if (response == NULL || response->status != 200)
		{
			cout << "获取共享文件列表响应错误。" << endl;
			return false;
		}

		cout << response->body << endl;
		cout << "请选择需要下载的文件：";
		fflush(stdout);
		std::string name;
		std::cin >> name;
		IntervalDownLoad(hostIP, name);
		return true;
	}

	// 下载文件，小文件直接下载
	bool AllDownLoad(const std::string& hostIP, const std::string& name)
	{
		std::string requestPath = "/downloadfile/" + name;
		httplib::Client c(hostIP.c_str(), P2P_PORT);
		cout << "向服务端发送文件下载请求：" << hostIP << ":" << requestPath << endl;
		auto response = c.Get(requestPath.c_str());
		if (response == NULL || response->status != 200)
		{
			cout << "下载文件获取响应信息失败。" << endl;
			return false;
		}
		cout << "获取文件下载响应成功。" << endl;

		if (!boost::filesystem::exists(DOWNLOAD_FOLDER))  // 若下载文件夹不存在则创建一个
			boost::filesystem::create_directory(DOWNLOAD_FOLDER);

		std::string realPath = DOWNLOAD_FOLDER + name;
		if (FileClass::Write(realPath, response->body) == false)
		{
			cout << "文件下载失败。" << endl;
			return false;
		}
		cout << "文件下载成功。" << endl;
		return true;
	}

	// 分块下载
	bool IntervalDownLoad(const std::string& hostIP, const std::string& name)
	{
		std::string requestPath = "/downloadfile/" + name;
		int64_t size = GetSize(hostIP, requestPath);
		if (size < MAX_INTERVAL)
		{
			cout << "文件较小，直接下载。" << endl;
			return AllDownLoad(hostIP, name);
		}
		cout << "文件过大，分块下载。" << endl;

		// 分块个数的计算
		int64_t count = 0;
		// 文件大小能整除块大小，分块个数 = 文件大小 / 分块大小
		// 文件大小不能整除块大小，分块个数 = 文件大小 / 分块大小 + 1
		if ((size % MAX_INTERVAL) == 0)
			count = size / MAX_INTERVAL;
		else
			count = size / MAX_INTERVAL + 1;

		int64_t begin = 0, end = 0;
		for (int i = 0; i < count; i++)
		{
			begin = i * MAX_INTERVAL;
			if (i == (count - 1))  // 最后一个分块
				end = size - 1;
			else
				end = ((i + 1) * MAX_INTERVAL) - 1;
			cout << "客户端请求分块：" << begin << "-" << end << endl;
			_IntervalDownLoad(hostIP, name, begin, end);
		}
		cout << "文件分块下载成功。" << endl;
		return true;
	}
	
	int64_t GetSize(const std::string& hostIP, const std::string& requestPath)
	{
		httplib::Client c(hostIP.c_str(), P2P_PORT);
		auto response = c.Head(requestPath.c_str());
		if (response == NULL || response->status != 200)
		{
			cout << "获取文件大小失败。" << endl;
			if (response)
				cout << "响应状态码：" << response->status << endl;
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
		cout << "分块下载区间信息：" << s.str() << endl;
		header.insert(std::make_pair("Range", s.str()));  // 添加头部Range字段
		auto response = c.Get(requestPath.c_str(), header);

		if (response == NULL || response->status != 206)
		{
			if (response == NULL)
				cout << "获取响应为NULL。" << endl;
			else
				cout << "响应状态码：" << response->status << endl;
			cout << "分块下载文件失败。" << endl;
			return false;
		}
		cout << "客户端写入文件：[ " << response->body << " ]" << endl;
		FileClass::Write(realPath, response->body, begin);
		cout << "客户端分块写入文件成功。" << endl;
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

		_server.listen("0.0.0.0", P2P_PORT);  // 阻塞函数
		return true;
	}
private:
	// 主机配对响应
	static void HostPair(const httplib::Request& request, httplib::Response& response)
	{
		response.status = 200;
		return;
	}

	// 获取共享文件列表响应
	static void ShareList(const httplib::Request& request, httplib::Response& response)
	{
		if (!boost::filesystem::exists(SHARED_FOLDER))  // 若共享文件夹不存在则创建一个
			boost::filesystem::create_directory(SHARED_FOLDER);

		boost::filesystem::directory_iterator it(SHARED_FOLDER);  // 实例化目录迭代器 
		boost::filesystem::directory_iterator end;  // 实例化目录迭代器的末尾
		for (; it != end; it++)  // 迭代目录
		{
			if (boost::filesystem::is_directory(it->status()))  // 如果是目录则跳过
				continue;
			
			std::string name = it->path().filename().string();  // 只取文件名
			response.body += name + "\r\n";
		}
		response.status = 200;
		return;
	}

	// 获取分块下载区间
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

	// 文件下载响应
	static void DownLoad(const httplib::Request& request, httplib::Response &response)
	{
		cout << "服务端收到文件下载请求：" << request.path << endl;
		boost::filesystem::path requestPath(request.path);
		std::string name = requestPath.filename().string();  // 只获取文件名
		cout << "服务端收到的文件名：" << name << " 路径:" << SHARED_FOLDER << endl;
		std::string realPath = SHARED_FOLDER + name;  // 共享目录下的实际文件路径 
		cout << "服务端收到的文件下载路径：" << realPath << endl;
		if (!boost::filesystem::exists(realPath) || boost::filesystem::is_directory(realPath))  // 文件存在且不为目录
		{
			response.status = 404;
			return;
		}
		if (request.method == "GET")
		{
			if (request.has_header("Range"))  // 包含Range字段的为分块传输
			{
				std::string interval = request.get_header_value("Range");  // 获取Range字段  bytes=start-end

				int64_t begin, end, length;
				GetInterval(interval, begin, end);
				length = end - begin + 1;  // 区间长度
				cout << "分块区间：" << realPath << ": range :" << begin << "-" << end << endl;

				std::string body;
				FileClass::ReadInterval(realPath, &body, length, begin);
				response.set_content(body.c_str(), length, "text/plain");  // 设置正文内容，长度，正文类型
				response.set_header("Content-Length", std::to_string(response.body.size()).c_str());  // 设置头部中正文长度
				response.status = 206;  // 分块传输状态码
				cout << "服务端获取区间数据：[" << response.body << "]" << endl;
			}
			else  // 不包含Range字段，直接下载整个文件
			{
				if (FileClass::Read(realPath, &response.body) == false)
				{
					response.status = 500;
					return;
				}
				response.status = 200;
			}
		}
		else  // 针对head请求，只获取头部
		{
			int64_t size = FileClass::GetFileSize(realPath);
			response.set_header("Content-Length", std::to_string(size).c_str());  // 设置响应的头部信息 
			response.status = 200;
		}
		cout << "服务端文件下载请求响应完毕。" << endl;;
		return;
	}
private:
	httplib::Server _server;
};