#ifndef UTILS
#define UTILS

#include <pthread.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod);
int ConvertStringToUI64(const char *str, uint64_t *val);

#endif