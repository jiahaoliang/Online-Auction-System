/*
 * EE450.h
 *
 *  Created on: Apr 14, 2014
 *      Author: frankie
 */

#ifndef EE450_H_
#define EE450_H_

#define NAME_MAX_LEN 10		//max length of a user name in Registration.txt
#define PW_MX_LEN 10		//max length of a password in Registration.txt
#define ACCOUNT_NUM_MAX_LEN 10	//max length of an account number in Registration.txt

#define REG_TXT_LINE_LEN 33		//max length of a line in Registration.txt, NAME_MAX_LEN + PW_MX_LEN + ACCOUNT_NUM_MAX_LEN + 3spaces
#define PASS_TXT_LINE_LEN 35	//max length of a line in bidderPass.txt & sellerPass.txt, REG_TXT_LINE_LEN +2

#define HOSTNAME "nunki.usc.edu"

/**********************************************************
 * Static Ports
 * 1 TCP, 1100+xxx (last three digits of your ID) (phase 1)
 * 1 TCP, 1200+xxx (last three digits of your ID) (phase 2)
 **********************************************************/
#define PORT_S_P1 "1100794"	//Port number of Server in Phase 1
#define PORT_S_P2 "1200794"	//Port number of Server in Phase 2


struct userNode{
	int type;	//default 0, unspecified; set 1 as bidder; set 2 as seller
	char name[NAME_MAX_LEN];
	char password[PW_MX_LEN];
	char accountNum[ACCOUNT_NUM_MAX_LEN];
};

#endif /* EE450_H_ */