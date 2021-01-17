#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
///login james 123 128.100.13.140 8080

void* recieveLoop();
int socketResp;

struct message {
	unsigned int type;
	unsigned int size;
	unsigned char source[100];
	unsigned char data[1000];
};

int main(){
    char buffer[1000] = {};
    bzero(buffer, sizeof(buffer));

    char command[50];
    char user[50];
    char pass[50];
    char addr[50];
    char port[50];
    bool login = false;

    printf("\033[0;33mEnter command and arguments: \033[0m");
    scanf("%s %s %s %s %s", command,user, pass, addr, port);

    socketResp = socket(AF_INET, SOCK_STREAM, 0);
    if(socketResp == -1){
        printf("Socket not created\n");
        return 0;
    }

    struct sockaddr_in hints;
    memset(&hints, 0, sizeof hints);
    hints.sin_family = AF_INET;
    hints.sin_port = htons(atoi(port));
    hints.sin_addr.s_addr = inet_addr(addr);

    int bindResp = connect(socketResp, (struct sockaddr*)&hints, sizeof hints);
    if(bindResp == -1){
        printf("\033[0;31mBind Failed.\033[0m\n");
        return 0;
    }
    printf("\033[0;32mConnected to %s\033[0m\n", addr);
    send(socketResp, strcat(pass,user), strlen(strcat(pass,user)), 0);
    recv(socketResp, buffer, 1000, 0);
    printf("Rec: %s\n", buffer);
    if (strstr(buffer, "LO_NAK") != NULL) {
        printf("\033[0;31mLogin failed.\033[0m\n");
        return 0;
    }
    else {
        printf("\033[0;32mLogin Successful.\033[0m\n");
    }
    // printf("Enter command and arguments: ");
    // scanf("%s %s %s %s %s", command,user, pass, addr, port);
    
    pthread_t id;
    pthread_create(&id, NULL, recieveLoop, NULL);
    while(true){
        bzero(buffer, sizeof(buffer));
        if(recv(socketResp, buffer, 1000, 0) == 0){
            printf("\033[0;31mData not received. Terminated.\033[0m\n");
            return 0;
        }else{
            printf("%s\n", buffer);
        }
    }
    return 0;
}

void* recieveLoop(){
    char buffer[1000] = {};
    while(true){
        bzero(buffer, sizeof(buffer));
        // scanf("%s", &buffer[0]);
        fgets(&buffer[0], 1000, stdin);
        send(socketResp, buffer, strlen(buffer), 0);
        if(strstr(buffer, "/quit") != NULL){
            close(socketResp);
            printf("\033[0;31mQuitting\033[0m\n");
            exit(EXIT_SUCCESS);
        }
        if(strstr(buffer, "/logout") != NULL){
            printf("\033[0;31mLogging Out\033[0m\n");
            main();
            return 0;
        }
    }
}