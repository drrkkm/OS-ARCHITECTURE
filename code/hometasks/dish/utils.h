#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define NCONFIG 10

struct dish
{
    char * type; // type < 100 chars
    int time;
};

struct msg {
    long mtype;
    char type[100];
    int ability;
};

void action(struct dish* config, char * type, int key);
struct dish* loadConfig(int nconfig, char* path);
void getStr(char ** type, char** array);

#endif // UTILS_H