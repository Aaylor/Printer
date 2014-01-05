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

struct queue * allocate_queue(void);

int add_in_queue(struct queue *q, void *data);

struct node *pop(struct queue *q);

struct node *remove_node(struct queue *q, struct node *n);


#endif /* end of include guard: QUEUE_H */

