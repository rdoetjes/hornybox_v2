#ifndef DEAMON_H
#define DEAMON_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <time.h>


void deamon(const char *chDirPath);

#endif
