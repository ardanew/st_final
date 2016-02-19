#include <iostream>

#include "final.h"
#include "server.h"
#ifdef _WIN32
#include "daemonize.h"
#else
#include "ccan_daemon_with_notify_daemon_with_notify.h"
#include <unistd.h> // sleep, dbg
#endif

int main(int argc, char **argv)
{
	HttpSrv srv;

	int nInitRes = srv.init("127.0.0.1", 1415);
	if( nInitRes < 0 )
	{
		std::cout << "Can't start server, err = " << nInitRes << std::endl;
		return -1;
	}

	int nDaemonizeRes = daemonize(1, 1, 1); // fork
	if( nDaemonizeRes < 0 )
	{
		std::cout << "Can't daemonize, err = " << nDaemonizeRes << std::endl;
		return -1;
	}
	else
		std::cout << "Daemonized." << std::endl;
	daemon_is_ready(); // tell parent to exit

	sleep(6);

	std::cout << "Stopping server...";
	srv.deinit();
	std::cout << " done." << std::endl;
	return 0;
}
