#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 5

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp () {
  if (head == NULL) {
    head = free_;
    free_ = free_->next;
    head->next = NULL;
    return head;
  }

  WP *tail = head;
  while ( tail->next != NULL) {
    tail = tail->next;
  }
  if (free_ == NULL) {
    Assert(0, "watch point consumes\n");
  }
  tail->next = free_;
  tail = tail->next;
  free_ = free_->next;
  tail->next = NULL;

  return tail;
}

void free_wp (int NO) {
  WP *node = head, *tail_free_ = free_, *pre_node = NULL;
  while (tail_free_ != NULL) {
    pre_node = tail_free_;
    tail_free_ = tail_free_->next;
  }
  tail_free_ = pre_node;

  while (node != NULL) {
    if (NO == node->NO) {
      if (node == head && head->next == NULL) {
        tail_free_->next = head;
        head = NULL;
        break;
      }
      else if (node == head && head->next != NULL) {
        tail_free_->next = head;
        head = head->next;
        tail_free_->next->next = NULL;
        break;
      }
      else if (node->next == NULL){
        tail_free_->next = node;
        pre_node->next = NULL;
        break;
      }
      else {
        tail_free_->next = node;
        pre_node->next = node->next;
        node->next = NULL;
        break;
      }
    }
    pre_node = node;
    node = node->next;
  }
}

WP* get_head() {
  return head;
}

