#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#define MAXBUF 1024
#define MAXHOSTNAME 200
#define MAXACTION  10
#define MAXPORT 6
#define logFile "log.txt"

struct agents{
	char agent[MAXHOSTNAME];
	time_t join_time;
	struct agents* nextAgent;
};

struct agents* head = NULL;

void GetTime(char* time, time_t &actionT);

int main(int argc, char* argv[]){
	int portNum,sockfd, agentfd, numRead,numWrite, fileLength = 0; //file descriptors, bytes read/write
	time_t actionT; // Time of action
	bool isAgent;
	socklen_t addrlen;
	char buffer[MAXBUF], 
		 cServer[MAXHOSTNAME],
		 cPort[MAXPORT],
		 cAction[MAXACTION],
		 agentIP[MAXHOSTNAME],
		 agentResponse[20], //respone
		 time[100];

	// Error check
	if(argc < 2){
		fprintf(stderr, "Usage: server_ip server_port, closing server\n");
		exit(1);
	}

	// generate sockfd
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "ERROR: Socket() failed, closing server\n");
		exit(1);
	}

	// set port
	memset(cPort, 0, MAXPORT);
	memset(cServer, 0, MAXHOSTNAME);
	//sprintf(cServer, "%s", argv[1]);
  	sprintf(cPort,"%s",argv[1]);
  	portNum = atoi(argv[1]);


  	// bind()
  	struct sockaddr_in my_addr, agent_addr;
  	my_addr.sin_family = AF_INET;
  	my_addr.sin_port = htons(portNum);
  	my_addr.sin_addr.s_addr = INADDR_ANY;
  	//my_addr.sin_addr.s_addr = inet_addr(cServer);

  	if(bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0){
  		fprintf(stderr, "ERROR: Bind() failed,  closing server\n");
  		exit(1);
  	}

  	// 0 out server and action
  	//memset(cServer, 0, MAXHOSTNAME);
  	memset(cAction, 0, MAXACTION);
  	inet_ntop(AF_INET, &my_addr.sin_addr.s_addr, cServer, sizeof(my_addr));
  	// listen()
  	if(listen(sockfd, MAXACTION) < 0){
  		fprintf(stderr, "ERROR: Listen() failed, closing server\n");
  		exit(1);
  	}


	printf("Server running on: %s\nUsing port       : %d\n", cServer, portNum);

	GetTime(time, actionT);
  	printf("Current time     : %s\n", time);

  	bool flag = true;
  	while(flag){
  		// log
  		FILE *fptr = fopen(logFile, "a");
  		if(logFile == NULL){
  			fprintf(stderr, "ERROR: log.txt failed to open, closing server\n");
  			exit(1);
  		}

  		bzero(buffer, MAXBUF);

  		// accept() agent
  		addrlen = sizeof(agent_addr);
  		if((agentfd = accept(sockfd, (struct sockaddr*) &agent_addr, &addrlen)) < 0){
  			fprintf(stderr, "ERROR: Agent accept failed, closing server\n");
  			exit(1);
  		}

  		inet_ntop(AF_INET, &my_addr.sin_addr.s_addr, agentIP, sizeof(agent_addr));

  		// read() from agent
  		if((numRead = read(agentfd, buffer, MAXBUF)) < 0){
  			fprintf(stderr, "ERROR: Failed to read data from agent, closing server\n");
  			exit(0);
  		}

  		GetTime(time, actionT);

  		memset(cAction,0,MAXACTION);
  		sprintf(cAction,"%s", buffer);

  		fprintf(fptr, "%s : Received a %s action from agent %s\n", time , cAction, agentIP);
  		
  		struct agents *ptr;
  		ptr = head;
  		isAgent = false;
  		while(ptr != NULL && isAgent == false){
  			if((strncmp(ptr->agent, agentIP, MAXHOSTNAME)) == 0){
  				isAgent = true;
  			}
  			ptr = ptr-> nextAgent;
  		}

  		// Add member to list
  		if((strncmp(cAction, "#JOIN", MAXACTION)) == 0){
  			fileLength++;
  			GetTime(time, actionT);
  			printf("%s request from agent %s at %s\n", cAction, agentIP, time);
  			if(isAgent == true){
  				sprintf(agentResponse, "$ALREADY MEMBER");
  				numWrite = write(agentfd, agentResponse, strlen(agentResponse));

  				fprintf(fptr, "%s : Responded to agent %s with %s\n", time , agentIP, agentResponse);
  			}
  			else{
  				sprintf(agentResponse, "$OK");
  				numWrite = write(agentfd, agentResponse, strlen(agentResponse));

 				if(head == NULL){
 					head = (agents*)malloc(sizeof(struct agents));
 					strncpy(head->agent, agentIP, MAXHOSTNAME);
 					head->join_time = actionT;
 					head->nextAgent = NULL;
 				}
 				else{
 					struct agents *tempPtr1, *tempPtr2;
 					tempPtr2 = head;
 					tempPtr1 = (agents*)malloc(sizeof(struct agents));
 					strncpy(tempPtr1->agent, agentIP, MAXHOSTNAME);
 					tempPtr1->join_time = actionT;
 					tempPtr1->nextAgent = tempPtr2;
 					head = tempPtr1;
 				}
 				fprintf(fptr, "%s : Responded to agent %s with %s\n", time , agentIP, agentResponse);
  			}
 			fclose(fptr);
  		}

  		if((strncmp(cAction, "#LEAVE", MAXACTION)) == 0){
  			fileLength++;
  			GetTime(time, actionT);
  			printf("%s request from agent %s at %s\n", cAction, agentIP, time);
  			if(isAgent == true){
  				sprintf(agentResponse, "$OK");
  				numWrite = write(agentfd, agentResponse, strlen(agentResponse));
  				fprintf(fptr, "%s : Responded to agent %s with %s\n", time, agentIP, agentResponse);

  				struct agents *tempPtr1, *tempPtr2;
  				tempPtr1 = head;

  				if((strncmp(tempPtr1->agent, agentIP, MAXHOSTNAME)) == 0){
  					head = head->nextAgent;
  					free(tempPtr1);
  				}
  				else{
  					bool del = false;
  					tempPtr2 = head->nextAgent;

  					while(tempPtr2 != NULL && tempPtr1 != NULL && del == false){
  						if((strncmp(tempPtr2->agent, agentIP, MAXHOSTNAME)) == 0){
  							struct agents *tempPtr3;
  							tempPtr3 = tempPtr2;
  							tempPtr2 = tempPtr2->nextAgent;
  							tempPtr1->nextAgent = tempPtr2;
  							free(tempPtr3);
  							del = true;
  						}
  						else{
  							tempPtr2 = tempPtr2->nextAgent;
  							tempPtr1 = tempPtr1->nextAgent;
  						}
  					}

  					if(tempPtr1 != NULL && tempPtr2 == NULL){
  						if((strncmp(tempPtr1->agent, agentIP, MAXHOSTNAME)) == 0) {free(tempPtr1);}
  					
  					}
  				}
  			}
  			else{
  				sprintf(agentResponse, "$NOT MEMBER");
  				numWrite = write(agentfd, agentResponse, strlen(agentResponse));
  				fprintf(fptr, "%s : Responded to agent %s with %s\n", time , agentIP, agentResponse);

  			}
  			fclose(fptr);
  		}

  		if((strncmp(cAction, "#LIST", MAXACTION)) == 0){
  			fileLength++;
  			GetTime(time, actionT);
  			printf("%s request from agent %s at %s\n", cAction, agentIP, time);
  			if(isAgent == false){
				sprintf(agentResponse, "$NOT MEMBER");
  				numWrite = write(agentfd, agentResponse, strlen(agentResponse));
  				fprintf(fptr, "%s : No response is supplied to agent %s (agent not active)\n", time, agentIP);
  				fclose(fptr);
  			}
  			else{
  				time_t timeAlive = actionT - head->join_time;
				char tempArr[MAXBUF];
  				sprintf(agentResponse, "$OK");
  				numWrite = write(agentfd, agentResponse, strlen(agentResponse));

  				memset(buffer, 0, MAXBUF);
  				struct agents *temp = head;
  				//int i;
  				char tAgent[MAXHOSTNAME];
  				while(temp != NULL){
  					strncpy(tAgent, temp->agent, MAXHOSTNAME);
  					temp = temp->nextAgent;
  					timeAlive = actionT - head->join_time;
  					strcpy(buffer, "\n$ <");
  					strcat(buffer, tAgent);
  					strcat(buffer, ", ");
  					sprintf(tempArr, "%li", timeAlive);
  					strcat(buffer, tempArr);
  					strcat(buffer, ">");
  					write(agentfd, buffer, strlen(buffer));
  				}
  				printf("\n");
  				fprintf(fptr, "%s : Responded to agent %s with %s\n", time , agentIP, agentResponse);
  				free(temp);
  			}
  		}

  		if((strncmp(cAction, "#LOG", MAXACTION)) == 0){
  			fileLength++;
  			GetTime(time, actionT);
  			printf("%s request from agent %s at %s\n", cAction, agentIP, time);
  			
  			if(isAgent == false){
  				sprintf(agentResponse, "$NOT MEMBER");
  				numWrite = write(agentfd, agentResponse, strlen(agentResponse));
  				fprintf(fptr, "%s : No response is supplied to agent %s (agent not active)\n", time, agentIP);
  				fclose(fptr);
  			}
  			else{
  				memset(buffer, 0, MAXBUF);
  				sprintf(agentResponse, "log file");
  				//numWrite = write(sockfd, agentResponse, strlen(agentResponse));
  				fprintf(fptr, "%s : Responded to agent %s with %s\n", time, agentIP, agentResponse);
  				fclose(fptr);
  				fptr = fopen(logFile, "r");
  				
  				// while(fileLength > 0){
  				// 	if(fgets(buffer, 100, fptr) != NULL){
  				// 		numWrite = write(agentfd, buffer, strlen(buffer));
  				// 	}	
  				// }
  				numRead = fread(buffer, sizeof(char), sizeof(buffer), fptr);

  				if(numRead == 0){
  					printf("log.txt is empty\n");
  				}
  				else if(numRead < 0){
  					fprintf(stderr, "ERROR: log.txt failed to read into buffer\n");
  				}
  				else if(numRead > 0){
  					char *temp = buffer;

  					while(numRead > 0){
  						numWrite = write(agentfd, buffer, numWrite);
  						if(numWrite <= 0){
  							fprintf(stderr, "ERROR: log.txt failed to write to buffer\n");
  						}

  						numRead -= numWrite;
  						temp+= numWrite;
  					}
  				}
  				fclose(fptr);
  			}
  		}
  	close(agentfd);
  	} // End of While loop
  	close(sockfd);
  	//fclose(fptr);


	return 0;
}

void GetTime(char * time, time_t &actionT){
	//time_t startTime;
	struct tm* tm_info;
	struct timeval tv;
	int millisec;
	char buf[50];
	gettimeofday(&tv, NULL);
	millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
	if (millisec>=1000) { // Allow for rounding up to nearest second
		millisec -=1000;
	    tv.tv_sec++;
	}

	//printf("%d\n", millisec);
  	tm_info = localtime(&tv.tv_sec);
  	strftime(buf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  	//printf("%s\n", buf);
  	strcpy(time, buf);
  	sprintf(buf, ":%d", millisec);
  	strcat(time, buf);
  	//printf("%s", time);
  	actionT = (unsigned long long)(tv.tv_sec)*1000 + millisec;
  }