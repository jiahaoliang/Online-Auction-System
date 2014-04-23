/*
 * singlyLinkedList.h
 *
 *  Created on: Apr 13, 2014
 *      Author: frankie
 */

#ifndef SINGLYLINKEDLIST_H_
#define SINGLYLINKEDLIST_H_

#include <stdlib.h>
#include <string.h>

struct listNode{
	void *obj;
	struct listNode *next;
};

struct singlyLinkedList{
	int num;
	struct listNode *head;
	struct listNode *tail;
};

int listInitialize(struct singlyLinkedList **p_list);
int listAppend(struct singlyLinkedList *list, void *newObj);
struct listNode* listSearch(struct singlyLinkedList *list, int (*func)(void*, void*),void *keyword);
#endif /* SINGLYLINKEDLIST_H_ */
