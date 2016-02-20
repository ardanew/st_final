#ifndef SERVER_H
#define SERVER_H
#include <cstdint>
#include <string>
#include <deque>
#include <functional>
#include <algorithm>

#include "includes.h"
#include "threadpool.h"
#include "client.h"
#include <mutex>

class HttpSrv
{
public:
	HttpSrv();
	~HttpSrv();

	void deinit();

	/**
	\brief Initializes the server.
	\param sLocalIp server ip address to listen on, pass "0.0.0.0" to listen on all available adapters.
	\param nLocalPort server listen port.
	\return negative when failure, 0 if success.
	**/
	int init(const std::string &sLocalIp, uint16_t nLocalPort, const std::string &sDir);

	/**
	\brief Accepts an incoming connection and handles it.
	\details Must be done in loop.
			Blocks calling thread.
	\return 0 if success, negative on critical failure, positive for non-fatal errors.
	**/
	int doAccept();

	

protected:
	void onClientDisconnectHandler(HttpClient *pCln);

	std::mutex m_mutClients;
	static const size_t MAX_SRV_THREADS = 10;
	ThreadPool m_threadPool;

	std::deque<HttpClient*> m_deqClients;

	SOCKET m_srvSock;

	std::string m_sDir;
};


#endif
