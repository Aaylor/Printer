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

/*
 *  Ajoute la donnée dans la file.
 */
int add_in_queue(struct queue *q, void *data);

/*
 *  Retire le premier element de la file.
 *  Renvoie NULL si aucun élément existe.
 */
struct node *pop(struct queue *q);

/*
 *  Retire l'élement donnée de la file.
 *  Renvoie NULL si l'élément n'existe pas, renvoie l'élement sinon.
 */
struct node *remove_node(struct queue *q, struct node *n);


#endif /* end of include guard: QUEUE_H */

