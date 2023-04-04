#ifndef SHARED_H
#define SHARED_H
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#define SERVERPORT 54321
#define SERVERPORTSTR "54321"
#define SERVERIP "127.0.0.1"
#define SERVERNAME "ahscentos"
#define BUFLEN 100
#define NUMPHILOSOPHERS 3
#define THINKING '0'
#define HUNGRY '1'
#define EATING '2'
#define DONE '3'
#endif