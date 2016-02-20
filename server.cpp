#include "server.h"
#include "errors.h"
#include <chrono>
#include <thread>

HttpSrv::HttpSrv() : m_srvSock(0) {}

HttpSrv::~HttpSrv() { deinit(); }

void HttpSrv::deinit()
{
	m_threadPool.deinit();

	if( m_srvSock ) { closesocket(m_srvSock); m_srvSock = 0; }

	std::for_each(m_deqClients.begin(), m_deqClients.end(), [](HttpClient* cln) { cln->stop(); });
	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::for_each(m_deqClients.begin(), m_deqClients.end(), [](HttpClient* del) { delete del; });
	m_deqClients.clear();

#ifdef _WIN32
	WSACleanup();
#endif
}

int HttpSrv::init(const std::string &sLocalIp, uint16_t nLocalPort, const std::string &sDir)
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

	m_sDir = sDir;
	return 0;
}

int HttpSrv::doAccept()
{
	struct sockaddr_in client_addr;
	int nClientAddrLen = sizeof(client_addr);
	SOCKET sockClient = accept(m_srvSock, (struct sockaddr*)&client_addr, &nClientAddrLen);
	if( sockClient == INVALID_SOCKET )
	{
		std::cout << "accept() failed, continue listening..." << std::endl;
		return Error::NONFATAL_ACCEPT_FAILED;
	}

	bool bMaxClientsReached = false;
	{ // do not accept too many clients
		std::unique_lock<std::mutex> protect(m_mutClients);
		if( m_deqClients.size() == MAX_SRV_THREADS )
			bMaxClientsReached = true;
	}
	if( bMaxClientsReached )
	{
		std::cout << "Maximum clients size is reached, disconnecting client." << std::endl;
		const char szHttpBusy[] = "HTTP/1.1 421 Misdirected Request\r\nContent-Type: text/html;charset=win-1251\r\nContent-Length: 41\r\nCache-Control: no-cache, no-store\r\n\r\n<html><body>Server is busy</body></html>";
		send(sockClient, szHttpBusy, sizeof(szHttpBusy), 0);
		closesocket(sockClient);
		return Error::NONFATAL_TOO_MANY_CLIENTS;
	}

	std::cout << "socket accepted, addr = " << inet_ntoa(client_addr.sin_addr) << " port = " << ntohs(client_addr.sin_port) << std::endl;
	HttpClient *pCln = new HttpClient(sockClient, client_addr, m_sDir);

	// when client disconnects - call onClientDisconnectHandler()
	pCln->setDisconnectHandler(std::bind(&HttpSrv::onClientDisconnectHandler, this, pCln) );

	{
		std::unique_lock<std::mutex> protect(m_mutClients);
		m_deqClients.push_back(pCln);

		// pass to thread pool
		m_threadPool.enqueue(std::bind(&HttpClient::execute, pCln), pCln);
	}

	return 0;
}

void HttpSrv::onClientDisconnectHandler(HttpClient *pCln)
{
	std::unique_lock<std::mutex> protect(m_mutClients);

	auto it = std::find_if(m_deqClients.begin(), m_deqClients.end(), [=](HttpClient *cln)->bool {
		if( cln == pCln )
			return true;
		return false;
	});
	if( it == m_deqClients.end() )
		return;

	std::cout << "Client ";
	pCln->printAddr();
	std::cout << " disconnected." << std::endl;

	pCln->deinit();
	delete pCln;
	m_deqClients.erase(it);
}