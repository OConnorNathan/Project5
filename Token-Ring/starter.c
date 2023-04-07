#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Shared.h"

#define PORT_NUMBER 54321

int startPhilosopher(int port1, int port2, int master)
{
    int childId;

    childId = fork();

    if (childId == -1)
    {
        printf("starter: error creating fork\n");
        exit(1);
    }
    else if (childId == 0)
    {
        //Child
        int id_len1 = snprintf(NULL, 0, "%d", port1);
        char* id_str1 = malloc(id_len1+1);
        snprintf(id_str1, id_len1+1, "%d", port1);

        int id_len2 = snprintf(NULL, 0, "%d", port2);
        char* id_str2 = malloc(id_len2+1);
        snprintf(id_str2, id_len2+1, "%d", port2);

        int id_len3 = snprintf(NULL, 0, "%d", master);
        char* id_str3 = malloc(id_len3+1);
        snprintf(id_str3, id_len3+1, "%d", master);

        execl("./philosopher", "philosopher", id_str1, id_str2, id_str3, (char *) NULL);
    }
    
    return childId;
}

int main()
{
    pid_t childId;
    pid_t wpid;
    int status = 0;
    int i;
    int port1, port2, master;

    for (i = 0; i < NUMPHILOSOPHERS; i++)
    {
        port1 = PORT_NUMBER+i;
        port2 = PORT_NUMBER+(i+1)%NUMPHILOSOPHERS;
        master = 0;
        if (i == 0)
            master = 1;
        childId = startPhilosopher(port1, port2, master);
        printf("Philosopher %d started! Pid = %d\n", i, childId);
    }
    printf("All child processes started.\n");

    while ((wpid = wait(&status)) > 0);

    printf("All child processes finished.\n");

    exit(0);
}