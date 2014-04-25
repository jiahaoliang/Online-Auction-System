/*
 * EE450.h
 *
 *  Created on: Apr 14, 2014
 *      Author: frankie
 */

#ifndef EE450_H_
#define EE450_H_

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

//#define DEBUG

#define MAXDATASIZE 100 // max number of bytes we can get at once
#define PORTNUM_LEN 6	//"65566\0", 6 char in total

#define NAME_MAX_LEN 10		//max length of a user name in Registration.txt
#define PW_MX_LEN 10		//max length of a password in Registration.txt
#define ACCOUNT_NUM_MAX_LEN 10	//max length of an account number in Registration.txt

#define REG_TXT_LINE_LEN 33		//max length of a line in Registration.txt, NAME_MAX_LEN + PW_MX_LEN + ACCOUNT_NUM_MAX_LEN + 3spaces
#define PASS_TXT_LINE_LEN 35	//max length of a line in bidderPass.txt & sellerPass.txt, REG_TXT_LINE_LEN +2

#define HOSTNAME "nunki.usc.edu"

/**********************************************************
 * Static Ports
 * Server
 * 1 TCP, 1100+xxx (last three digits of your ID) (phase 1)
 * 1 TCP, 1200+xxx (last three digits of your ID) (phase 2)
 **********************************************************/
#define PORT_S_P1 "1894"	//Port number of Server in Phase 1, 1100+794 = 1894
#define PORT_S_P2 "1994"	//Port number of Server in Phase 2, 1200+794 = 1994

/**********************************************************
 * Seller1
 * 1 TCP, 2100+xxx (last three digits of your ID) (phase 3)
 * Seller2
 * 1 TCP, 2200+xxx (last three digits of your ID) (phase 3)
 **********************************************************/
#define PORT_SL1_P3 "2894"	//Port number of Seller1 in Phase 3, 2100+794 = 2894
#define PORT_SL2_P3 "2994"	//Port number of Seller1 in Phase 3, 2200+794 = 2994

/**********************************************************
 * Bidder1
 * 1 UDP, 3100 + xxx (last three digits of your ID) (phase 3)
 * 1 TCP, 4100 + xxx (last three digits of your ID) (phase 3)
 * Bidder2
 * 1 UDP, 3200 + xxx (last three digits of your ID) (phase 3)
 * 1 TCP, 4200 + xxx (last three digits of your ID) (phase 3)
 **********************************************************/
#define PORT_BD1_P3_UDP "3894"	//UDP Port number of Bidder1 in Phase 3, 3100+794 = 3894
#define PORT_BD1_P3_TCP "4894"	//TCP Port number of Bidder1 in Phase 3, 4100+794 = 4894
#define PORT_BD2_P3_UDP "3994"	//UDP Port number of Bidder2 in Phase 3, 3200+794 = 3994
#define PORT_BD2_P3_TCP "4994"	//TCP Port number of Bidder2 in Phase 3, 4200+794 = 4994


struct userNode{
	int type;	//default 0, unspecified; set 1 as bidder; set 2 as seller
	int userIndex; //identify different users, example seller1, seller 2, bidder 1, bidder2, etc.
	char name[NAME_MAX_LEN];
	char password[PW_MX_LEN];
	char accountNum[ACCOUNT_NUM_MAX_LEN];
};

struct acceptedUserNode{
	int type;	//default 0, unspecified; set 1 as bidder; set 2 as seller
	int userIndex; //identify different users, example seller1, seller 2, bidder 1, bidder2, etc.
	char name[NAME_MAX_LEN];
	char password[PW_MX_LEN];
	char accountNum[ACCOUNT_NUM_MAX_LEN];
	char ip_addr[INET6_ADDRSTRLEN];	//ip address of accepted user, can be either IPv4 or IPv6
};

int findByName(void* listObj, void* keyword);
char *addheader(char* dest, char* header);
int removeheader(char* input);

#endif /* EE450_H_ */
