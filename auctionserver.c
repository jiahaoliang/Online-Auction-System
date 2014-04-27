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
#define MAXSELLER 2
#define MAXBIDDER 2
#define REG_TXT_LINE_LEN 33		//max length of a line in Registration.txt, NAME_MAX_LEN + PW_MX_LEN + ACCOUNT_NUM_MAX_LEN + 3spaces

int g_userIndex = 0;	//global variable, indicate the index of user, such as user1, user2, etc

/*
 * read broadcastList.txt and parse all information then store them
 * into the struct list. At the end, close file.
 * listNode->obj = broadcastItemNode
 * return 0 if successful, otherwise return 1.
 */
int readBroadcastList(const char *filename, struct singlyLinkedList *list){
	FILE *fp = fopen(filename, "r");
	char buffer[MAXDATASIZE] = {0};
	char *p = buffer;

	if (fp==NULL) return 1;

	/*read a line from broadcastList.txt and parse it*/
	while (fgets(buffer, MAXDATASIZE, fp) != NULL){
		if(!strcmp(buffer, "\n")) break;	//two '\n' at the end of broadcastList.txt
		p = buffer;
		struct broadcastItemNode *newObj = malloc(sizeof(struct broadcastItemNode));	//construct a new data node
		memset(newObj, 0, sizeof(struct broadcastItemNode));

		p = strtok(buffer, " ");
		strcpy (newObj->name,p);
		p = strtok(NULL, " ");
		strcpy (newObj->itemName,p);
		p = strtok(NULL, "\n");
		newObj->minPrice = atoi(p);
		if (listAppend(list, (void *)newObj) != 0) return 1;	//append newNode to the list
	}

	fclose(fp);
	return 0;
}
/*
 * read Registration.txt and parse all information then store them
 * into the struct list. At the end, close file.
 * listNode->obj = userNode
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
		//store user info into struct
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

//void sigchld_handler(int s)
//{
//	while(waitpid(-1, NULL, WNOHANG) > 0);
//}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void){

	int sockfd, new_fd, udp_sock_fd[2];  // listen on sock_fd, new connection on new_fd
	int numbytes;
	char buf[MAXDATASIZE], *str;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sockaddr_in sa;	//store local address
	int sa_len = sizeof(sa);
	int yes=1;
	char s[INET6_ADDRSTRLEN], hostIP[INET6_ADDRSTRLEN];
	int i, rv;
	char* header = "Server";
	char bidderName[MAXBIDDER][NAME_MAX_LEN], sellerName[MAXSELLER][NAME_MAX_LEN]; //store bidders' name and sellers' name
	FILE *fp;

	struct singlyLinkedList *reg_list, *accept_list, //for phase 1
	*broadcast_list, *bidding_list[MAXBIDDER], *sold_list; //for phase 3

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
	gethostname(buf, MAXDATASIZE-1);	//use buf to store hostname temporarily

	if ((rv = getaddrinfo(buf, PORT_S_P1, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	/*phase 1: Authorization*/
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


	inet_ntop(p->ai_family,	get_in_addr(p->ai_addr), hostIP, sizeof hostIP);
	printf("Phase 1: Auction server has TCP port number %d and IP address %s\n",
			((struct sockaddr_in*)(p->ai_addr))->sin_port, hostIP);
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

	for(i=0;i<MAXUSER;i++) {  // main accept() loop
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
		//receive Login command from users
		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		removeheader(buf);
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
				++g_userIndex, newUser->name, newUser->password, newUser->accountNum, s, buf);
//		printf("User IP Addr: %s. Authorized: %s\n", s, buf);
	#ifdef DEBUG
		puts(buf);
	#endif
		//send Accepted# or Rejected# command to user
		addheader(buf, header);
		if (send(new_fd, buf, MAXDATASIZE-1, 0) == -1)
			perror("send");
		//Upon acceptance the server will
		//save the IP address of the accepted user and will bind it to its username for future reference.
		removeheader(buf);
		if(!strcmp(buf, "Accepted#")){
			strcpy(newUser->ip_addr, s);
			//if newUser is a seller, send the IP and PreAuction Port number to the it
			if(newUser->type == 2){
				//store its name for further use
				//userIndex begins with 1, sellerName begins with 0
				strcpy(sellerName[(newUser->userIndex)-1], newUser->name);
				//send IP address
				strcpy(buf, hostIP);
				addheader(buf, header);
				if (send(new_fd, buf, MAXDATASIZE-1, 0) == -1)
					perror("send");
				//send Port Number
				strcpy(buf, PORT_S_P2);
				addheader(buf, header);
				if (send(new_fd, buf, MAXDATASIZE-1, 0) == -1)
					perror("send");
				printf("Phase 1: Auction Server IP Address:%s "
						"PreAuction Port Number:%s sent to the <Seller%d>\n", hostIP, PORT_S_P2, newUser->userIndex);
			}else{
				//if it is a bidder, store its name for further use
				//userIndex begins with 1, bidderName begins with 0
				strcpy(bidderName[(newUser->userIndex)-1], newUser->name);
			}
		}

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
	puts("End of Phase 1 for Auction Server\n");
	close(sockfd);
	if(accept_list->num < MAXUSER) return 1;	//if any of user rejected, stop doing following phases
	/*End of phase 1*/

	/*************************************************************************************************/

	/*phase 2: PreAuction*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;	//ipv4
	hints.ai_socktype = SOCK_STREAM;	//TCP socket
	gethostname(buf, MAXDATASIZE-1);	//use buf to store hostname temporarily

	if ((rv = getaddrinfo(buf, PORT_S_P2, &hints, &servinfo)) != 0) {
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


	inet_ntop(p->ai_family,	get_in_addr(p->ai_addr), hostIP, sizeof hostIP);
	printf("Phase 2: Auction Server IP Address:%s PreAuction TCP Port Number:%d .\n",
			hostIP, ((struct sockaddr_in*)(p->ai_addr))->sin_port);
//	printf("server IP %s Port:%d\n", s, ((struct sockaddr_in*)(p->ai_addr))->sin_port);
	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	for(i=0;i<MAXSELLER;i++){

		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		// recv "Phase 2: <Seller#> send item lists.";
		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		removeheader(buf);
		puts(buf);
		//receive user name
		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		removeheader(buf);
		puts(buf);

		do{
			if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}
			removeheader(buf);
			if(!strcmp("ListEnd#",buf)) break;
			puts(buf);
		}while(1);

		close(new_fd);
	}

	puts("End of Phase 2 for Auction Server\n");
	/*End of phase 2*/
	/*************************************************************************************************/

	/*************************************************************************************************/
	/*phase 3: Auction*/
	// contain all broadcast items, listNode->obj = broadcastItemNode
	broadcast_list = malloc(sizeof(struct singlyLinkedList));
	memset(broadcast_list, 0, sizeof(struct singlyLinkedList));
	readBroadcastList("broadcastList.txt", broadcast_list);

	if((fp = fopen("broadcastList.txt", "r")) == NULL){
		perror("broadcastList.txt");
		exit(1);
	}
	//send broadcastList to bidder1 and bidder2, respectively
	for(i=0;i<MAXBIDDER;i++){
		rewind (fp);	//Set position of stream to the beginning
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;	//ipv4
		hints.ai_socktype = SOCK_DGRAM;	//UDP socket
		gethostname(buf, MAXDATASIZE-1);	//use buf to store hostname temporarily

		if ((rv = getaddrinfo(buf, (i==0)?PORT_BD1_P3_UDP:PORT_BD2_P3_UDP, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		// loop through all the results and bind to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next){
			if ((udp_sock_fd[i] = socket(p->ai_family, p->ai_socktype,
					p->ai_protocol)) == -1) {
					perror("server: socket");
					continue; }
//			if (setsockopt(udp_sock_fd[i], SOL_SOCKET, SO_REUSEADDR, &yes,
//					sizeof(int)) == -1) {
//					perror("setsockopt");
//					exit(1); }
//			if (bind(udp_sock_fd[i], p->ai_addr, p->ai_addrlen) == -1) {
//				close(udp_sock_fd[i]);
//				perror("server: bind");
//				continue; }
			break;
		}

		if (p == NULL){
			fprintf(stderr, "server: failed to bind\n");
			return 2;
		}

		//send one line first, because socket cannot get IP and Port Number before first call of sendto
		if(fgets(buf, sizeof(buf), fp) != NULL){
			if(!strcmp(buf, "\n")) break;	//two '\n' at the end of broadcastList.txt
			buf[strlen(buf)-1] = '\0';		//remove '\n' in the end
			addheader(buf, header);
			if ((numbytes = sendto(udp_sock_fd[i], buf, strlen(buf), 0,
								 p->ai_addr, p->ai_addrlen)) == -1) {
							perror("talker: sendto");
							exit(1);
						}
#ifdef DEBUG
			printf("talker: sent %d bytes to Bidder%d\n", numbytes, i+1);
#endif
			removeheader(buf);
			//shpw IP and Port Number
			getsockname(udp_sock_fd[i], (struct sockaddr*)&sa, &sa_len);
			inet_ntop(AF_INET, (void*)&(sa.sin_addr), s, sizeof s);
			//Phase 3: Auction Server IP Address: _______ Auction UDP Port Number: _______ .
			printf("Phase 3: Auction Server IP Address:%s Auction UDP Port Number:%d .\n",
					hostIP, sa.sin_port);
			puts("Phase 3: (Item list displayed here)");
			puts(buf);
		}

		//read one line and send per loop
		while(fgets(buf, sizeof(buf), fp) != NULL){
			if(!strcmp(buf, "\n")) break;	//two '\n' at the end of broadcastList.txt
			buf[strlen(buf)-1] = '\0';		//remove '\n' in the end
			addheader(buf, header);
			if ((numbytes = sendto(udp_sock_fd[i], buf, strlen(buf), 0,
								 p->ai_addr, p->ai_addrlen)) == -1) {
							perror("talker: sendto");
							exit(1);
						}
#ifdef DEBUG
		    printf("talker: sent %d bytes to Bidder%d\n", numbytes, i+1);
#endif
			removeheader(buf);
			puts(buf);
		}

		//indicate end of file
		strcpy(buf, "ListEnd#");
		addheader(buf, header);
		if ((numbytes = sendto(udp_sock_fd[i], buf, strlen(buf), 0,
				 p->ai_addr, p->ai_addrlen)) == -1){
			perror("talker: sendto");
			exit(1);
		}

	#ifdef DEBUG
		puts(buf);
	#endif

	    freeaddrinfo(servinfo);
	}
	fclose(fp);

	//receiver bidding from bidder1 and bidder2, respectively
	for(i=0;i<MAXBIDDER;i++){
		printf("Phase 3: Auction Server received a bidding from <Bidder%d>\n", i+1);
		puts("Phase 3: (Bidding information displayed here)");
		bidding_list[i] = malloc(sizeof(struct singlyLinkedList));	// contain bidding items, listNode->obj = BiddingItemNode
		memset(bidding_list[i], 0, sizeof(struct singlyLinkedList));
		do{
			if ((numbytes = recvfrom(udp_sock_fd[i], buf, MAXDATASIZE-1 , 0,
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

			//store bidding info into bidding_list
			struct BiddingItemNode *newObj = malloc(sizeof(struct BiddingItemNode));	//construct a new data node
			memset(newObj, 0, sizeof(struct BiddingItemNode));
			str = strtok(buf, " ");
			strcpy (newObj->name,str);
			str = strtok(NULL, " ");
			strcpy (newObj->itemName,str);
			str = strtok(NULL, "\n");
			newObj->price = atoi(str);
			strcpy(newObj->bidder, bidderName[i]);
			if (listAppend(bidding_list[i], (void *)newObj) != 0) return 1;
		}while(1);
	}


	//process info and make sold decision
	struct listNode* itr_broadcast, *itr_bidding, *itr_sold;	//iterator for broadcastlist, biddinglist and sold_list
	struct BiddingItemNode* itemBidding1, *itemBidding2;	//item on biddinglist1 and biddinglist2
	struct broadcastItemNode* itemBroadcast;	//item on broadcastlist

	listInitialize(&sold_list);

	//iterate broadcast_list for every item on the list
	for(itr_broadcast = broadcast_list->head; itr_broadcast != NULL; itr_broadcast = itr_broadcast->next){

		itemBroadcast = (struct broadcastItemNode*)itr_broadcast->obj;

		//find the matching item on biddinglist1 (bidding_list[0])
		itemBidding1 = NULL;
		for(itr_bidding = (bidding_list[0])->head; itr_bidding != NULL; itr_bidding = itr_bidding->next){
			if(!matchItem(itr_bidding->obj, itemBroadcast->name, itemBroadcast->itemName)){
				itemBidding1 = (struct BiddingItemNode*)itr_bidding->obj;
				break;
			}
		}

		//find the matching item on biddinglist2 (bidding_list[1])
		itemBidding2 = NULL;
		for(itr_bidding = (bidding_list[1])->head; itr_bidding != NULL; itr_bidding = itr_bidding->next){
			if(!matchItem(itr_bidding->obj, itemBroadcast->name, itemBroadcast->itemName)){
				itemBidding2 = (struct BiddingItemNode*)itr_bidding->obj;
				break;
			}
		}

		//bidding price must not less than minimum price
		if(itemBidding1 != NULL){
			if(itemBidding1->price < itemBroadcast->minPrice) itemBidding1 = NULL;
		}
		if(itemBidding2 != NULL){
			if(itemBidding2->price < itemBroadcast->minPrice) itemBidding2 = NULL;
		}

		struct BiddingItemNode *newSoldListObj;

		//if cannot find matching item on biddinglist1
		if(itemBidding1 == NULL){
			if(itemBidding2 == NULL) continue;
			newSoldListObj = malloc(sizeof(struct BiddingItemNode));
			memcpy(newSoldListObj, itemBidding2, sizeof(struct BiddingItemNode));
			listAppend(sold_list, newSoldListObj);
		//if can find matching item on biddinglist1, cannot find matching list on itemBidding2
		}else if(itemBidding2 == NULL){
			newSoldListObj = malloc(sizeof(struct BiddingItemNode));
			memcpy(newSoldListObj, itemBidding1, sizeof(struct BiddingItemNode));
			listAppend(sold_list, newSoldListObj);
		//if can find matching item on both lists
		}else{
			newSoldListObj = malloc(sizeof(struct BiddingItemNode));
			memcpy(newSoldListObj,
					(itemBidding1->price > itemBidding2->price)?itemBidding1:itemBidding2,
					sizeof(struct BiddingItemNode));
			listAppend(sold_list,newSoldListObj);
		}
	}

	//Now we have the complete sold_list
	//print the whole sold_list on server
	struct BiddingItemNode* soldListItem;
	for(itr_sold = sold_list->head; itr_sold != NULL; itr_sold = itr_sold->next){
		soldListItem = (struct BiddingItemNode*)itr_sold->obj;
		printf("Phase 3: Item: %s %s was sold at price %d .\n",
				soldListItem->name, soldListItem->itemName, soldListItem->price);
	}

	//sent 4 TCP packet to each of 4 users indicate the final sold info
	sleep(5);	//wait for users listen their port
	char *port_P3, *userName;
	for(i=0;i<MAXUSER;i++){
		//sending order: bidder1, bidder2, seller1, seller2
		switch(i){
		case 0:
			port_P3 = PORT_BD1_P3_TCP;
			userName = bidderName[0];
			break;
		case 1:
			port_P3 = PORT_BD2_P3_TCP;
			userName = bidderName[1];
			break;
		case 2:
			port_P3 = PORT_SL1_P3;
			userName = sellerName[0];
			break;
		case 3:
			port_P3 = PORT_SL2_P3;
			userName = sellerName[1];
			break;
		default:
			fprintf(stderr, "More than 4 users!\n");
			exit(1);
		}

		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		if ((rv = getaddrinfo(HOSTNAME, port_P3, &hints, &servinfo)) != 0) {
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

		freeaddrinfo(servinfo); // all done with this structure

		for(itr_sold = sold_list->head; itr_sold != NULL; itr_sold = itr_sold->next){
			soldListItem = (struct BiddingItemNode*)itr_sold->obj;
			if(!strcmp(soldListItem->name,userName) || !strcmp(soldListItem->bidder,userName)){
				sprintf(buf, "Phase 3: Item: %s %s was sold at price %d .\n",
							soldListItem->name, soldListItem->itemName, soldListItem->price);
				addheader(buf, header);
				if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
					perror("send");
					exit(1);
				}
			}
		}
		//indicate end of file
		strcpy(buf, "ListEnd#");
		addheader(buf, header);
		if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
				perror("send");
				exit(1);
		}

		close(sockfd);
	}

	puts("End of Phase 3 for Auction Server.");


	return 0;


}

