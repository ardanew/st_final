#ifndef DEAMONIZE_H
#define DEAMONIZE_H

#ifdef _WIN32

int daemonize(int nochdir, int noclose, int wait_sigusr1) { return 0; }
int daemon_is_ready(void) { return 0; }

#else

#include "ccan_daemon_with_notify_daemon_with_notify.h"

#endif

#endif
