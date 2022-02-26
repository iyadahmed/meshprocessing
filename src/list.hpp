#ifndef LIST_HPP
#define LIST_HPP

#include "poolmem.hpp"

template <typename T> struct ListNode {
  T data;
  PoolMemNode<ListNode<T>> *mem = nullptr;
  ListNode *next = nullptr;
  ListNode *prev = nullptr;
};

template <typename T> struct List {
private:
  PoolMem<ListNode<T>> mem_pool;

public:
  ListNode<T> *head = nullptr;

  ListNode<T> *prepend(T data) {
    PoolMemNode<ListNode<T>> *new_node_mem = mem_pool.allocate();
    ListNode<T> *new_node = &(new_node_mem->data);
    new_node->mem = new_node_mem;
    new_node->data = data;
    new_node->next = head;
    new_node->prev = nullptr;
    if (head != nullptr) {
      head->prev = new_node;
    }
    head = new_node;
    return new_node;
  }

  void remove(ListNode<T> *node) {
    auto prev = node->prev;
    auto next = node->next;
    if (prev != nullptr) {
      prev->next = next;
    }
    if (next != nullptr) {
      next->prev = prev;
    }
    mem_pool.deallocate(node->mem);
  }

  void remove_by_value(T data) {
    ListNode<T> *it = head;
    while (it) {
      if (it->data == data) {
        remove(it);
        return;
      }
      it = it->next;
    }
  }
};

#endif