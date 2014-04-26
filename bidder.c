/*
 * bidder.c
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

// get sockaddr,
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//read bidderPass.txt
int readBidderPass(int bidderIndex, const char *filename, struct userNode **node){
	FILE *fp = fopen(filename, "r");
	char buffer[PASS_TXT_LINE_LEN] = {0};
	char *p = buffer;

	if (fp==NULL) return 1;

	/*read a line from bidderPass.txt and parse it*/
	if (fgets(buffer, PASS_TXT_LINE_LEN, fp) != NULL){

		p = buffer;

		p = strtok(buffer, " ");
		if (((*node)->type = atoi(p)) != 1){
			fprintf(stderr,"Not a bidder: %d %s %s %s\n",(*node)->type,(*node)->name,(*node)->password,p);
			return 1;
		}
		(*node)->userIndex = bidderIndex;
		p = strtok(NULL, " ");
		strcpy ((*node)->name,p);
		p = strtok(NULL, " ");
		strcpy ((*node)->password,p);
		p = strtok(NULL, "\n");
		strcpy ((*node)->accountNum,p);
//		if (strlen(p) == 9 && strncmp(p,"4519",4) == 0)
//			strcpy ((*node)->accountNum,p);
//		else{
//			fprintf(stderr,"Wrong Bank Account: %s %s %s\n",(*node)->name,(*node)->password,p);
//			return 1;
//		}

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
	char s[INET_ADDRSTRLEN];
	char* header;
	struct sockaddr_in sa;	//store local address
	int sa_len = sizeof(sa);
	FILE *fp;

	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;

	int cpid;

	struct userNode* bidderInfo;
	bidderInfo = malloc(sizeof(struct userNode));
	memset(bidderInfo,0,sizeof(struct userNode));

	/*phase 1: Authorization*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(HOSTNAME, PORT_S_P1, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	cpid = fork();
	if(cpid){
		//parent process
		if (readBidderPass(2, "bidderPass2.txt", &bidderInfo) != 0){	//read bidderpass1.txt and load user information
				perror("bidderPass1.txt");
				return 1;
			}
		header = "Bidder2";
		sleep(2);	//parent sleep 2s, wait until child finished
	}else{
		//child process
		if (readBidderPass(1, "bidderPass1.txt", &bidderInfo) != 0){	//read bidderpass1.txt and load user information
						perror("bidderPass1.txt");
						return 1;}
		header = "Bidder1";
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

	//Phase 1: <Bidder#>__ has TCP port ___ and IP address: ____
	getsockname(sockfd, (struct sockaddr*)&sa, &sa_len);
	inet_ntop(AF_INET, (void*)&(sa.sin_addr), s, sizeof s);

	if(cpid){
		//parent process
		printf("Phase 1: <Bidder2> has TCP port %d and IP address: %s\n",	sa.sin_port, s);
	}else{
		//child process
		printf("Phase 1: <Bidder1> has TCP port %d and IP address: %s\n",	sa.sin_port, s);
	}

#ifdef DEBUG
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s Port:%d\n", s, ((struct sockaddr_in*)(p->ai_addr))->sin_port);
#endif
	freeaddrinfo(servinfo); // all done with this structure

	//Login cammand: "Login#type userIndex username password bankaccount"
	sprintf(buf,"Login#%d %d %s %s %s",bidderInfo->type, bidderInfo->userIndex,
			bidderInfo->name, bidderInfo->password, bidderInfo->accountNum);
	printf("Phase 1: Login request. User:%s password:%s Bank account:%s\n",
			bidderInfo->name, bidderInfo->password, bidderInfo->accountNum);
#ifdef DEBUG
	puts(buf);
#endif
	//send Login command to server
	addheader(buf, header);
	if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		    perror("send");
		    exit(1);
		}
	sleep(1); //sleep 1 second

	//receive Accepted# or Rejected# command from server
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}
	buf[numbytes] = '\0';
	removeheader(buf);
	printf("Phase 1: Login request reply: %s .\n\n", buf);
//	printf("client: received '%s'\n",buf);

//	while ((recv(sockfd, buf, MAXDATASIZE-1, 0)) != 0);	//wait until server close(sockfd), phase 1
	close(sockfd);
	/*End of phase 1*/
	if(!strcmp(buf, "Rejected#")) return 0;	//if rejected, bidder shouldn't appear in following phases

	/*************************************************************************************************/
	/*phase 3: Auction*/
#ifdef DEBUG
	puts("Phase 3");
#endif
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;	//ipv4
	hints.ai_socktype = SOCK_DGRAM;	//UDP socket
	gethostname(buf, MAXDATASIZE-1);	//use buf to store hostname temporarily

	if ((rv = getaddrinfo(buf, (!cpid)?PORT_BD1_P3_UDP:PORT_BD2_P3_UDP, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue; }
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	sleep(15); //wait for server begin phase 3

	//Phase 3: <Bidder#>__ has UDP port ___ and IP address: ____
	getsockname(sockfd, (struct sockaddr*)&sa, &sa_len);
	inet_ntop(AF_INET, (void*)&(sa.sin_addr), s, sizeof s);
	if(cpid){
		//parent process
		sleep(5);	// parent/bidder2 sleep 5s, wait until child finished
		printf("Phase 3: <Bidder2> has UDP port %d and IP address: %s\n",	sa.sin_port, s);
		puts("Phase 3: <Bidder2> (Item list displayed here)");
	}else{
		//child process
		printf("Phase 3: <Bidder1> has UDP port %d and IP address: %s\n",	sa.sin_port, s);
		puts("Phase 3: <Bidder1> (Item list displayed here)");
	}
	freeaddrinfo(servinfo);

	//receive broadcastList
	do{
		if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
				(struct sockaddr *)&their_addr, &sin_size)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		buf[numbytes] = '\0';
#ifdef DEBUG
		printf("listener: got packet from %s\n",
			   inet_ntop(their_addr.ss_family,
				   get_in_addr((struct sockaddr *)&their_addr),
				   s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);
		printf("listener: packet contains \"%s\"\n", buf);
#endif
		removeheader(buf);
		if(!strcmp("ListEnd#",buf)) break;
		puts(buf);
	}while(1);


	sleep(5);	//wait for server open UDP recv

	/*send bidding info*/
	if(cpid){
		//parent process
		if((fp = fopen("bidding2.txt", "r")) == NULL){
				perror("bidding2.txt");
				exit(1);
			}
		puts("Phase 3: <Bidder2> (Bidding information displayed here)");
	}else{
		//child process
		if((fp = fopen("bidding1.txt", "r")) == NULL){
				perror("bidding1.txt");
				exit(1);
		}
		puts("Phase 3: <Bidder1> (Bidding information displayed here)");
	}
	//read one line and send per loop
	while(fgets(buf, sizeof(buf), fp) != NULL){
		buf[strlen(buf)-1] = '\0';		//remove '\n' in the end
		addheader(buf, header);
		if ((numbytes = sendto(sockfd, buf, strlen(buf), 0,
				(struct sockaddr*)&their_addr,
				sizeof(struct sockaddr_in))) == -1) {
						perror("talker: sendto");
						exit(1);
					}
#ifdef DEBUG
		printf("talker: sent %d bytes to server\n", numbytes);
		puts(buf);
#endif
		removeheader(buf);
		puts(buf);
	}
	//indicate end of file
	strcpy(buf, "ListEnd#");
	addheader(buf, header);
	if ((numbytes = sendto(sockfd, buf, strlen(buf), 0,
			(struct sockaddr*)&their_addr,
			sizeof(struct sockaddr_in))) == -1){
				perror("talker: sendto");
				exit(1);
			}
#ifdef DEBUG
	puts(buf);
#endif



	return 0;
}
