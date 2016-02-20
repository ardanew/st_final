#include "client.h"

HttpClient::HttpClient(SOCKET clientSocket, sockaddr_in client_addr, const std::string &sDir) :
	m_sock(clientSocket), m_addr(client_addr), m_sDir(sDir), m_bStop(false)
{}

HttpClient::~HttpClient()
{
	deinit();
}

void HttpClient::deinit()
{
	if( m_sock != 0 )
	{
		shutdown(m_sock, 0);
		closesocket(m_sock);
		m_sock = 0;
	}
}

void HttpClient::setDisconnectHandler(std::function<void()> &&handler)
{
	onDisconnect = std::move(handler);
}

void HttpClient::printAddr()
{
	std::cout << "ip = " << inet_ntoa(m_addr.sin_addr) << " port = " << ntohs(m_addr.sin_port);
}

/** Returns true on success, or false if there was an error */
bool SetSocketBlockingEnabled(int fd, bool blocking)
{
	if( fd < 0 ) return false;

#ifdef WIN32
	unsigned long mode = blocking ? 0 : 1;
	return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
	int flags = fcntl(fd, F_GETFL, 0);
	if( flags < 0 ) return false;
	flags = blocking ? (flags&~O_NONBLOCK) : (flags | O_NONBLOCK);
	return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}

void HttpClient::execute()
{
	char szRecvBuf[10000];

	SetSocketBlockingEnabled(m_sock, false);

	fd_set read_set;
	struct timeval tv;


	while( 1 )
	{
		FD_ZERO(&read_set);
		FD_SET(m_sock, &read_set);
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		int nSelectRes = select(FD_SETSIZE, &read_set, NULL, NULL, &tv);
		if( nSelectRes < 0 )
		{
			std::cout << "Select() fails" << std::endl;
			onDisconnect();
			return;
		}

		int nRecvRes;

		if( FD_ISSET(m_sock, &read_set) )
		{
			nRecvRes = recv(m_sock, szRecvBuf, sizeof(szRecvBuf), 0);
			if( nRecvRes <= 0 )
			{
				onDisconnect();
				return;
			}
		}
		else
		{
			if( recv(m_sock, szRecvBuf, sizeof(szRecvBuf), 0) == 0 )
				break;
			if( m_bStop )
				break;
			continue;
		}


		szRecvBuf[nRecvRes] = '\0';

		std::istringstream split(szRecvBuf);
		std::string sRequestFileName;
		for( std::string token; std::getline(split, token); )
		{
			if( token.find("GET ") != std::string::npos )
			{
				std::istringstream ss(token);
				std::getline(ss, sRequestFileName, ' '); // GET
				std::getline(ss, sRequestFileName, ' '); // fname
			}
		}

		if( sRequestFileName.empty() )
		{
			std::cout << "invalid request" << std::endl;
			onDisconnect();
			return;
		}

		std::cout << "GET " << sRequestFileName << std::endl;

		std::string sUsedFileName;
		if( sRequestFileName == "/" )
			sUsedFileName = "/index.html";
		else
			sUsedFileName = sRequestFileName;


#ifdef _WIN32
		FILE *f = fopen((m_sDir + sUsedFileName).c_str(), "rb");
#else
		FILE *f = fopen((m_sDir + sUsedFileName).c_str(), "r");
#endif
		if( f == NULL )
		{ // 404
			std::cout << "No page " << (m_sDir + sUsedFileName) << ", disconnecting." << std::endl;

			std::string s404 = "HTTP/1.1 ";
			s404 += sRequestFileName;
			const char sz404page[] = "<html><body>404 Page not found :(</html></body>";
			s404 += " 404 Not Found\r\nContent-Type: text/html;charset=win-1251\r\nContent-Length: ";
			s404 += std::to_string(sizeof(sz404page));
			s404 += "\r\nCache - Control: no - cache, no - store\r\n\r\n";
			s404 += sz404page;

			send(m_sock, s404.c_str(), s404.length(), 0);
			onDisconnect();
			//fclose(f);
			return;
		}

		long lSize = 0;
		if( fseek(f, 0, SEEK_END) == 0 )
		{
			lSize = ftell(f);
			fseek(f, 0, SEEK_SET);
		}

		char *pBuf = new char[lSize + 1];

		if( fread(pBuf, lSize, 1, f) != 1 )
		{ // read error
			delete[] pBuf;

			std::cout << "Read error, disconnecting." << std::endl;

			std::string s404 = "HTTP/1.1 ";
			s404 += sRequestFileName;
			const char sz404page[] = "<html><body>404 Page not found :(</html></body>";
			s404 += " 404 Not Found\r\nContent-Type: text/html;charset=win-1251\r\nContent-Length: ";
			s404 += std::to_string(sizeof(sz404page));
			s404 += "\r\nCache - Control: no - cache, no - store\r\n\r\n";
			s404 += sz404page;

			send(m_sock, s404.c_str(), s404.length(), 0);
			onDisconnect();
			fclose(f);
			return;
		};

		//std::cout << pBuf << std::endl;

		std::string sMsg = "HTTP/1.1 ";
		sMsg += sRequestFileName;
		sMsg += " 200 OK \r\nContent-Type: text/html;charset=win-1251\r\nContent-Length: ";

		//if( pBuf[1] != 'h' )
		//	sMsg += std::to_string(lSize + 12 + 14 - 1);
		//else
			sMsg += std::to_string(lSize - 1);
		sMsg += "\r\nCache - Control: no - cache, no - store\r\n\r\n";


		pBuf[lSize] = '\0';

//		if( pBuf[1] != 'h' )
//			sMsg += "<html><body>";
		sMsg += pBuf;
		sMsg.pop_back();
		delete[] pBuf;
		//if( pBuf[1] != 'h' )
		//	sMsg += "</body></html>";

		if( send(m_sock, sMsg.c_str(), sMsg.length(), 0) != sMsg.length() )
		{
			std::cout << "Send fails, disconnecting." << std::endl;
			onDisconnect();
			fclose(f);
			return;
		}

		std::cout << "File " << sUsedFileName << " transferred." << std::endl;
		fclose(f);
	}

	onDisconnect();
}
