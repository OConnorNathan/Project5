
#include "Shared.h"

pthread_mutex_t forks[NUMPHILOSOPHERS];

void takeForks(int, int, int*);
void* threadFunc(void*);

typedef struct philInfo {
    int pid;
    int cSocket;
    int *numEatingAddr;
} philInfo;


int numEating = 0;
pthread_mutex_t eatingVar;
pthread_mutex_t forkLocking;

int main() {
    int i;
    int sSocket, err, cSocLen;
    int opt = 1;
    struct sockaddr_in sAddr;
    struct sockaddr_in cAddr;
    char buffer[BUFLEN];
    pthread_t threads[NUMPHILOSOPHERS];
    philInfo pInfo[NUMPHILOSOPHERS];

    for (i = 0; i < NUMPHILOSOPHERS; i++) {     //mutex initialization
        pthread_mutex_init(&forks[i], NULL);
    }
    pthread_mutex_init(&eatingVar, NULL);
    pthread_mutex_init(&forkLocking, NULL);

    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(SERVERPORT);
    sAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sSocket == -1) {
        perror("Controller: socket creation failed");
        exit(1);
    }

    err = setsockopt(sSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    if (err == -1)
    {
        perror("Controller: error setsockopt");
        exit(1);
    }

    err = bind(sSocket, (struct sockaddr*)&sAddr, sizeof(struct sockaddr_in));
    if (err == -1) {
        perror("Controller: bind address to socket failed");
        exit(2);
    }

    int connections = 0;
    while (connections < NUMPHILOSOPHERS) {    //Wait for all philosophers
        err = listen(sSocket, NUMPHILOSOPHERS); //NOTE: Do we need to listen each time?
        if (err == -1) {
            perror("Controller: listen failed");
            exit(3);
        }
        cSocLen = sizeof(sAddr);
        pInfo[connections].cSocket = accept(sSocket, (struct sockaddr*)&cAddr, (socklen_t*)&cSocLen);
        pInfo[connections].pid = connections;
        pInfo[connections].numEatingAddr = &numEating;
    
        if (pInfo[connections++].cSocket == -1) {
            perror("Controller: accept failed");
            exit(4);
        }
    }

    for (i = 0; i < NUMPHILOSOPHERS; i++) { //create a thread for each philosopher to handle recv and send
        err = pthread_create(&threads[i], NULL, threadFunc, (void*)&pInfo[i]); 
        if(err == -1) {
            fprintf(stderr, "Controller: Failed to create threads with errno %d\n", errno);
            exit(5);
        }
    }
    
    for (i = 0; i < NUMPHILOSOPHERS; i++) {     //send message to all philosophers when all have connected
        err = send(pInfo[i].cSocket, "All philosophers connected!", BUFLEN, 0);
        if(err == -1) {
            fprintf(stderr, "Controller: Failed to send with errno %d\n", errno);
            exit(6);
        }
    }

    for (i = 0; i < NUMPHILOSOPHERS; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}

void* threadFunc(void* p) {
    struct philInfo* info = (struct philInfo*)p;
    int pid = info->pid;
    int cSocket = info->cSocket;
    char buffer[BUFLEN];

    for (;;) {
        buffer[0] = '\0';
        int err = recv(cSocket, buffer, BUFLEN, 0);
        if(err == -1){
            fprintf(stderr, "Controller: Failed to recv with errno %d\n", errno);
            exit(7);
        }
        if (buffer[0] == HUNGRY) {   //Philosopher: Hungry
            takeForks(pid, cSocket, info->numEatingAddr); //Try to take forks
            memset(buffer, 0, BUFLEN);

        }
        else if (buffer[0] == DONE) {    //Philosopher: done (eating)
            printf("Controller: Received Done!\n");
        }
        //else printf("We shouldn't be here!\n");

    }
}


void takeForks(int id, int cSocket, int *eatingAddr) {
    char buffer[BUFLEN];
    char buf[2];
    int left = (id - 1 + NUMPHILOSOPHERS) % NUMPHILOSOPHERS; //NOTE: In token ring, left is the same as philosopher id. But if we do id-1...are we doing chopsticks 2 
    int right = (id + 1) % NUMPHILOSOPHERS;
    int err;
    pthread_mutex_lock(&forks[left]);
    pthread_mutex_lock(&forks[right]);

    memset(buffer, 0, BUFLEN);
    strncpy(buffer, "Controller: Access granted for forks ", 100);
    sprintf(buf, "%d", left);
    strcat(buffer, buf);
    strcat(buffer, ", ");
    sprintf(buf, "%d", right);
    strcat(buffer, buf);

    printf("%s\n", buffer);
    err = send(cSocket, buffer, BUFLEN, 0);
    if (err == -1) {
        perror("Controller: failed to send to socket");
        exit(6);
    }

    pthread_mutex_lock(&eatingVar);
    (*eatingAddr)++;
    printf("Controller: Number of philosophers eating %d\n", (*eatingAddr));
    pthread_mutex_unlock(&eatingVar);

    memset(buffer, 0, BUFLEN);

    err = recv(cSocket, buffer, BUFLEN, 0);
    if (err == -1) {
        perror("Controller: failed to read from socket");
        exit(7);
    }

    pthread_mutex_lock(&eatingVar); 
    (*eatingAddr)--;
    printf("Controller: Number of philosophers eating %d\n", (*eatingAddr));
    pthread_mutex_unlock(&eatingVar); 
    
    pthread_mutex_unlock(&forks[left]);
    pthread_mutex_unlock(&forks[right]);
    printf("Controller: Released forks %d, %d\n", left, right);
}

//void 