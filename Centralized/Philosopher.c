
#include "Shared.h"

char getTask();

int main(){
    srand(time(0));
    char task;
    int err, cSocket;
    struct sockaddr_in cAddr;
    char buf1[BUFLEN];

    cAddr.sin_family = AF_INET;
    cAddr.sin_port = htons(SERVERPORT);
    cAddr.sin_addr.s_addr = inet_addr(SERVERIP);

    //Create socket
    cSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (cSocket == -1) {
        printf(("Philosopher (%d): socket creation failed. Error = %s\n", getpid(), strerror(errno)));
        exit(1);
    }

    sleep(2);

    //Create connection to ServerC
    err = connect(cSocket, (struct sockaddr*)&cAddr, sizeof(struct sockaddr_in));
    if (err == -1) {
        perror(("Philosopher (%d): connect failed with error: %s\n", getpid(), strerror(errno)));
        exit(8);
    }

    err = recv(cSocket, buf1, BUFLEN, 0);       //Waits for other philosophers
    if (err == -1) {
        fprintf(stderr, "Philosopher (%d): recv failed\n", getpid());
        exit(7);
    }

    while(true){
        memset(buf1, 0, BUFLEN);
        task = getTask();
        int sleepTime = (rand() % 5) + 1;

        if(task == THINKING){
            printf("Philosopher (%d): Thinking for %d seconds\n", getpid(), sleepTime);
            sleep(sleepTime);
        }
        else{
            printf("Philosopher (%d): Hungry.\n", getpid());

            memset(buf1, '\0', BUFLEN);
            buf1[0] = task;
            err = send(cSocket, buf1, BUFLEN, 0);
            //printf("Philosopher (%d): Sent message = %s\n", getpid(), buf1);
            if (err == -1) {
                fprintf(stderr, "Philosopher (%d): send failed with errno %s\n", getpid(), strerror(errno));
                exit(6);
            }

            memset(buf1, '\0', BUFLEN);
            err = recv(cSocket, buf1, BUFLEN, 0);
            if (err == -1) {
                fprintf(stderr, "Philosopher (%d): recv failed\n", getpid());
                exit(7);
            }

            //printf("Philosopher (%d): %s\n", getpid(), buf1);
            printf("Philosopher (%d): Eating for %d seconds\n", getpid(), sleepTime);
            sleep(sleepTime);

            memset(buf1, '\0', BUFLEN);
            buf1[0] = DONE;
            err = send(cSocket, buf1, BUFLEN, 0);
            if (err == -1) {
                fprintf(stderr, "Philosopher (%d): send failed\n", getpid());
                exit(6);
            }

            //It is also possible that we may need to do a receive here to wait until 
            //controller has received the DONE message
        }
    }

    printf("Philosopher (%d): Program finished\n", getpid());
}

char getTask(){
    if((rand() % 2 == 0)){
        return THINKING;
    }
    else{
        return HUNGRY;
    }  
}

