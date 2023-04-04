
#include "Shared.h"

pthread_mutex_t forks[NUMPHILOSOPHERS] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, 
PTHREAD_MUTEX_INITIALIZER};

void takeForks(int, int);
void* threadFunc(void*);

typedef struct philInfo {
    int pid;
    int cSocket;
} philInfo;

int main(){
    int sSocket, err, cSocLen;
    struct sockaddr_in sAddr;
    struct sockaddr_in cAddr;
    char buffer[BUFLEN];
    pthread_t threads[NUMPHILOSOPHERS];
    philInfo pInfo[NUMPHILOSOPHERS];

    int i;

    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(SERVERPORT);
    sAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sSocket == -1) {
        perror("Controller: socket creation failed");
        exit(1);
    }

    err = bind(sSocket, (struct sockaddr*)&sAddr, sizeof(struct sockaddr_in));
    if (err == -1) {
        perror("Controller: bind address to socket failed");
        exit(2);
    }
    

    int connections = 0;
    while (connections < NUMPHILOSOPHERS) {    //Wait for all philosophers
        printf("%d\n", connections);
        printf("Before Listen\n");
        err = listen(sSocket, NUMPHILOSOPHERS); //NOTE: Do we need to listen each time?
        if (err == -1) {
            perror("socServer: listen failed");
            exit(3);
        }
        printf("Before accept\n");
        cSocLen = sizeof(sAddr);
        pInfo[connections].cSocket = accept(sSocket, (struct sockaddr*)&cAddr, (socklen_t*)&cSocLen);
        printf("After accept\n");
        pInfo[connections].pid = connections;
    
        if (pInfo[connections++].cSocket == -1) {
            perror("socServer: accept failed");
            exit(4);
        }
    }
     printf("Exited loop\n");

    for (i = 0; i < NUMPHILOSOPHERS; i++) { //create a thread for each philosopher to handle recv and send
        err = pthread_create(&threads[i], NULL, threadFunc, (void*)&pInfo[i]); 
        if(err == -1){
            fprintf(stderr, "Failed to create threads with errno %d\n", errno);
        }
    }
    
    for (i = 0; i < NUMPHILOSOPHERS; i++) {     //send message to all philosophers when all have connected
        err = send(pInfo[i].cSocket, "All philosophers connected!", BUFLEN, 0);
        if(err == -1){
            fprintf(stderr, "Failed to send with errno %d\n", errno);
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

    printf("cSocket: %d\npid: %d\n", cSocket, pid);
    for (;;) {
        //printf("Recieving\n");
        int err = recv(cSocket, buffer, BUFLEN, 0);
        if(err == -1){
            fprintf(stderr, "Failed to recv with errno %d\n", errno);
        }
        if (buffer[0] == HUNGRY) {   //Philosopher: Hungry
            //takeForks(i); //Try to take forks
            memset(buffer, 0, BUFLEN);
        }
        else if (buffer[0] == DONE) {    //Philosopher: done (eating)

        }
        //else printf("We shouldn't be here!\n");
    }
}


void takeForks(int id, int cSocket) {
    char buffer[BUFLEN];
    int left = (id - 1 + NUMPHILOSOPHERS) % NUMPHILOSOPHERS;
    int right = (id + 1) % NUMPHILOSOPHERS;

    pthread_mutex_lock(&forks[left]);
    pthread_mutex_lock(&forks[right]);

    send(cSocket, EATING, BUFLEN, 0);    //EATEAINGTING

    recv(cSocket, buffer, BUFLEN, 0);
    pthread_mutex_unlock(&forks[left]);
    pthread_mutex_unlock(&forks[right]);
}

//void 