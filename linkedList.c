
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include "p1fxns.h"	
#include "linkedList.h"
#include <time.h>

typedef struct node Node;
struct node{

	Node *next;
	pid_t data;

};

typedef struct data{

	int size;
	Node *HEAD;
	Node *TAIL;

}LLData;

	
void llen_queue(linkedList *me, pid_t data){
	LLData *lldata = (LLData *)me->self;
	
	Node *new = (Node *)malloc(sizeof(Node));
	new->data = data;
	new->next = lldata->TAIL;
	lldata->TAIL = new;
	
	if(lldata->size == 0){
		lldata->HEAD = new;
	}
	lldata->size++;
}
	
pid_t llde_queue(linkedList *me){
	LLData *lldata = (LLData *)me->self;
	if(lldata->size == 0){
		return -99;
	}
	Node *remove = lldata->HEAD;
	pid_t data = remove->data;
	if(lldata->size == 1){
		free(remove);
		lldata->HEAD=NULL;
		lldata->TAIL=NULL;
		lldata->size--;
		return data;	
	}

	Node *curser = lldata->TAIL;
	while (curser->next != lldata->HEAD){
		curser = curser->next;
	}
	lldata->HEAD = curser;
	curser->next = NULL;
	
	free(remove);
	lldata->size--;
	
	return data;
		
}
void lldestroy(linkedList *me){
	LLData *lldata = (LLData *)me->self;

	Node *curser = lldata->TAIL;
	while(curser != NULL){
		Node *ref = curser;	
		curser = curser->next;
		free(ref);
	}

	free(lldata);	
	free(me);
}
static linkedList template = {
	NULL,llen_queue,llde_queue,lldestroy
};

linkedList *linkedListCreate(){
	linkedList *ll = (linkedList *)malloc(sizeof(linkedList));
	if(ll == NULL){
		return NULL;
	}
	LLData *lld = (LLData *)malloc(sizeof(LLData));
	if(ll == NULL){
		return NULL;
	}
	lld->size = 0;
	lld->HEAD = NULL;
	lld->TAIL = NULL;
	*ll = template;
	ll->self = lld;

	return ll;  
}











