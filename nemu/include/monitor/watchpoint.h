#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  char expr[65536];
	uint32_t Val;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */


} WP;

WP* new_wp ();
void free_wp (int NO);
WP* get_head();

#endif
