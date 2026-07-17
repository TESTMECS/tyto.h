//! @module ty_linkedlist.h
//! 	A linked list module.
#ifndef TY_LINKLIST_H_
#define TY_LINKLIST_H_

#include <stdlib.h>

#ifndef TY_LINKLIST_MALLOC
#define TY_LINKLIST_MALLOC malloc
#endif  // TY_LINKEDLIST_MALLOC

#ifndef TY_LINKLIST_FREE
#define TY_LINKLIST_FREE free
#endif  // TY_LINKEDLIST_FREE

enum ty_list_direction
{
    TY_LIST_HEAD,
    TY_LIST_TAIL,
};

struct ty_list_node_t
{
    struct ty_list_node_t* next;
    struct ty_list_node_t* prev;
    void*                  data;
};

struct ty_list_t
{
    struct ty_list_node_t* head;
    struct ty_list_node_t* tail;
    unsigned int           length;
    void (*list_free_fn)(void* val);
    int (*match_fn)(void* a, void* b);
};

struct ty_list_iter_t
{
    struct ty_list_node_t* next;
    enum ty_list_direction direction;
};

struct ty_list_node_t*
ty_list_node_new(void* val);

struct ty_list_t*
ty_list_new(void);

struct ty_list_node_t*
ty_list_rpush(struct ty_list_t* lst, struct ty_list_node_t* node);

struct ty_list_node_t*
ty_list_lpush(struct ty_list_t* lst, struct ty_list_node_t* node);

struct ty_list_node_t*
ty_list_find(struct ty_list_t* lst, void* data);

struct ty_list_node_t*
ty_list_at(struct ty_list_t* lst, int idx);

struct ty_list_node_t*
ty_list_rpop(struct ty_list_t* lst);

struct ty_list_node_t*
ty_list_lpop(struct ty_list_t* lst);

void
ty_list_remove(struct ty_list_t* lst, struct ty_list_node_t* node);

void
ty_list_del(struct ty_list_t* lst);

struct ty_list_iter_t*
ty_list_iter_new(struct ty_list_t* lst, enum ty_list_direction dir);

struct ty_list_iter_t*
ty_list_iter_new_from_node(
    struct ty_list_node_t* node,
    enum ty_list_direction dir);

struct ty_list_node_t*
ty_list_iter_next(struct ty_list_iter_t* lst);

void
ty_list_iter_del(struct ty_list_iter_t* lst);

#endif /* TY_LINKLIST_H_ */
