#include <iostream>

#include "final.h"
#include "server.h"
#ifdef _WIN32
#include "daemonize.h"
#else
#include "ccan_daemon_with_notify_daemon_with_notify.h"
#include <unistd.h> // sleep, dbg
#endif

// -h <ip> -p <port> -d <directory>
int main(int argc, char **argv)
{
	std::string sIp = "127.0.0.1";
	std::string sPort = "1415";
#ifdef _WIN32
	std::string sDir = "d:\\srv";
#else
	std::string sDir = "/tmp";
#endif

	for(int nArg = 1; nArg < argc; nArg += 2 )
	{
		if( nArg == argc - 1 )
			break; // even check

		if( strcmp(argv[nArg], "-h") == 0 )
			sIp = argv[nArg + 1];
		else if( strcmp(argv[nArg], "-p") == 0 )
			sPort = argv[nArg + 1];
		else if( strcmp(argv[nArg], "-d") == 0 )
			sDir = argv[nArg + 1];
	}
	
	if( sDir.length() == 0 ) { std::cout << "Wrong path parameter" << std::endl;  return -1; }

	// remove trailing slash if any
	std::string::iterator it = sDir.end() - 1;
	if( *it == '/' || *it == '\\' )
		sDir.erase(it);

	std::cout << "Server parameters :" << std::endl << " host = " << sIp << ":" << sPort << std::endl << " dir = " << sDir << std::endl;

	// run as background process
	// NOTE not implemented for windows
	
	int nDaemonizeRes = daemonize(1, 1, 1); // fork
	if( nDaemonizeRes < 0 )
	{
		std::cout << "Can't daemonize, err = " << nDaemonizeRes << std::endl;
		return -1;
	}
	else
		std::cout << "Daemonized." << std::endl;
	daemon_is_ready(); // tell parent to exit


	HttpSrv srv;
	int nInitRes = srv.init(sIp, std::stoi(sPort), sDir);
	if( nInitRes < 0 )
	{
		std::cout << "Can't start server, err = " << nInitRes << std::endl;
		return -1;
	}

	std::cout << "Server started." << std::endl;


	while( 1 )
	{
		int nAcceptRes = srv.doAccept();
		if( nAcceptRes < 0 )
		{
			std::cout << "Srv doAccept() fatal error, err = " << nAcceptRes << std::endl;
			break;
		}
	}

	std::cout << "Stopping server...";
	srv.deinit();
	std::cout << " done." << std::endl;
	return 0;
}
