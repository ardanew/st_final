#ifndef CLIENT_H
#define CLIENT_H

#include <functional>
#include <string>
#include <sstream>
#include <fcntl.h>

#include "includes.h"
#include "errors.h"

class HttpClient
{
public:
	HttpClient(SOCKET clientSocket, sockaddr_in client_addr, const std::string &sDir);
	~HttpClient();
	void deinit();
	void setDisconnectHandler(std::function<void()> &&handler);
	void printAddr();
	void execute();
	void stop() { m_bStop = true; }
protected:
	SOCKET m_sock;
	sockaddr_in m_addr;
	const std::string &m_sDir;
	bool m_bStop;

	std::function<void()> onDisconnect;
};


#endif
