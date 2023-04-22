#ifndef HEADER_FILE
#define HEADER_FILE


struct element {
	// Define the struct yourself
	char *operacion;
	int num_cuenta;	
	int elem1;
	int elem2;
};

typedef struct queue {
	// Define the struct yourself
	int size;
	struct element ** elementQueue;
}queue;

queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);

#endif
