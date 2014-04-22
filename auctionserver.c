/*
 * auctionserver.c
 *
 *  Created on: Mar 20, 2014
 *      Author: frankie
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "singlyLinkedList.h"
#include "EE450.h"

//#define DEBUGFORK

#define MAXUSER 4
#define REG_TXT_LINE_LEN 33		//max length of a line in Registration.txt, NAME_MAX_LEN + PW_MX_LEN + ACCOUNT_NUM_MAX_LEN + 3spaces
#define BACKLOG 10	 // how many pending connections queue will hold

int g_userIndex = 0;	//global variable, indicate the index of user, such as user1, user2, etc

/*
 * read Registration.txt and parse all information then store them
 * into the struct list. At the end, close file.
 * return 0 if successful, otherwise return 1.
 */
int readRegistration(const char *filename, struct singlyLinkedList *list){
	FILE *fp = fopen(filename, "r");
	char buffer[REG_TXT_LINE_LEN] = {0};
	char *p = buffer;

	if (fp==NULL) return 1;

	/*read a line from Registration.txt and parse it*/
	while (fgets(buffer, REG_TXT_LINE_LEN, fp) != NULL){
		p = buffer;
		struct userNode *newObj = malloc(sizeof(struct userNode));	//construct a new data node
		memset(newObj, 0, sizeof(struct userNode));

		p = strtok(buffer, " ");
		strcpy (newObj->name,p);
		p = strtok(NULL, " ");
		strcpy (newObj->password,p);
		p = strtok(NULL, "\n");
		if (strlen(p) == 9 && strncmp(p,"4519",4) == 0)
			strcpy (newObj->accountNum,p);
		else{
			fprintf(stderr,"%s %s %s %s\n","Wrong Bank Account:",newObj->name,newObj->password,p);
			return 1;
		}

		if (listAppend(list, (void *)newObj) != 0) return 1;	//append newNode to the list
	}

	fclose(fp);
	return 0;
}

//if each field matches between user_1 and user_2, return 0; otherwise, return 1;
int compareUser(struct userNode *user_1, struct userNode *user_2){
	if (user_1 == NULL || user_2 == NULL) return 1;
	else return (strcmp(user_1->name, user_2->name) ||
				 strcmp(user_1->password, user_2->password) ||
		         strcmp(user_1->accountNum, user_2->accountNum));
}

/*
 * char* processLogin(char* buf, struct singlyLinkedList *reg_list, struct singlyLinkedList *accept_list);
 * usage:	deal with the Login# command. If the user information matches any user in reg_list,
 * 			copy it into accept_list, return "Accepted#"
 * input: 	char* buf = "Login#type userIndex username password bankaccount"
 * output:	 1. "Accepted#", if successful;
 * 		  	 2. "Rejected#", if unsuccessful;
 */
char* processLogin(char* buf, struct acceptedUserNode* newUser, struct singlyLinkedList *reg_list, struct singlyLinkedList *accept_list){
	char *str = NULL, *accept = "Accepted#", *reject = "Rejected#";
//	struct userNode *newUser = malloc(sizeof(struct userNode));
//	memset(newUser,0,sizeof(struct userNode));
	#ifdef DEBUG
		puts("bp:processLogin:");
	#endif
		if(strcmp(str = strtok(buf,"#"), "Login") == 0){
	#ifdef DEBUG
		puts(str);
	#endif
		newUser->type = atoi(strtok(NULL," "));
		newUser->userIndex = atoi(strtok(NULL," "));
		strcpy(newUser->name,strtok(NULL," "));
		strcpy(newUser->password,strtok(NULL," "));
		str = strtok(NULL, "\n");
//		printf("Phase 1: Authentication request. User%d: Username: %s Password: %s Bank Account: %s ",
//				++g_userIndex, newUser->name, newUser->password, str);
		if (strlen(str) == 9 && strncmp(str,"4519",4) == 0)
			strcpy (newUser->accountNum,str);
		else{
			fprintf(stderr,"Wrong Bank Account: Login#%d %d %s %s %s\n",newUser->type,newUser->userIndex,newUser->name,newUser->password,str);
			return reject;
		}
	}
	struct userNode *sameNameUser = (struct userNode *)(listSearch(reg_list, findByName, (void*)(newUser->name))->obj);
	if (sameNameUser != NULL){
		if (compareUser((struct userNode *)newUser, sameNameUser)){
			fprintf(stderr,"No matching user: Login#%s %s %s\n",newUser->name,newUser->password,str);
			return reject;
		}else{
			if (listAppend(accept_list, (void*)newUser)){
				fprintf(stderr,"Cannot save accetped user: Login#%s %s %s\n",newUser->name,newUser->password,str);
				return reject;
			}else return accept;
		}
	}else return reject;
}

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void){

	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	int numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	struct singlyLinkedList *reg_list, *accept_list;

	reg_list = malloc(sizeof(struct singlyLinkedList));	//registration list, contain all users info in Registration.txt
	memset(reg_list, 0, sizeof(struct singlyLinkedList));
	accept_list = malloc(sizeof(struct singlyLinkedList));	// contain all users accepted
	memset(accept_list, 0, sizeof(struct singlyLinkedList));


	if (readRegistration("Registration.txt", reg_list) != 0){	//read Registration.txt and load user information
		perror("readRegistration");
		return 1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;	//ipv4
	hints.ai_socktype = SOCK_STREAM;	//TCP socket

	if ((rv = getaddrinfo(HOSTNAME, PORT_S_P1, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}


	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
				perror("server: socket");
				continue; }
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
	            perror("setsockopt");
	            exit(1); }
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue; }
		break;
	}

	if (p == NULL){
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}


	inet_ntop(p->ai_family,	get_in_addr(p->ai_addr),
				s, sizeof s);
	printf("Phase 1: Auction server has TCP port number %d and IP address %s\n",
			((struct sockaddr_in*)(p->ai_addr))->sin_port, s);
//	printf("server IP %s Port:%d\n", s, ((struct sockaddr_in*)(p->ai_addr))->sin_port);
	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

//	sa.sa_handler = sigchld_handler; // reap all dead processes
//	sigemptyset(&sa.sa_mask);
//	sa.sa_flags = SA_RESTART;
//	if (sigaction(SIGINT, &sa, NULL) == -1) {
//		perror("sigaction");
//		exit(1);
//	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

//#ifndef DEBUGFORK
//		if (!fork()) { // this is the child process
//			close(sockfd); // child doesn't need the listener
//#endif
		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
	#ifdef DEBUG
		printf("recv: %s\n",buf);
//			puts("press Enter key..");
//			getchar();
	#endif
		//04/15 03:47 continue to work here, gonna develop codes deal with the Login# command
		struct acceptedUserNode *newUser = malloc(sizeof(struct acceptedUserNode));
		memset(newUser,0,sizeof(struct acceptedUserNode));
		strcpy(buf,processLogin(buf, newUser, reg_list, accept_list));	//process Login# command, and put the response command to buf
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
//		printf("server: got connection from %s Port:%d\n", s, ((struct sockaddr_in*)(&their_addr))->sin_port);
		printf("Phase 1: Authentication request. User%d: Username:%s Password:%s "
				"Bank Account:%s User IP Addr:%s Authorized:%s\n",
				++g_userIndex, newUser->name, newUser->password, newUser->accountNum,
				s, buf);
		if(!strcmp(buf, "Accepted#")) strcpy(newUser->ip_addr, s);
//		printf("User IP Addr: %s. Authorized: %s\n", s, buf);
	#ifdef DEBUG
		puts(buf);
	#endif
		if (send(new_fd, buf, MAXDATASIZE-1, 0) == -1)
			perror("send");
	#ifdef DEBUG
		printf("send: %s\n",buf);
	#endif
//		#ifndef DEBUGFORK
//			close(new_fd);
//			exit(0);
//
//		}
//		#endif
		close(new_fd);
	}

	return 0;


}

