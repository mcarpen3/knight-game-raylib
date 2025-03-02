#include <stdlib.h>
#include<string.h>
#include "doublelinkedlist.h"

void dlist_init(DList *dlist, void (*destroy)(void *data)) {
    dlist->head = NULL;
    dlist->tail = NULL;
    dlist->destroy = destroy;
    dlist->size = 0;
}
void dlist_destroy(DList *dlist) {
    void *data;
    while(dlist_size(dlist) > 0) {
        if (dlist_remove(dlist, dlist_tail(dlist), (void **)&data) == 0 && dlist->destroy != NULL) {
            dlist->destroy(data);
        }
    }
    memset(dlist, 0, sizeof(DList));
    return;
}
int dlist_ins_next(DList *dlist, DListElmt *element, const void *data) {
    DListElmt *new_el; 
    if (element == NULL && dlist_size(dlist) != 0) return -1;
    if ((new_el = (DListElmt *)malloc(sizeof(DListElmt))) == NULL) {
        return -1;
    }
    new_el->data = (void *)data;
    if (dlist_size(dlist) == 0) {
        new_el->next = NULL;
        new_el->prev = NULL;
        dlist->head = new_el;
        dlist->tail = new_el;
    } else {
        new_el->next = element->next;
        element->next = new_el;
        new_el->prev = element;
        if (new_el->next == NULL) {
            dlist->tail = new_el;
        } else {
            new_el->next->prev = new_el;
        }
    }
    dlist->size++;
    return 0;
}

int dlist_ins_prev(DList *dlist, DListElmt *element, const void *data) {
    DListElmt *new_el;
    if (element == NULL && dlist_size(dlist) != 0) return -1;
    if ((new_el = (DListElmt *)malloc(sizeof(DListElmt))) == NULL) return -1;
    new_el->data = (void *)data;
    if (element == NULL) {
        dlist->head = new_el;
        dlist->tail = new_el;
        new_el->prev = NULL;
        new_el->next = NULL;
    } else {
        new_el->prev = element->prev;
        new_el->next = element;
        element->prev = new_el;
        if (new_el->prev == NULL) {
            dlist->head = new_el;
        }
    }
    dlist->size++;
    return 0;
}

int dlist_remove(DList *dlist, DListElmt *element, void **data) {
    if (element == NULL || dlist_size(dlist)== 0) return -1;
    *data = element->data;
    if (element == dlist->head) {
        dlist->head = element->next;
        if (dlist->head == NULL) {
            dlist->tail = NULL;
        } else {
            element->next->prev = NULL;
        }
    } else {
        element->prev->next = element->next;
        if (element->next == NULL) {
            dlist->tail = element->prev;
        } else {
            element->next->prev = element->prev;
        }
    }
    free(element);
    dlist->size--;
    return 0;
}