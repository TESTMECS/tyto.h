#include <ty_linklist.h>

struct ty_list_iter_t*
ty_list_iter_new(struct ty_list_t* lst, enum ty_list_direction dir)
{
    struct ty_list_node_t* node = (dir == TY_LIST_HEAD) ? lst->head : lst->tail;
    return ty_list_iter_new_from_node(node, dir);
}

struct ty_list_iter_t*
ty_list_iter_new_from_node(
    struct ty_list_node_t* node,
    enum ty_list_direction dir)
{
    struct ty_list_iter_t* self;
    //!
    if (!(self = TY_LINKLIST_MALLOC(sizeof(struct ty_list_iter_t))))
        return NULL;
    self->next      = node;
    self->direction = dir;
    return self;
}

struct ty_list_node_t*
ty_list_iter_next(struct ty_list_iter_t* lst)
{
    struct ty_list_node_t* curr = lst->next;
    if (curr)
        lst->next = (lst->direction == TY_LIST_HEAD) ? curr->next : curr->prev;
    return curr;
}

void
ty_list_iter_del(struct ty_list_iter_t* lst)
{
    TY_LINKLIST_FREE(lst);
    lst = NULL;
}

struct ty_list_node_t*
ty_list_node_new(void* val)
{
    struct ty_list_node_t* self;
    if (!(self = TY_LINKLIST_MALLOC(sizeof(struct ty_list_node_t))))
        return NULL;
    self->prev = NULL;
    self->next = NULL;
    self->data = val;
    return self;
}

struct ty_list_t*
ty_list_new(void)
{
    struct ty_list_t* self;
    if (!(self = TY_LINKLIST_MALLOC(sizeof(struct ty_list_t))))
        return NULL;
    self->head         = NULL;
    self->tail         = NULL;
    self->list_free_fn = NULL;
    self->match_fn     = NULL;
    self->length       = 0;
    return self;
}

void
ty_list_del(struct ty_list_t* lst)
{
    unsigned int           len = lst->length;
    struct ty_list_node_t* next;
    struct ty_list_node_t* curr = lst->head;

    while (len--) {
        next = curr->next;
        if (lst->list_free_fn)
            lst->list_free_fn(curr->data);
        TY_LINKLIST_FREE(curr);
        curr = next;
    }
    TY_LINKLIST_FREE(lst);
}

struct ty_list_node_t*
ty_list_rpush(struct ty_list_t* lst, struct ty_list_node_t* node)
{
    if (!node)
        return NULL;

    if (lst->length) {
        node->prev      = lst->tail;
        node->next      = NULL;
        lst->tail->next = node;
        lst->tail       = node;
    } else {
        lst->head = lst->tail = node;
        node->prev = node->next = NULL;
    }
    ++lst->length;
    return node;
}
