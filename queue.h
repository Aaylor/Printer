#ifndef QUEUE_H
#define QUEUE_H

#include <sys/types.h>

struct queue
{
    size_t length;
    struct node *head;
    struct node *tail;
};

struct node
{
    void *data;
    struct node *prev;
    struct node *next;
};

struct queue *
allocate_queue(void);

void
free_queue(struct queue);

int 
add_in_queue(struct queue *, void *);

struct node *
pop(struct queue *);

void 
print_queue(struct queue);

#endif /* end of include guard: QUEUE_H */

