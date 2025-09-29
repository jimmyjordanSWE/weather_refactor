#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>

typedef struct Node Node;
struct Node {
    Node *back;
    Node *front;
    void *item;
};

typedef struct {
    Node *head;
    Node *tail;
    size_t size;
} LinkedList;

/* initialize new LinkedList */
LinkedList *LinkedList_create();

/* return Node at specific index in LinkedList */
Node *LinkedList_get_index(LinkedList *list, size_t index);

/* add void pointer to specified index of LinkedList */
int LinkedList_insert(LinkedList *list, size_t index, void *item);

/* add void pointer to end of LinkedList */
int LinkedList_append(LinkedList *list, void *item);

/* remove Node from LinkedList by reference */
int LinkedList_remove(LinkedList *list, Node *item);

/* remove Node from LinkedList at index */
int LinkedList_pop(LinkedList *list, size_t index);

/* remove all nodes in a LinkedList */
void LinkedList_clear(LinkedList *list, void free_function(void *));

/* dispose LinkedList, does NOT free memory of items in nodes
                 as we cannot ensure it's safe */
void LinkedList_dispose(LinkedList *list, void free_function(void *));

#endif