#include "../include/list.h"
#include "../include/isr.h"

void list_init(list *a) {
    a->head.next = &a->tail;
    a->head.pre = 0;
    a->tail.pre =  &a->head;
    a->tail.next = 0;
}

void list_insert_after(list_node *before, list_node *node) {
    disable_int();
    list_node *after = before->next;
    before->next = node;
    after->pre = node;
    node->next = after;
    node->pre = before;
    enable_int();
}

void list_add_first(list *a, list_node *node) {
    list_insert_after(&a->head, node);
}

void list_add_last(list *a, list_node *node) {
    list_insert_after(a->tail.pre, node);
}

void list_remove(list_node *node) {
    disable_int();
    list_node *before = node->pre;
    list_node *after = node->next;
    before->next = after;
    after->pre = before;
    enable_int();
}

list_node *list_pop_first(list *a) {
    list_node *res = a->head.next;
    list_remove(res);
    return res;
}

list_node *list_pop_last(list *a) {
    list_node *res = a->tail.pre;
    list_remove(res);
    return res;
}

u8 find_node(list *a, list_node *node) {
    list_node *p = a->head.next;
    while (p != &a->tail) {
        if (p == node) return 1;
        p = p->next;
    }
    return 0;
}

list_node *list_traversal(list *a, list_func *func, void *arg) {
    list_node *p = a->head.next;
    while (p != &a->tail) {
        if (func(p, arg)) return p;
        p = p->next;
    }
    return 0;
}

u32 list_len(list *a) {
    u32 cnt = 0;
    list_node *p = a->head.next;
    while (p != &a->tail) {
        cnt++;
        p = p->next;
    }
    return cnt;
}

u8 list_empty(list *a) {
    return a->head.next == &a->tail;
}

