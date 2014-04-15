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

#define MAXUSER 4

#define REG_TXT_LINE_LEN 33		//max length of a line in Registration.txt, NAME_MAX_LEN + PW_MX_LEN + ACCOUNT_NUM_MAX_LEN + 3spaces

#define BACKLOG 10	 // how many pending connections queue will hold


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
	reg_list = malloc(sizeof(struct singlyLinkedList));
	memset(reg_list, 0, sizeof(struct singlyLinkedList));
	accept_list = malloc(sizeof(struct singlyLinkedList));
	memset(accept_list, 0, sizeof(struct singlyLinkedList));

	if (readRegistration("Registration.txt", reg_list) != 0){	//read Registration.txt and load user information
		perror("readRegistration");
		return 1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;	//don't care ipv4 or ipv6
	hints.ai_socktype = SOCK_STREAM;	//TCP socket
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(NULL, PORT_S_P1, &hints, &servinfo)) != 0) {
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

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
//			if (send(new_fd, "Hello, world!", 13, 0) == -1)
//				perror("send");
			if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
			    exit(1);
			}
		#ifdef DEBUG
			puts(buf);
		#endif
			//04/15 03:47 continue to work here, gonna develop codes deal with the login# commmand
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;


}

