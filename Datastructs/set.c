#include "set.h"

void set_init(Set *set, int (*match)(const void *key1, const void *key2), void (*destroy)(void *data)) {
    dlist_init(set, destroy);
    set->match = match;
    return;
}
int set_insert(Set *set, const void *data) {
    if (set_is_member(set, data)) 
        return -1;
    return dlist_ins_next(set, dlist_tail(set), data);
}
int set_remove(Set *set, void **data) {
    DListElmt *member;
    for (member = dlist_head(set); member != NULL; member = dlist_next(member)) {
        if (set->match(dlist_data(member), *data))
            return dlist_remove(set, member, data);
    }
    return -1;
}
int set_union(Set *setu, const Set *set1, const Set *set2) {
    DListElmt *el;
    void *data;
    set_init(setu, set1->match, NULL);
    // insert from set1
    for (el = dlist_head(set1); el != NULL; el = dlist_next(el)) {
        data = dlist_data(el);
        if (dlist_ins_next(setu, dlist_tail(setu), data) != 0) {
            set_destroy(setu);
            return -1;
        }
    }
    // insert from set2
    for (el = dlist_head(set2); el != NULL; el = dlist_next(el)) {
        if (set_is_member(set1, dlist_data(el))) {
            continue;
        } else {
            data = dlist_data(el);
            if (dlist_ins_next(setu, dlist_tail(setu), data) != 0) {
                set_destroy(setu);
                return -1;
            }
        }
    }
    return 0;
}
int set_intersection(Set *seti, const Set *set1, const Set *set2) {
    DListElmt *el;
    void *data;
    set_init(seti, set1->match, NULL);
    for (el = dlist_head(set1); el != NULL; el = dlist_next(el)) {
        if (set_is_member(set2, dlist_data(el))) {
            data = dlist_data(el);
            if (dlist_ins_next(seti, dlist_tail(seti), data) != 0) {
                dlist_destroy(seti);
                return -1;
            }
        }
    }
    return 0;
}
int set_difference(Set *setd, const Set *set1, const Set *set2) {
    DListElmt *el;
    void *data;
    set_init(setd, set1->match, NULL);
    for (el = dlist_head(set1); el != NULL; el = dlist_next(el)) {
        if (!set_is_member(set2, dlist_data(el))) {
            data = dlist_data(el);
            if (dlist_ins_next(setd, dlist_tail(setd), data) != 0) {
                set_destroy(setd);
                return -1;
            }
        }
    }
    return 0;
}
int set_is_member(const Set *set, const void *data) {
    DListElmt *member;
    for (member = dlist_head(set); member != NULL; member = dlist_next(member)) {
        if (set->match(data, dlist_data(member))) 
            return 1;
    }
    return 0;
}
int set_is_subset(const Set *set1, const Set *set2) {
    DListElmt *el;
    if (set_size(set1) > set_size(set2)) 
        return 0;
    for (el = dlist_head(set1); el != NULL; el = dlist_next(el)) {
        if (!set_is_member(set2, dlist_data(el)))
            return 0;
    }
    return 1;
}
int set_is_equal(const Set *set1, const Set *set2) {
    if (set_size(set1) != set_size(set2))
        return 0;
    return set_is_subset(set1, set2);
}

void *set_get_member(Set *set, const void *data) {
    DListElmt *member;
    for (member = dlist_head(set); member != NULL; member = dlist_next(member)) {
        if (set->match(data, dlist_data(member))) 
            return dlist_data(member);
    }
    return NULL;
}