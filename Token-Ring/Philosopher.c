
#include "Shared.h"

pthread_mutex_t tokenLock = PTHREAD_MUTEX_INITIALIZER;

char getTask();
void* threadFunc(void*);

typedef struct SockInfo {
    int sendTo;
    int recvFrom;
    int serverStat;
} SockInfo;

int main(int argc, char* argv[]){

    if(argc < 4){
        printf("Invalid args: ./philosopher listenPort sendPort startAsServer (0 or 1)\n");
        exit(-1);
    }

    int listenPort = atoi(argv[1]);
    int sendPort = atoi(argv[2]);
    int serverStatus = atoi(argv[3]);

    srand(time(0));
    int err, listenClientSocket, sendClientSocket, sendServerSocket, cSocLen, i, left, right, id;
    struct sockaddr_in listenClientAddr;
    struct sockaddr_in sendClientAddr;
    struct sockaddr_in sendServerAddr;
    char buffer[BUFLEN];
    pthread_t thread;
    SockInfo sockInfo;

    listenClientAddr.sin_family = AF_INET;
    listenClientAddr.sin_port = htons(listenPort);
    listenClientAddr.sin_addr.s_addr = inet_addr(SERVERIP);

    sendClientAddr.sin_family = AF_INET;
    sendClientAddr.sin_port = htons(sendPort);
    sendClientAddr.sin_addr.s_addr = inet_addr(SERVERIP);

    sendServerAddr.sin_family = AF_INET;
    sendServerAddr.sin_port = htons(sendPort);
    sendServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(serverStatus == 1){
        sendServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sendServerSocket == -1) {
            fprintf(stderr, "Philosopher (%d): socket creation failed\n", getpid());
            exit(13);
        }

        err = bind(sendServerSocket, (struct sockaddr*)&sendServerAddr, sizeof(struct sockaddr_in));
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): socket bind failed\n", getpid());
            exit(2);
        }

        err = listen(sendServerSocket, 1); //NOTE: Do we need to listen each time?
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): socket listen failed\n", getpid());
            exit(3);
        }

        cSocLen = sizeof(sendServerAddr);
        sendClientSocket = accept(sendServerSocket, (struct sockaddr*)&sendClientAddr, (socklen_t*)&cSocLen);

        memset(buffer, 0, BUFLEN);
        err = recv(sendClientSocket, buffer, BUFLEN, 0);       //Waits for other philosophers
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): recv failed\n", getpid());
            exit(16);
        }

        memset(buffer, 0, BUFLEN);
        listenClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenClientSocket == -1) {
            fprintf(stderr, "Philosopher (%d): socket creation failed\n", getpid());
            exit(13);
        }

        while(connect(listenClientSocket, (struct sockaddr*)&listenClientAddr, sizeof(struct sockaddr_in)) < 0){
        }

        //Main server sends ids to clients and it gets forwarded to the other clients
        id = 0;
        memset(buffer, 0, BUFLEN);
        sprintf(buffer, "1", BUFLEN);

        printf("ID: %d\n", id);
        err = send(sendClientSocket, buffer, BUFLEN, 0);       //Waits for other philosophers
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): send failed\n", getpid());
            exit(16);
        }

        sockInfo.sendTo = sendClientSocket;
        sockInfo.recvFrom = listenClientSocket;
        sockInfo.serverStat = serverStatus;
        printf("Philosopher (%d): Initialized\n", getpid());

    }
    else{
        memset(buffer, 0, BUFLEN);
        listenClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenClientSocket == -1) {
            fprintf(stderr, "Philosopher (%d): socket creation failed\n", getpid());
            exit(13);
        }

        while(connect(listenClientSocket, (struct sockaddr*)&listenClientAddr, sizeof(struct sockaddr_in)) < 0){
        }

        sprintf(buffer, "Initialized", BUFLEN);
        err = send(listenClientSocket, buffer, BUFLEN, 0);
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): send failed\n", getpid());
            exit(16);
        }

        sendServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sendServerSocket == -1) {
            fprintf(stderr, "Philosopher (%d): socket creation failed\n", getpid());
            exit(1);
        }

        err = bind(sendServerSocket, (struct sockaddr*)&sendServerAddr, sizeof(struct sockaddr_in));
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): socket bind failed\n", getpid());
            exit(2);
        }

        err = listen(sendServerSocket, 1);
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): socket listen failed\n", getpid());
            exit(3);
        }

        cSocLen = sizeof(sendServerAddr);
        sendClientSocket = accept(sendServerSocket, (struct sockaddr*)&sendClientAddr, (socklen_t*)&cSocLen);

        //Receive id number
        memset(buffer, 0, BUFLEN);
        err = recv(listenClientSocket, buffer, BUFLEN, 0);
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): recv failed\n", getpid());
            exit(16);
        }
        id = atoi(buffer);
        printf("ID: %d\n", id);

        memset(buffer, 0, BUFLEN);
        sprintf(buffer, "%d", (id + 1));
        err = send(sendClientSocket, buffer, BUFLEN, 0);
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): send failed\n", getpid());
            exit(16);
        }

        sockInfo.sendTo = sendClientSocket;
        sockInfo.recvFrom = listenClientSocket;
        sockInfo.serverStat = serverStatus;
        printf("Philosopher (%d): Initialized\n", getpid());

    }


    left = (id - 1 + NUMPHILOSOPHERS) % NUMPHILOSOPHERS;
    right = (id + 1) % NUMPHILOSOPHERS;
    
    //create thread to handle token
    err = pthread_create(&thread, NULL, threadFunc, (void*)&sockInfo);
    if(err == -1) {
        fprintf(stderr, "Controller: Failed to create threads with errno %d\n", errno);
    }

    while(true){
        char task = getTask();
        int sleepTime = (rand() % 5) + 1;

        if(task == THINKING){
            printf("Philosopher (%d): Thinking for %d seconds\n", getpid(), sleepTime);
            sleep(sleepTime);
        }
        else{
            printf("Philosopher (%d): Hungry, waiting for token\n", getpid());
            pthread_mutex_lock(&tokenLock);

            printf("Philosopher (%d): Eating for %d seconds\n", getpid(), sleepTime);
            sleep(sleepTime);
            printf("Philosopher (%d): Eating finished\n", getpid(), sleepTime);
            pthread_mutex_unlock(&tokenLock);
        }
    }
    pthread_join(thread, NULL);
}

void* threadFunc(void* p) {
    SockInfo* info = (SockInfo*)p;
    int err;
    int sendTo = info->sendTo;
    int recvFrom = info->recvFrom;
    int serverStatus = info->serverStat;
    char buffer[BUFLEN];

    //if its the master server initialize the token
    if(serverStatus){
        sleep(1);
        memset(buffer, 0, BUFLEN);
        strcpy(buffer, "Token");
        err = send(sendTo, buffer, BUFLEN, 0);
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): send failed\n", getpid());
            exit(16);
        }
    }
    while(true){
        err = recv(recvFrom, buffer, BUFLEN, 0);
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): recv failed\n", getpid());
            exit(16);
        }

        pthread_mutex_unlock(&tokenLock);

        pthread_mutex_lock(&tokenLock);
        err = send(sendTo, buffer, BUFLEN, 0);
        if (err == -1) {
            fprintf(stderr, "Philosopher (%d): send failed\n", getpid());
            exit(16);
        }
    }
}

char getTask(){
    char buf[1];

    int choice = rand() % 2;
    sprintf(buf, "%d", choice);

    if(buf[0] == THINKING){
        return THINKING;
    }
    else{
        return HUNGRY;
    }
    
}

