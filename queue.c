//SSOO-P3 2022-2023
				
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"



//To create a queue
queue* queue_init(int size){

	queue * q = (queue *)malloc(sizeof(queue));

	q->pos_ins = 0;
	q->pos_bor = 0;
	q->n_elem = 0;
	q->size = size;
	q->elementQueue = (struct element ** )malloc(size*sizeof(struct element *));	

	return q;
}


// To Enqueue an element
int queue_put(queue *q, struct element* x) {
	if (queue_full(q) != 0 ){
		q->elementQueue[q->pos_ins] = x;
		q->pos_ins = (q->pos_ins+ 1) % q->size;
		q->n_elem ++;
	}
	return 0;
}


// To Dequeue an element.
struct element* queue_get(queue *q) {
	struct element* element;
	if (queue_empty(q) != 0){
		printf("--\n");
		element = q->elementQueue[q->pos_bor];
		printf("Hola\n");
		free(q->elementQueue[q->pos_bor]);
		q->pos_bor = (q->pos_bor + 1) % q->size;
		q->n_elem --;
	}
	return element;
}


//To check queue state
int queue_empty(queue *q){
	if (q->n_elem == 0){
		return 0;
	}
	return 1;
}

int queue_full(queue *q){
	if (q->n_elem == q->size){
		return 0;
	}
	return 1;
	
}

//To destroy the queue and free the resources
int queue_destroy(queue *q){
	free(q->elementQueue);
	free(q);
	return 0;
}
