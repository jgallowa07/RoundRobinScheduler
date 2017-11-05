#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include "p1fxns.h"	
#include <time.h>

typedef struct LL linkedList;

linkedList *linkedListCreate();

struct LL{

	void *self;
	
	void (*en_queue)(linkedList *me, int theData);
	
	int (*de_queue)(linkedList *me);

	void (*destroy)(linkedList *me);

};




