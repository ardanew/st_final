#ifndef SERVER_H
#define SERVER_H
#include <cstdint>
#include <string>
#include "threadpool.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SOCKET int
#define closesocket(a) close(a)
#endif

class HttpSrv
{
public:
	HttpSrv();
	~HttpSrv();
	void deinit();
	int init(const std::string &sLocalIp, uint16_t nLocalPort);

protected:
	static const size_t MAX_SRV_THREADS = 10;
	ThreadPool m_threadPool;

	SOCKET m_srvSock;
};


#endif
