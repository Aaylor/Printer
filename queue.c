#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

int
add_in_queue(struct queue *q, void *data)
{
    struct node *new;

    new = malloc(sizeof(struct node));
    if (new == NULL)
        return 1;

    new->data = data;
    new->prev = NULL;
    new->next = NULL;
    
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
    if (q->head == NULL)
        q->tail = NULL;
    
    q->length--;

    return tmp;
}

struct node *
remove_node(struct queue *q, struct node *n)
{
    int pos;
    struct node *tmp;

    if (q->length == 0)
        return NULL;

    for(tmp = q->head, pos = 0; tmp != NULL; tmp = tmp->next, pos++)
    {
        if (tmp == n)
        {
            if (tmp == q->head)
                q->head = tmp->next;

            if (tmp == q->tail)
                q->tail = tmp->prev;

            if (tmp->prev != NULL)
                tmp->prev->next = tmp->next;
            if (tmp->next != NULL)
                tmp->next->prev = tmp->prev;

            q->length--;
            
            return tmp;
        }
    }

    return NULL;
}

