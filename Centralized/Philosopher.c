
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
    cSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (cSocket == -1) {
        perror(("Philosopher (%d): socket creation failed\n", getpid()));
        exit(13);
    }

    printf("CSocket %d\n", cSocket);
    //Create connection to ServerC
    err = connect(cSocket, (struct sockaddr*)&cAddr, sizeof(struct sockaddr_in));
    if (err == -1) {
        perror(("Philosopher (%d): connect failed with error: %d\n", getpid(), errno));
        exit(14);
    }

    err = recv(cSocket, buf1, BUFLEN, 0);       //Waits for other philosophers
    if (err == -1) {
        fprintf(stderr, "Philosopher (%d): recv failed\n", getpid());
        exit(16);
    }
    printf("%s\n", buf1);

    while(true){
        memset(buf1, 0, BUFLEN);
        task = getTask();
        int sleepTime = (rand() % 5) + 1;

        if(task == THINKING){
            printf("Philosopher (%d): Thinking for %d seconds\n", getpid(), sleepTime);
            sleep(sleepTime);
        }
        else{
            printf("Philosopher (%d): Hungry\n", getpid());

            err = send(cSocket, task, 1, 0);
            if (err == -1) {
                fprintf(stderr, "Philosopher (%d): send failed with errno %d\n", getpid(), errno);
                exit(15);
            }

            err = recv(cSocket, buf1, BUFLEN, 0);
            if (err == -1) {
                fprintf(stderr, "Philosopher (%d): recv failed\n", getpid());
                exit(16);
            }
            printf("Philosopher (%d): Eating for %d seconds\n", getpid(), sleepTime);
            sleep(sleepTime);

            err = send(cSocket, DONE, 1, 0);
            if (err == -1) {
                fprintf(stderr, "Philosopher (%d): send failed\n", getpid());
                exit(15);
            }
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

