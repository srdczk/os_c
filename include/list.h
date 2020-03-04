#pragma once

#include "types.h"

#define offset(struct_name, member) (u32)(&((struct_name *)0)->member)
#define node2entry(struct_name, member, ptr) \
    (struct_name *)((u32)ptr - offset(struct_name, member))

// 双向链表
typedef struct list_node {
    struct list_node *pre, *next;
} list_node;

typedef struct {
    list_node head, tail;
} list;

// 判断链表 元素是否满足条件
typedef u8 list_func(list_node *, void *);

void list_init(list *a);

void list_insert_after(list_node *before, list_node *node);

void list_add_first(list *a, list_node *node);

void list_add_last(list *a, list_node *node);

void list_remove(list_node *node);

list_node *list_pop_first(list *a);

list_node *list_pop_last(list *a);

u8 find_node(list *a, list_node *node);

list_node *list_traversal(list *a, list_func *func, void *arg);

u32 list_len(list *a);

u8 list_empty(list *a);
