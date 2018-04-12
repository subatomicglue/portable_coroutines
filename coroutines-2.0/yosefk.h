
/*
Coroutines in one page of C
Posted by Yossi Kreinin on Aug 20 2013 under Software Development | Microprocessor

A coroutine is a function that you can jump back into after returning from it - and it remembers where it was in the code, and all the variables. This is very useful at times.

One use is generating a sequence of values. Here's how you can generate all the x,y pairs in a 2D range in Python:

def iterate(max_x, max_y):
  for x in range(max_x):
    for y in range(max_y):
      yield x,y

for x,y in iterate(2,2):
  print x,y

This prints:

0 0
0 1
1 0
1 1

The yield keyword is like return, except you can call the function again and it proceeds from the point where it left. Without the syntactic sugar, the loop using iterate() would look like this:

coroutine = iterate(2,2)
while True:
  try:
    x,y = coroutine.next()
    print x,y
  except StopIteration:
    break

So iterate() returns a coroutine object, and you can call coroutine.next() to get the next value produced by yield. Eventually, iterate() runs out of values and returns, and then you get a StopIteration exception. That's Python's protocol for coroutines.

This can be nicer than straightforwardly iterating over the 2D range with 2 nested loops, because we're abstracting away the nested loops - or recursive tree/graph walks, or other hairy traversals. And it's nicer than writing iterator classes to abstract away traversals, because you don't need to manually compile the logic down to a state machine, as in "if(_x==_max_x) _y++; else _x++". The iteration is both abstracted away and can be seen plainly in the code implementing it.

Coroutines have other uses; a fashionable one is multiplexing many concurrent processes onto a small thread pool. As in, goroutines are coroutines, just, um, spelled differently.

Unfortunately, like many other languages, C doesn't have coroutines. What it does have though is setjmp/longjmp, which come very close to coroutine support - the missing step is just a sprinkle of inline assembly.

On some systems you actually get full coroutine support out of the box - as Wikipedia points out, you don't need any assembly if you have makecontext and friends. Many embedded systems don't have makecontext though, and do have setjmp/longjmp, so it's interesting how we can get by with just that.

The example code below isn't a full-fledged coroutine library - there are many libraries to choose from already. Rather, it shows what it takes to roll your own coroutines if you need to - which is just a page of code.

So here's how the iterator example can look in C with coroutines:

#include <stdio.h>
#include "coroutine.h"

typedef struct {
  coroutine* c;
  int max_x, max_y;
  int x, y;
} iter;

void iterate(void* p) {
  iter* it = (iter*)p;
  int x,y;
  for(x=0; x<it->max_x; x++) {
    for(y=0; y<it->max_y; y++) {
      it->x = x;
      it->y = y;
      yield(it->c);
    }
  }
}

#define N 1024

int main() {
  coroutine c;
  int stack[N];
  iter it = {&c, 2, 2};
  start(&c, &iterate, &it, stack+N);
  while(next(&c)) {
    printf("%d %d\n", it.x, it.y);
  }
}

This is somewhat uglier than Python, with an iter struct used for specifying the 2D range (max_x, max_y) as well as for generating the values (x,y). We also get to allocate a stack - and, in my simplistic example, we must actually know that stacks grow downwards on our platform (hence we pass stack+N instead of stack). Worse, C doesn't really give us a way to estimate how much stack space we'll need, or to grow the stack on demand.

But the flow is similar - start initializes a coroutine, next is used to get values, returning false when it runs out of values, and yield is called to jump back to the consumer.

We'll now walk through an implementation of this coroutine interface. The interface itself looks like this:
*/

#include <setjmp.h>

#ifdef _WIN64
"sorry, inline asm doesn't work in WIN64, and yousefk-asm.asm doesn't work either";
#endif

struct coroutine
{
  jmp_buf callee_context;
  jmp_buf caller_context;
  enum { WORKING=1, DONE };
  int state;
};

typedef void* (*func)(void*);

void start(coroutine* c, func f, void* arg, void* sp);
void yield(coroutine* c);
int resume(coroutine* c);
