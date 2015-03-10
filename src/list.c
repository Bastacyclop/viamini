#include "list.h"

static
ListNode* new_node(void* e, size_t elem_size, ListNode* next);

List List_new(size_t elem_size) {
    return (List) { .head = NULL, .len = 0, .elem_size = elem_size };
}

size_t List_len(const List* l) {
    return l->len;
}

bool List_is_empty(const List* l) {
    return !l->head;
}

void List_clear(List* l, void (*drop_elem)(void*)) {
    void* e = NULL;
    while (List_pop(l, e)) {
        drop_elem(e);
    }
}

void List_plain_clear(List* l) {
    while (List_pop(l, NULL)) {}
}

void* List_peek(List* l) {
    if (List_is_empty(l)) {
        return NULL;
    } else {
        return l->head->elem;
    }
}

void List_push(List* l, void* e) {
    ListNode* new_head = new_node(e, l->elem_size, l->head);
    l->head = new_head;
    l->len++;
}

ListNode* new_node(void* e, size_t elem_size, ListNode* next) {
    ListNode* n = malloc(sizeof(ListNode));
    assert_alloc(n);

    void* elem = malloc(elem_size); // should avoid double allocation
    memcpy(elem, e, elem_size);

    *n = (ListNode) { .elem = elem, .next = next };
    return n;
}

bool List_pop(List* l, void* e) {
    if (List_is_empty(l)) {
        return false;
    } else {
        if (e) memcpy(e, l->head->elem, l->elem_size);
        ListNode* new_head = l->head->next;
        free(l->head->elem);
        free(l->head);
        l->head = new_head;
        l->len--;
        return true;
    }
}


const ListNode* List_front_node(const List* l) {
    return l->head;
}

ListNode* List_front_node_mut(List* l) {
    return l->head;
}

const ListNode* ListNode_next(const ListNode* n) {
    return n->next;
}

ListNode* ListNode_next_mut(ListNode* n) {
    return n->next;
}

const void* ListNode_elem(const ListNode* n) {
    return n->elem;
}

void* ListNode_elem_mut(ListNode* n) {
    return n->elem;
}

void List_remove(List* l, ListNode* n) {
    if (n->next) {
        ListNode* new_next = n->next->next;
        free(n->next->elem);
        free(n->next);
        n->next = new_next;
        l->len--;
    }
}

void List_insert(List* l, ListNode* n, void* e) {
    ListNode* i = new_node(e, l->elem_size, n->next);
    n->next = i;
    l->len++;
}
