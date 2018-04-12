
#include <stdio.h>
#include <cstring>
#include "yosefk.h"

typedef struct {
  coroutine* c;
  int max_x, max_y;
  int x, y;
} iter;

void* iterate(void* p) {
  iter* it = (iter*)p;
  int x,y;
  for(x=0; x<it->max_x; x++) {
    for(y=0; y<it->max_y; y++) {
      it->x = x;
      it->y = y;
      yield(it->c);
    }
  }
  return p;
}

void* iterate2(void* p) {
  iter* it = (iter*)p;
  int x,y;
  for(x=0; x<it->max_x; x++) {
    for(y=0; y<it->max_y; y++) {
      it->x = x;
      it->y = y;
      yield(it->c);
    }
  }
  return p;
}

#define N 16 * 1024

int main() {
  coroutine c;
  coroutine c2;
  coroutine c3;
  int stack[N];
  int stack2[N];
  int stack3[N];
  memset( stack, 0, sizeof( int ) * N );
  memset( stack2, 0, sizeof( int ) * N );
  memset( stack3, 0, sizeof( int ) * N );
  iter it = {&c, 2, 2};
  iter it2 = {&c2, 2, 2};
  iter it3 = {&c3, 2, 2};
  start(&c, &iterate, &it, stack+N);
  start(&c2, &iterate, &it2, stack2+N);
  start(&c3, &iterate, &it3, stack3+N);
  while(c.state || c2.state || c3.state)
  {
    if (c.state)
       resume(&c);
    if (c2.state)
       resume(&c2);
    if (c3.state)
       resume(&c3);
    printf("%d %d - %d %d - %d %d : %d %d %d\n", it.x, it.y, it2.x, it2.y, it3.x, it3.y, c.state, c2.state, c3.state );
  }
}
