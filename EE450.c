/*
 * EE450.c
 *
 *  Created on: Apr 14, 2014
 *      Author: frankie
 */

#include "EE450.h"
#include <string.h>

//if the listObj->name field matches keyword, return 0;
int findByName(void* listObj, void* keyword){
	return strcmp(((struct userNode*)listObj)->name, (char*)keyword);
}
