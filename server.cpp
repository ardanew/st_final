#include "server.h"
#include "errors.h"

HttpSrv::HttpSrv() : m_srvSock(0) {}

HttpSrv::~HttpSrv() { deinit(); }

void HttpSrv::deinit()
{
	m_threadPool.deinit();

	if( m_srvSock ) { closesocket(m_srvSock); m_srvSock = 0; }

#ifdef _WIN32
	WSACleanup();
#endif
}

int HttpSrv::init(const std::string &sLocalIp, uint16_t nLocalPort)
{
	deinit();

#ifdef _WIN32
	WSADATA wsaData;
	ZeroMemory(&wsaData, sizeof(wsaData));
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if( res != 0 ) { return Error::WSA_STARTUP;  }
#endif

	try { m_threadPool.init(MAX_SRV_THREADS); }
	catch( ... ) { return Error::THREADPOOL_INIT; }
	
	m_srvSock = socket(AF_INET, SOCK_STREAM, 0);
	if( m_srvSock < 0 ) { return Error::CREATE_SRV_SOCKET; }

	struct sockaddr_in sv_addr;
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(nLocalPort);
	sv_addr.sin_addr.s_addr = inet_addr(sLocalIp.c_str());
	if( bind(m_srvSock, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) != 0 ) { return Error::BIND_SRV_SOCKET;  }

	if( listen(m_srvSock, 0) != 0 ) { return Error::LISTEN_SRV_SOCKET;  }

	const int enableRAddr = 1;
	if( setsockopt(m_srvSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&enableRAddr, sizeof(int)) < 0 ) { return Error::REUSEADDR_SRV; }

	return 0;
}

