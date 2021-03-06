/*
 * EE450.c
 *
 *  Created on: Apr 14, 2014
 *      Author: frankie
 */

#include "EE450.h"
#include <string.h>
#include <stdio.h>

//struct userNode*)listObj
//if the listObj->name field matches keyword, return 0;
int findByName(void* listObj, void* keyword){
	return strcmp(((struct userNode*)listObj)->name, (char*)keyword);
}

//(struct BiddingItemNode*)listObj
//if the listObj->bidder field matches keyword, return 0;
int findByBidder(void* listObj, void* keyword){
	return strcmp(((struct BiddingItemNode*)listObj)->bidder, (char*)keyword);
}

//(struct BiddingItemNode*)listObj
//if the listObj->name field matches name, listObj->itemName matches itemName, return 0;
int matchItem(void* listObj, void* name, void* itemName){
	return (strcmp(((struct BiddingItemNode*)listObj)->name, (char*)name) ||
			strcmp(((struct BiddingItemNode*)listObj)->itemName, (char*)itemName));
}

//header format: "header"
//output string: "<header>dest"
char *addheader(char* dest, char* header){
	char temp[MAXDATASIZE];
	strcpy(temp, dest);
	sprintf(dest,"<%s>%s", header, temp);
	return dest;
}

//input string: "<header>string"
//output string:"string"
//return: 0, if successful;1, if fails, and input unchanges
int removeheader(char* input){
	char temp[MAXDATASIZE], *str;
	strcpy(temp, input);
	if(temp[0]!='<') return 1;
	if(strtok(temp,"<>") != NULL){
		str = strtok(NULL,"\0");
		strcpy(input, str);
		return 0;
	}else{
		return 1;
	}
}
