#ifndef LIST_H
#define LIST_H

#include "core.h"

// A simply linked list.

typedef struct ListNode ListNode;
struct ListNode {
    void* elem;
    ListNode* next;
};

typedef struct {
    ListNode* head;
    size_t len;
    size_t elem_size;
} List;

/// Creates an empty list.
/// No allocation is done.
List List_new(size_t elem_size);

/// Clears the list by removing all its elements.
void List_clear(List* l);

/// Clears the list by removing all its elements.
/// `drop_elem` will be called on each element.
void List_clear_with(List* l, void (*drop_elem)(void*));

/// Returns the length of the list.
size_t List_len(const List* l);

/// Is the list empty ?
bool List_is_empty(const List* l);

/// Returns a pointer to the first element of the list.
void* List_peek(List* l);

/// Pushes an element in front of the list.
/// The element is copied from `e`.
void List_push(List* l, void* e);

/// Pops the first element of the list.
/// Returns `true` if it was popped,
///   and copies it to `e` if `e` is not `NULL`.
/// Returns `false` otherwise.
bool List_pop(List* l, void* e);


/// Returns a pointer to the first node of the list.
const ListNode* List_front_node(const List* l);

/// Returns a pointer to the first node of the list.
ListNode* List_front_node_mut(List* l);

/// Returns a pointer to the next node.
const ListNode* ListNode_next(const ListNode* n);

/// Returns a pointer to the next node.
ListNode* ListNode_next_mut(ListNode* n);

/// Returns a pointer to the element of the node.
const void* ListNode_elem(const ListNode* n);

/// Returns a pointer to the element of the node.
void* ListNode_elem_mut(ListNode* n);

/// Removes the node after `n` from the list.
void List_remove(List* l, ListNode* n);

/// Inserts an element after `n` into the list.
/// The element is copied from `e`.
void List_insert(List* l, ListNode* n, void* e);

#endif // LIST_H
