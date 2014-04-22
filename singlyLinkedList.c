/*
 * singlyLinkedList.c
 *
 *  Created on: Apr 13, 2014
 *      Author: frankie
 */

#include "singlyLinkedList.h"


/*
 * Append a new listNode with newObj to the end of singlyLinkedList.
 * If successful, return 0. Otherwise, return 1;
 */

int listAppend(struct singlyLinkedList *list, void *newObj){
	struct listNode *newNode = malloc(sizeof(struct listNode));	//construct a new list node
	memset(newNode, 0, sizeof(struct listNode));

	newNode->obj = newObj;

	if (list->tail != NULL){				//list isn't empty
		list->tail->next = newNode;
		list->tail = newNode;
	}
	else list->head = list->tail = newNode;	//list is empty

	(list->num)++;
	return (list->tail == newNode)?0:1;
}

//This function will return the first node to which the supplied function pointer returns 0
struct listNode* listSearch(struct singlyLinkedList *list, int (*func)(void*, void*),void *keyword){
	struct listNode* p;
	for(p=list->head;p!=list->tail;p=p->next){
		if(func(p->obj, keyword)==0) return p;
	}
	return NULL;
}
/**********END: singly linked list:***************/
