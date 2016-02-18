#ifndef DEAMONIZE_H
#define DEAMONIZE_H

#if !defined(_WIN32)

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include "errors.h"

int daemonize()
{
	pid_t pid, sid;

	pid = fork(); //Fork the Parent Process
	if( pid < 0 ) { return Error::FORK_FAILED; }

	if( pid > 0 ) { exit(EXIT_SUCCESS); } //We got a good pid, Close the Parent Process

	
	umask(0); //Change File Mask
	sid = setsid(); //Create a new Signature Id for our child
	if( sid < 0 ) { return Error::FORK_FAILED; }
}

#else

int daemonize() { return 0; }

#endif

#endif
