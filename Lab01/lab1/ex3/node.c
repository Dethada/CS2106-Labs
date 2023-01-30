/*************************************
* Lab 1 Exercise 3
* Name: David Zhu, Tan Yuan Wei
* Student No: A0253167A, A0235417H
* Lab Group: B01, B14
*************************************/

#include "node.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Add in your implementation below to the respective functions
// Feel free to add any headers you deem fit (although you do not need to)


// Traverses list and returns the sum of the data values
// of every node in the list.
long sum_list(list *lst) {
    long sum = 0;
    node *curr = lst->head;
    while (curr != NULL) {
        sum += curr->data;
        curr = curr->next;
    }
    return sum;
}

// Traverses list and returns the number of data values in the list.
int list_len(list *lst) {
    int len = 0;
    node *curr = lst->head;
    while (curr != NULL) {
        len++;
        curr = curr->next;
    }
    return len;
}



// Inserts a new node with data value at index (counting from head
// starting at 0).
// Note: index is guaranteed to be valid.
void insert_node_at(list *lst, int index, int data) {
    node *insert = (node*)malloc(sizeof(node));
    insert->data = data;
    node *curr = lst->head;
    if (index == 0) {
        insert->next = curr;
        lst->head = insert;
    } else {
        for (int i = 0; i < index - 1; i++) {
            curr = curr->next;
        }
        insert->next = curr->next;
        curr->next = insert;
    }
}

// Deletes node at index (counting from head starting from 0).
// Note: index is guarenteed to be valid.

void delete_node_at(list *lst, int index) {
    node *curr = lst->head;
    if (index == 0) {
        lst->head = curr->next;
        free(curr);
    } else {
        for (int i = 0; i < index - 1; i++) {
            curr = curr->next;
        }
        node *toDelete = curr->next;
        curr->next = toDelete->next;
        free(toDelete);
    }
}

// Search list by the given element.
// If element not present, return -1 else return the index. If lst is empty return -2.

int search_list(list *lst, int element) {
    if (lst->head == NULL) {
        return -2;
    }

    node *curr = lst->head;
    int index = 0;
    while (curr != NULL) {
        if (curr->data == element) {
            return index;
        }
        index++;
        curr = curr->next;
    }

    return -1;
}

// Reverses the list with the last node becoming the first node.
void reverse_list(list *lst) {
    node *curr = lst->head;
    node *prev = NULL;
    node *next = NULL;
    while (curr != NULL) {
        next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }
    lst->head = prev;
}

// Resets list to an empty state (no nodes) and frees
// any allocated memory in the process
void reset_list(list *lst) {
    node *curr = lst->head;
    node *temp;
    lst->head = NULL;
    while (curr != NULL) {
        temp = curr->next;
        free(curr);
        curr = temp;
    }
    free(lst->head);
}



// Traverses list and applies func on data values of
// all elements in the list.
void map(list *lst, int (*func)(int)) {
    node *curr = lst->head;
    while (curr != NULL) {
        curr->data = (*func)(curr->data);
        curr = curr->next;
    }
}

