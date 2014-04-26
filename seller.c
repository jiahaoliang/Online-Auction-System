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

// get sockaddr,
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//read sellerPass.txt
int readSellerPass(int sellerIndex, const char *filename, struct userNode **node){
	FILE *fp = fopen(filename, "r");
	char buffer[PASS_TXT_LINE_LEN] = {0};
	char *p = buffer;

	if (fp==NULL) return 1;

	/*read a line from sellerPass.txt and parse it*/
	if (fgets(buffer, PASS_TXT_LINE_LEN, fp) != NULL){

		p = buffer;

		p = strtok(buffer, " ");
		if (((*node)->type = atoi(p)) != 2){
			fprintf(stderr,"Not a seller: %d %s %s %s\n",(*node)->type,(*node)->name,(*node)->password,p);
			return 1;
		}
		(*node)->userIndex = sellerIndex;
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
	struct sockaddr_in sa;	//store local address
	int sa_len = sizeof(sa);
	char serverIP[INET6_ADDRSTRLEN];
	char port_S_P2[PORTNUM_LEN]= {0} ;	//variable to store server port number for phase 2
	char *header;
	FILE *fp = NULL;

	int cpid;

	struct userNode* sellerInfo;
	sellerInfo = malloc(sizeof(struct userNode));
	memset(sellerInfo,0,sizeof(struct userNode));

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
		if (readSellerPass(2, "sellerPass2.txt", &sellerInfo) != 0){	//read sellerpass1.txt and load user information
				perror("sellerPass1.txt");
				return 1;
			}
		header = "Seller2";
		sleep(2);	//parent sleep 2s, wait until child finished
	}else{
		//child process
		if (readSellerPass(1, "sellerPass1.txt", &sellerInfo) != 0){	//read sellerpass1.txt and load user information
						perror("sellerPass1.txt");
						return 1;}
		header = "Seller1";
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

	//Phase 1: <Seller#>__ has TCP port ___ and IP address: ____
	getsockname(sockfd, (struct sockaddr*)&sa, &sa_len);
	inet_ntop(AF_INET, (void*)&(sa.sin_addr), s, sizeof s);

	if(cpid){
		//parent process
		printf("Phase 1: <Seller2> has TCP port %d and IP address: %s\n",	sa.sin_port, s);
	}else{
		//child process
		printf("Phase 1: <Seller1> has TCP port %d and IP address: %s\n",	sa.sin_port, s);
	}

#ifdef DEBUG
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s Port:%d\n", s, ((struct sockaddr_in*)(p->ai_addr))->sin_port);
#endif
	freeaddrinfo(servinfo); // all done with this structure

	//Login cammand: "Login#type userIndex username password bankaccount"
	sprintf(buf,"Login#%d %d %s %s %s",sellerInfo->type, sellerInfo->userIndex,
			sellerInfo->name, sellerInfo->password, sellerInfo->accountNum);
	printf("Phase 1: Login request. User:%s password:%s Bank account:%s\n",
			sellerInfo->name, sellerInfo->password, sellerInfo->accountNum);
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
	printf("Phase 1: Login request reply: %s .\n", buf);

	if(!strcmp(buf, "Rejected#")){	//if rejected, seller shouldn't appear in following phases
		if(cpid){
			//parent process
			puts("End of Phase 1 for <Seller2>.");
		}else{
			//child process
			puts("End of Phase 1 for <Seller1>.");}
		close(sockfd);
		return 0;
	}

	//receive server IP
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}
	buf[numbytes] = '\0';
	removeheader(buf);
	strcpy(serverIP, buf);

	//receive server Port Number for phase 2
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}
	buf[numbytes] = '\0';
	removeheader(buf);
	strcpy(port_S_P2, buf);

	printf("Phase 1: Auction Server has IP Address:%s and PreAuction TCP Port Number:%s\n", serverIP, port_S_P2);

//	while ((recv(sockfd, buf, MAXDATASIZE-1, 0)) != 0);	//wait until server close(sockfd), phase 1 end.
	close(sockfd);

	if(cpid){
		//parent process
		puts("End of Phase 1 for <Seller2>.\n");
	}else{
		//child process
		puts("End of Phase 1 for <Seller1>.\n");
	}
	/*End of phase 1*/

	/**************************************************************************************************/
	/*phase 2: PreAuction*/
	sleep(5);	//wait until server finished phase 1
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(serverIP, port_S_P2, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	printf("Phase 2: Auction Server has IP Address:%s PreAuction Port Number:%s\n", serverIP, port_S_P2);

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


#ifdef DEBUG
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s Port:%d\n", s, ((struct sockaddr_in*)(p->ai_addr))->sin_port);
#endif
	freeaddrinfo(servinfo); // all done with this structure

	if(cpid){
		//parent process
		if ((fp = fopen("itemList2.txt", "r")) == NULL){	//open itemList2.txt
				perror("itemList2.txt");
				return 1;
			}
		sleep(2);
		strcpy(buf, "Phase 2: <Seller2> send item lists.");
		puts(buf);
	}else{
		//child process
		if ((fp = fopen("itemList1.txt", "r")) == NULL){	//open itemList1.txt
						perror("itemList1.txt");
						return 1;}
		strcpy(buf, "Phase 2: <Seller1> send item lists.");
		puts(buf);
	}
	addheader(buf, header);
	if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		    perror("send");
		    exit(1);
		}

	//read and send seller name
	fgets(buf, sizeof(buf), fp);
	buf[strlen(buf)-1] = '\0';		//remove '\n' in the end
	addheader(buf, header);
	if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		    perror("send");
		    exit(1);
		}
	removeheader(buf);
	puts(buf);

	//read one line and send per loop
	while(fgets(buf, sizeof(buf), fp) != NULL){
		buf[strlen(buf)-1] = '\0';		//remove '\n' in the end
		addheader(buf, header);
		if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("send");
			    exit(1);
			}
		removeheader(buf);
		puts(buf);
	}

	//indicate end of file
	strcpy(buf, "ListEnd#");
	addheader(buf, header);
	if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		    perror("send");
		    exit(1);
		}
#ifdef DEBUG
	puts(buf);
#endif

	if(cpid){
		//parent process
		puts("End of Phase 2 for <Seller2>.\n");
	}else{
		//child process
		puts("End of Phase 2 for <Seller1>.\n");
	}
	/*End of phase 2*/
	/**************************************************************************************************/
	return 0;
}
