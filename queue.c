#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

struct queue *
allocate_queue(void)
{
    struct queue *new_queue;

    new_queue = malloc(sizeof(struct queue));
    if (new_queue == NULL)
        return NULL;

    new_queue->length   = 0;
    new_queue->head     = NULL;

    return new_queue;
}


void
free_queue(struct queue q)
{

}

void
free_node(struct node *n)
{

}


int
add_in_queue(struct queue *q, void *data)
{
    struct node *new;

    new = malloc(sizeof(struct node));
    if (new == NULL)
        return 1;

    new->data = data;
    
    if (q->head == NULL)
    {
        q->head = new;
        q->tail = new;
    }
    else
    {
        new->prev = q->tail;
        q->tail->next = new;
        q->tail = new;
    }

    q->length++;

    return 0;
}

struct node *
pop(struct queue *q)
{
    struct node *tmp;

    if (q->length == 0)
        return NULL;

    tmp = q->head;
   
    if (tmp->next != NULL)
        tmp->next->prev = NULL;
    q->head = tmp->next;
    q->length--;

    return tmp;
}

/*
 * TEMP
 */
void
print_queue(struct queue q)
{
    int i;
    struct node *tmp;

    if (q.length == 0)
        printf("QUEUE VIDE\n");
    else
    {
        i = 1;
        tmp = q.head;
        while (tmp != NULL)
        {
            printf("ELEMENT %d\n", i++);
            tmp = tmp->next;
        }
    }
}
