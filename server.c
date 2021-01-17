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
#include <time.h>

#define BACKLOG 10
#define MAXCONNECTIONS 10

int newSocket[MAXCONNECTIONS] = {0};
char sessActive[10][100] = {};
char sessActiveUsers[MAXCONNECTIONS][100] = {};
char sessActiveUsernames[MAXCONNECTIONS][100] = {};
char sessActiveUsernamesDisplay[MAXCONNECTIONS][100] = {};
// int sessNum = 0;
int socketResp;
struct sockaddr_in newAddr;
int activeConnections;
int sessNum;

void* readClient(void *threadid);

struct message {
	unsigned int type;
	unsigned int size;
	unsigned char source[100];
	unsigned char data[100];
};

int main(int argc, char const *argv[]){

	char* port = argv[1];
	sessNum = 0;
	socketResp = socket(AF_INET, SOCK_STREAM, 0);
	if(socketResp == -1){
		printf("Socket not created\n");
		return 0;
	}

	struct sockaddr_in hints;
	memset(&hints, 0, sizeof hints);
	hints.sin_family = AF_INET;
	hints.sin_port = htons(atoi(port));
	hints.sin_addr.s_addr = inet_addr("128.100.13.140");

	int bindResp = bind(socketResp, (struct sockaddr*)&hints, sizeof(hints));
	if(bindResp == -1){
		printf("Bind failed\n");
		return 0;
	}
	printf("\033[0;32mServer started at: 128.100.13.140:%d \033[0m\n", atoi(port));

	if(listen(socketResp, BACKLOG) == 0){
		printf("\033[0;33mWaiting for client to connect\033[0m\n");
	} 

	// accepting data from multiple clients
	socklen_t addrSize;


	char connections[][25] = {};
	pthread_t id[MAXCONNECTIONS];
	for(int socketID = 0; socketID < MAXCONNECTIONS; socketID++) {
		char usernames[][10] = {"yash", "james", "jim", "bob"};
    	char passwords[][10] = {"000", "123", "456", "789"};
		char userpass[100] = {};
		bool login = false;
		newSocket[socketID] = accept(socketResp, (struct sockaddr*)&newAddr, &addrSize);
		activeConnections++;
		strcpy(sessActiveUsers[socketID],"main");
		if(newSocket[socketID] < 0){
			return 0;
		}

		printf("\033[0;32mConnected to: %s:%d\033[0m\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		recv(newSocket[socketID], userpass, 15, 0);
		for(int i = 0; i < 4; i++) {
        	if (strstr(userpass, strcat(passwords[i],usernames[i])) != NULL) {
				strcpy(sessActiveUsernames[socketID],usernames[i]);
				strcpy(sessActiveUsernamesDisplay[socketID],usernames[i]);
				login = true;
				send(newSocket[socketID], "\033[0;32mLO_ACK\033[0m\n", sizeof("\033[0;32mLO_ACK\033[0m\n"), 0);
                break;
        	}
    	}
		
		if (!login) {
			send(newSocket[socketID], "\033[0;31mLO_NAK\033[0m\n", sizeof("\033[0;31mLO_NAK\033[0m\n"), 0);
			close(newSocket[socketID]);
		}
		else {
			pthread_create(&id[socketID], NULL, readClient, (void *)socketID);

			char bufferCombined[500]={};
			strcpy(bufferCombined, "\033[0;33m");
			strcat(bufferCombined, sessActiveUsernamesDisplay[socketID]);
			strcat(bufferCombined, " joined the server.\033[0m");
			for(int i = 0; i < activeConnections; i++){
				send(newSocket[i], bufferCombined, strlen(bufferCombined), 0);
			}
		}
	}

	return 0;
}

void* readClient(void *threadID){
	int currentSocketID = threadID;
	while(true){
		char buffer[1000] = {};
		bzero(buffer, sizeof(buffer));
		if (recv(newSocket[currentSocketID], buffer, 1000, 0) == 0) {
			return 0;
		}
		
		if(strstr(buffer, "/quit") != NULL || strstr(buffer, "/logout") != NULL){
			printf("\033[0;31mDisconnected from %s:%d\033[0m\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
			//activeConnections--;
			strcpy(sessActiveUsernames[currentSocketID],"");
			close(newSocket[currentSocketID]);
			newSocket[currentSocketID] = 0;
			pthread_exit(0);
			break;
		}
		else if (strstr(buffer, "/createsession") != NULL) {
			strcpy(sessActive[sessNum], buffer + 15);
			printf("Created session %d: %s\n", sessNum, sessActive[sessNum]);
			send(newSocket[currentSocketID], "\033[0;32mNS_ACK\033[0m\n", strlen("\033[0;32mNS_ACK\033[0m\n"), 0);
			char bufferCombined[500]={};
			strcpy(bufferCombined, "\033[0;33m");
			strcat(bufferCombined, sessActiveUsernamesDisplay[currentSocketID]);
			strcat(bufferCombined, " created a session: ");
			strcat(bufferCombined, sessActive[sessNum]);
			strcat(bufferCombined, "\033[0m");
			for(int i = 0; i < activeConnections; i++){
				send(newSocket[i], bufferCombined, strlen(bufferCombined), 0);
			}
			sessNum++;
		}
		else if (strstr(buffer, "/joinsession") != NULL || strstr(buffer, "/accept") != NULL) {
			char sessID[100] ={};
			bool joined = false;
			if(strstr(buffer, "/joinsession") != NULL) {
				strcpy(sessID, buffer + 13);
			}
			else {
				strcpy(sessID, buffer + 8);
			}
			for (int i = 0; i < MAXCONNECTIONS; i++) {
				if (strstr(sessActive[i], sessID) != NULL && strcmp(sessID, " ") != 0 && strcmp(sessID, "") != 0) {
					joined = true;
					send(newSocket[currentSocketID], "\033[0;33mJoined session\033[0m\n", strlen("\033[0;33mJoined session\033[0m\n"), 0);
					if(strstr(buffer, "/joinsession") != NULL) {
						if (strstr(sessActiveUsers[currentSocketID], "main") != NULL) {
							strcpy(sessActiveUsers[currentSocketID], buffer + 13);
						}
						else {
							strcat(sessActiveUsers[currentSocketID], buffer + 13);
						}
						strcat(sessActiveUsers[currentSocketID], ",");
					}
					else {
						if (strstr(sessActiveUsers[currentSocketID], "main") != NULL) {
							strcpy(sessActiveUsers[currentSocketID], buffer + 8);
						}
						else {
							strcat(sessActiveUsers[currentSocketID], buffer + 8);
						}
						strcat(sessActiveUsers[currentSocketID], ",");
					}
					//printf("%s", sessActiveUsers[currentSocketID]);
					break;
				}
			}
			if (!joined) {
				send(newSocket[currentSocketID], "\033[0;31mSession DNE\033[0m\n", strlen("\033[0;31mSession DNE\033[0m\n"), 0);
			} else {
				char bufferCombined[500]={};
				strcpy(bufferCombined, "\033[0;33m");
				strcat(bufferCombined, sessActiveUsernamesDisplay[currentSocketID]);
				strcat(bufferCombined, " joined ");
				strcat(bufferCombined, sessID);
				strcat(bufferCombined, "\033[0m");
				for(int i = 0; i < activeConnections; i++){
					send(newSocket[i], bufferCombined, strlen(bufferCombined), 0);
				}
			}
		}

		else if (strstr(buffer, "/leavesession") != NULL) {
			char sessionToLeave[100]={};
			char sessionsLeftover[100]={};
			strcpy(sessionToLeave, buffer + 14);

			if (strstr(sessActiveUsers[currentSocketID], sessionToLeave) != NULL) {
				char * token = strtok(sessActiveUsers[currentSocketID], ",");
				while (token != NULL) {
					if (strstr(sessionToLeave, token) == NULL) {
						strcat(sessionsLeftover, token);
						strcat(sessionsLeftover, ",");
					}
					token = strtok(NULL, ",");
				}
				

				if (strcmp(sessionsLeftover, "") == 0) {
					strcpy(sessActiveUsers[currentSocketID], "main");
				}
				else {
					strcpy(sessActiveUsers[currentSocketID],sessionsLeftover);
				}
				printf("Sessions youre still in: ", sessActiveUsers[currentSocketID]);

				send(newSocket[currentSocketID], "\033[0;33mYou left the session\033[0m\n", strlen("\033[0;33mYou left the session\033[0m\n"), 0);
			}
			else {
				send(newSocket[currentSocketID], "\033[0;31mYou are not in that session\033[0m\n", strlen("\033[0;31mYou are not in that session\033[0m\n"), 0);
			}
		}

		else if (strstr(buffer, "/leaveallsessions") != NULL) {
			strcpy(sessActiveUsers[currentSocketID],"main");
			send(newSocket[currentSocketID], "\033[0;33mYou left all sessions\033[0m\n", strlen("\033[0;33mYou left all sessions\033[0m\n"), 0);
		}

		else if (strstr(buffer, "/list") != NULL) {
			char activeSend[500] ={};
			strcpy(activeSend,"\n\033[0;33m---Active rooms:---\033[0m\n");
			for(int i = 0; i < MAXCONNECTIONS; i++){
				strcat(activeSend,sessActive[i]);
			}
			strcat(activeSend,"\n\033[0;33m---Active users:---\033[0m\n");
			for(int i = 0; i < MAXCONNECTIONS; i++){
				strcat(activeSend,sessActiveUsernamesDisplay[i]);
				if (strcmp(sessActiveUsernames[i], "") != 0) {
					strcat(activeSend, ": | ");
					char sessions[500] ={};
					strcpy(sessions, sessActiveUsers[i]);
					char * token = strtok(sessions, ",\n");
					while (token != NULL) {
						strcat(activeSend, token);
						strcat(activeSend, " | ");
						token = strtok(NULL, ",\n");
					}
					strcat(activeSend, "\n");
				}
			}
			send(newSocket[currentSocketID], activeSend, strlen(activeSend), 0);
		}
		else if (strstr(buffer, "/help") != NULL) {
			char commands[500];
			strcpy(commands, "\033[0;33mList of valid commands: \n /help \n /createsession <roomname> \n /joinsession <roomname> \n /leavesession <roomname> \n /leaveallsessions \n /quit \n /logout \n /list \n /colour <red, green, yellow, blue, purple, cyan, white> \n /time \n /invite <name> <roomname>\033[0m\n");
			send(newSocket[currentSocketID], commands, strlen(commands), 0);
		}
		else if (strstr(buffer, "/colour") != NULL) {
			char colour[100];
			char colouredName[100];
			strcpy(colour, buffer + 8);
			
			if (strstr(buffer, "red\n") != NULL) {
				strcpy(colouredName, "\033[0;31m");
				strcat(colouredName, sessActiveUsernames[currentSocketID]);
				strcat(colouredName, "\033[0m");
				strcpy(sessActiveUsernamesDisplay[currentSocketID], colouredName);
			}

			else if (strstr(buffer, "green\n") != NULL) {
				strcpy(colouredName, "\033[0;32m");
				strcat(colouredName, sessActiveUsernames[currentSocketID]);
				strcat(colouredName, "\033[0m");
				strcpy(sessActiveUsernamesDisplay[currentSocketID], colouredName);
			}

			else if (strstr(buffer, "yellow\n") != NULL) {
				strcpy(colouredName, "\033[0;33m");
				strcat(colouredName, sessActiveUsernames[currentSocketID]);
				strcat(colouredName, "\033[0m");
				strcpy(sessActiveUsernamesDisplay[currentSocketID], colouredName);
			}

			else if (strstr(buffer, "blue\n") != NULL) {
				strcpy(colouredName, "\033[0;34m");
				strcat(colouredName, sessActiveUsernames[currentSocketID]);
				strcat(colouredName, "\033[0m");
				strcpy(sessActiveUsernamesDisplay[currentSocketID], colouredName);
			}

			else if (strstr(buffer, "purple\n") != NULL) {
				strcpy(colouredName, "\033[0;35m");
				strcat(colouredName, sessActiveUsernames[currentSocketID]);
				strcat(colouredName, "\033[0m");
				strcpy(sessActiveUsernamesDisplay[currentSocketID], colouredName);
			}

			else if (strstr(buffer, "cyan\n") != NULL) {
				strcpy(colouredName, "\033[0;36m");
				strcat(colouredName, sessActiveUsernames[currentSocketID]);
				strcat(colouredName, "\033[0m");
				strcpy(sessActiveUsernamesDisplay[currentSocketID], colouredName);
			}

			else if (strstr(buffer, "white\n") != NULL) {
				strcat(colouredName, sessActiveUsernames[currentSocketID]);
				strcpy(sessActiveUsernamesDisplay[currentSocketID], colouredName);
			}
		}

		else if (strstr(buffer, "/time") != NULL) {
			char serverTime[100] ={};
			time_t rawtime;
			struct tm * timeinfo;

			time ( &rawtime );
			timeinfo = localtime ( &rawtime );
			strcpy(serverTime, "\033[0;33mCurrent local time and date: ");
			strcat(serverTime, asctime (timeinfo));
			strcat(serverTime, "\033[0m");

			send(newSocket[currentSocketID], serverTime, strlen(serverTime), 0);
		}

		else if (strstr(buffer, "/invite") != NULL) {
			int spaceCount = 0;
			char name[100] = {};
			char session[100] = {};
			int nameCount = 0;
			int sessionCount = 0;

			for (int i = 0; i < sizeof(buffer); i++) {
				if (buffer[i] == ' ') {
					spaceCount++;
					continue;
				}
				if (spaceCount == 1) {
					name[nameCount] = buffer[i];
					nameCount++;
				}
				else if (spaceCount == 2) {
					session[sessionCount] = buffer[i];
					sessionCount++;
				}
			}
			bool sessExists = false;

			for (int i = 0; i < MAXCONNECTIONS; i++) {
				if (strstr(sessActive[i], session) != NULL) {
					sessExists = true;
					break;
				}
			}
			if (!sessExists) {
				send(newSocket[currentSocketID], "\033[0;31mSession DNE\033[0m\n", strlen("\033[0;31mSession DNE\033[0m\n"), 0);
			} else {
				bool userExists = false;
				char inviteMsg[500] = {};
				strcpy(inviteMsg, "\033[0;33mYou have been invited to \033[0m\033[0;36m");
				strcat(inviteMsg, session);
				strcat(inviteMsg, "\033[0m \033[0;33mUse\033[0m \033[0;31m/deny\033[0m \033[0;33mor\033[0m \033[0;32m/accept ");
				strcat(inviteMsg, session);
				strcat(inviteMsg, "\033[0m");

				for (int i = 0; i < MAXCONNECTIONS; i++) {
					//printf("%s %s\n", sessActiveUsernames[i], name);
					if (strstr(sessActiveUsernames[i], name) != NULL) {
						send(newSocket[i], inviteMsg, strlen(inviteMsg), 0);
						send(newSocket[currentSocketID], "\033[0;33mInvite sent\033[0m\n", strlen("\033[0;33mInvite sent\033[0m\n"), 0);
						userExists = true;
					}
				}
				
				if(!userExists){
					send(newSocket[currentSocketID], "\033[0;31mUser DNE\033[0m\n", strlen("\033[0;31mUser DNE\033[0m\n"), 0);
				}
			}
		}
			
		else if (strstr(buffer, "/deny") != NULL) {
			send(newSocket[currentSocketID], "\033[0;33mInvitation denied\033[0m\n", strlen("\033[0;33mInvitation denied\033[0m\n"), 0);
		}

		else if (strstr(buffer, "/") != NULL) {
			send(newSocket[currentSocketID], "\033[0;31mNot a valid command, try /help\033[0m\n", strlen("\033[0;31mNot a valid command, try /help\033[0m\n"), 0);
		}
		
		else{
			// printf("Client %d: %s\n", currentSocketID, buffer);
			printf("%s: %s\n", sessActiveUsernamesDisplay[currentSocketID], buffer);
			for(int i = 0; i < activeConnections; i++){
				char sessions[500] ={};
				strcpy(sessions, sessActiveUsers[i]);
				char * token = strtok(sessions, ",");
				while (token != NULL) {
					if(newSocket[i]!=newSocket[currentSocketID] && strstr(sessActiveUsers[currentSocketID], token)!=NULL) {
						char bufferCombined[500]={};
						strcpy(bufferCombined, sessActiveUsernamesDisplay[currentSocketID]);
						strcat(bufferCombined, ": ");
						strcat(bufferCombined, buffer);
						send(newSocket[i], bufferCombined, strlen(bufferCombined), 0);
						break;
					}
					token = strtok(NULL, ",");
				}

			}
		}
	}
}