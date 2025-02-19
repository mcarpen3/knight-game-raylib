#include "../datastructs/doublelinkedlist.h"
#include<stdio.h>
#include <stdlib.h>

typedef struct Test_ {
    float value;
    char *name;
} Test;

static void Destroy(Test *t) {
    float out = t->value;
    free(t->name);
    free(t);
    printf("%f freed!\n", out);
}
int main(void) {
    printf("Hello, World!\n");
    DList *dlist = (DList *)malloc(sizeof(DList));
    dlist_init(dlist, (void *)Destroy);
    for (int i = 0; i < 10; ++i) {
        Test *t = (Test *)malloc(sizeof(Test));
        t->name = (char *)malloc(2);
        t->value = i;
        *t->name = i + 65;
        t->name[1] = '\0';
        dlist_ins_next(dlist, dlist_tail(dlist), t);
    }
    printf("dlist size = %d\n", dlist_size(dlist));
    DListElmt *idx = dlist_head(dlist);
    Test *t;
    while(idx != NULL) {
        t = (Test *)idx->data;
        printf("%.2f - %s\n", t->value, t->name);
        idx = idx->next;
    }
    idx = dlist_head(dlist);
    int i = 0;
    while(idx != NULL) {
        int mod = i % 2;
        if (mod == 0) {
            Test *rmvd = (Test *)malloc(sizeof(Test));
            DListElmt *tmp = idx->next;
            dlist_remove(dlist, idx, (void **)&rmvd);
            printf("%.2f, %s\n", rmvd->value, rmvd->name);
            free(rmvd);
            idx = tmp;
        } else {
            idx = idx->next;
        }
        i++;
    }
    printf("dlist size = %d\n", dlist_size(dlist));
    idx = dlist_tail(dlist);
    while (idx != NULL) {
        t = (Test *)idx->data;
        printf("%.2f - %s\n", t->value, t->name);
        idx = idx->prev;
    }
    printf("destroying dlist\n");
    dlist_destroy(dlist);
}