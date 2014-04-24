/*
 * EE450.c
 *
 *  Created on: Apr 14, 2014
 *      Author: frankie
 */

#include "EE450.h"
#include <string.h>
#include <stdio.h>

//if the listObj->name field matches keyword, return 0;
int findByName(void* listObj, void* keyword){
	return strcmp(((struct userNode*)listObj)->name, (char*)keyword);
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
