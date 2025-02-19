#ifndef DOUBLELINKEDLIST_H
#define DOUBLELINKEDLIST_H

typedef struct DListElmt_ {
    struct DListElmt_ *prev;
    struct DListElmt_ *next;
    void *data;
} DListElmt;

typedef struct DList_ {
    int (*match)(const void *key1, const void *key2);
    void (*destroy)(void *data);
    DListElmt *head;
    DListElmt *tail;
    int size;
} DList;

void dlist_init(DList *dlist, void (*destroy)(void *data));
void dlist_destroy(DList *dlist);
int dlist_ins_next(DList *dlist, DListElmt *element, void *data);
int dlist_ins_prev(DList *dlist, DListElmt *element, void *data);
int dlist_remove(DList *dlist, DListElmt *element, void **data);

#define dlist_size(dlist) ((dlist)->size)
#define dlist_head(dlist) ((dlist)->head)
#define dlist_tail(dlist) ((dlist)->tail)
#define dlist_is_head(element) ((element)->prev == NULL ? 1 : 0)
#define dlist_is_tail(element) ((element)->next == NULL ? 1 : 0)
#define dlist_data(element) ((element)->data)
#define dlist_next(element) ((element)->next)

#endif // DOUBLELINKEDLIST_H