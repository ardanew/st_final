#include <iostream>

#include "final.h"
#include "server.h"

#include <unistd.h> // sleep

int main(int argc, char **argv)
{
	HttpSrv srv;

	int nInitRes = srv.init("127.0.0.1", 1415);
	if( nInitRes < 0 )
	{
		std::cout << "Can't start server, err = " << nInitRes << std::endl;
		return -1;
	}

	int nDaemonizeRes = daemonize();
	if( nDaemonizeRes < 0 )
	{
		std::cout << "Can't daemonize, err = " << nDaemonizeRes << std::endl;
		return -1;
	}

sleep(8);
	std::cout << "Stopping server...";
	srv.deinit();
	std::cout << " done." << std::endl;
	return 0;
}
