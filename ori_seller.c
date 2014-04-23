/*
 * seller.c
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

#include "EE450.h"

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//read sellerPass.txt
int readSellerPass(const char *filename, struct userNode **node){
	FILE *fp = fopen(filename, "r");
	char buffer[PASS_TXT_LINE_LEN] = {0};
	char *p = buffer;

	if (fp==NULL) return 1;

	/*read a line from sellerPass.txt and parse it*/
	if (fgets(buffer, PASS_TXT_LINE_LEN, fp) != NULL){

		p = buffer;

		p = strtok(buffer, " ");
		if ((node->type = atoi(p)) != 2){
			fprintf(stderr,"Not a seller: %d %s %s %s\n",(*node)->type,(*node)->name,(*node)->password,p);
			return 1;
		}
		p = strtok(NULL, " ");
		strcpy ((*node)->name,p);
		p = strtok(NULL, " ");
		strcpy ((*node)->password,p);
		p = strtok(NULL, "\n");
		if (strlen(p) == 9 && strncmp(p,"4519",4) == 0)
			strcpy ((*node)->accountNum,p);
		else{
			fprintf(stderr,"%s %s %s %s\n","Wrong Bank Account:",(*node)->name,(*node)->password,p);
			return 1;
		}
	}
	else{
		fprintf(stderr,"Can't read any information from %s\n",filename);
		return 1;
	}

	fclose(fp);
	return 0;
}

int main(void)
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];


	/*phase 1: Authorization*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(HOSTNAME, PORT_S_P1, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);

	close(sockfd);

	return 0;
}
