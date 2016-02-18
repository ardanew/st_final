#ifndef ERRORS_H
#define ERRORS_H

struct Error
{
	static const int CREATE_SRV_SOCKET = -100;
	static const int THREADPOOL_INIT = -101;
	static const int BIND_SRV_SOCKET = -102;
	static const int LISTEN_SRV_SOCKET = -103;
	static const int WSA_STARTUP = -104;
	static const int REUSEADDR_SRV = -105;

	static const int FORK_FAILED = -200;
	static const int SETSID_FAILED = -201;
};

#endif
