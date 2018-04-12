
Coroutines in one page of C
Allocates coroutine stacks onto the heap - unlimited coros, less portable w/asm
http://www.embeddedrelated.com/showarticle/455.php
http://yosefk.com/blog/coroutines-in-one-page-of-c.html


=================================================

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

#include <setjmp.h>

typedef struct {
  jmp_buf callee_context;
  jmp_buf caller_context;
} coroutine;

typedef void (*func)(void*);

void start(coroutine* c, func f, void* arg, void* sp);
void yield(coroutine* c);
int next(coroutine* c);

A coroutine is thus two contexts: the callee's (producer's), and the caller's (consumer's). The tricky and non-portable part is initializing these contexts (jmp_buf's). The cleaner and portable part is jumping back and forth between them, so let's see that first:

#include "coroutine.h"

enum { WORKING=1, DONE };

void yield(coroutine* c) {
  if(!setjmp(c->callee_context)) {
    longjmp(c->caller_context, WORKING);
  }
}

int next(coroutine* c) {
  int ret = setjmp(c->caller_context);
  if(!ret) {
    longjmp(c->callee_context, 1);
  }
  else {
    return ret == WORKING;
  }
}

So yield and next are each just a pair of setjmp/longjmp calls. The trick with setjmp is, it returns 0 right after you call it - having saved the context (machine registers, basically) to your jmp_buf. You can later return to this context (restore register values) with longjmp.

Where exactly does longjmp bring you? To the point in code where setjmp has just returned. How do you know that setjmp has just returned because longjmp brought you there - as opposed to returning directly after you called setjmp? Because setjmp returns the non-zero value that was passed to longjmp, rather than 0 as it does when returning directly. So if(!setjmp) asks, "did I just call you - or was it a long time ago and we're back from longjmp?"

As you can see, there's almost no difference between yield and next - both say, "save our current context and then go to the other side's context". The only thing setting next apart is, it reports whether we're DONE or WORKING.

But, it looks like we're always WORKING, doesn't it? ret is what's passed to longjmp and yield always passes WORKING. Which brings us to start - the one passing DONE, because, well, the code of start also contains the part where we're finished.

typedef struct {
  coroutine* c;
  func f;
  void* arg;
  void* old_sp;
  void* old_fp;
} start_params;

#define get_sp(p) \
  asm volatile("movq %%rsp, %0" : "=r"(p))
#define get_fp(p) \
  asm volatile("movq %%rbp, %0" : "=r"(p))
#define set_sp(p) \
  asm volatile("movq %0, %%rsp" : : "r"(p))
#define set_fp(p) \
  asm volatile("movq %0, %%rbp" : : "r"(p))

enum { FRAME_SZ=5 }; //fairly arbitrary

void start(coroutine* c, func f, void* arg, void* sp)
{
  start_params* p = ((start_params*)sp)-1;

  //save params before stack switching
  p->c = c;
  p->f = f;
  p->arg = arg;
  get_sp(p->old_sp);
  get_fp(p->old_fp);

  set_sp(p - FRAME_SZ);
  set_fp(p); //effectively clobbers p
  //(and all other locals)
  get_fp(p); //…so we read p back from $fp

  //and now we read our params from p
  if(!setjmp(p->c->callee_context)) {
    set_sp(p->old_sp);
    set_fp(p->old_fp);
    return;
  }
  (*p->f)(p->arg);
  longjmp(p->c->caller_context, DONE);
}

This is longer, uglier, non-portable and perhaps not strictly correct (please tell me if you spot bugs - I have close to zero experience with x86-64, it's just what I test on at home.)

So why so ugly? Because we need a separate stack for the coroutine (the producer), and C doesn't have a way to give it a stack. And without a separate stack, our setjmp will save the registers alright - and then we go back to the consumer, and write happily to places on the stack.

By the time we longjmp back to the producer, all the local variables it kept on the stack (as opposed to in registers) may be long overwritten. That's why out of the box, setjmp/longjmp work fine as a try-catch/throw - but not as next/yield.

If, however, we:

    Allocate space for a separate stack
    Make the stack pointer point into that stack before setjmp
    Restore the stack pointer after setjmp
    Return to the caller (consumer)

…then we'll have created a jmp_buf with a stack pointer pointing to our separate space - jmp_buf remembers the stack pointer among the other registers. And then we can jump as we please back and forth between the producer and the consumer - they no longer overwrite each other's on-stack variables.

The actual stack switching is the ugly part of start. Once we accomplish that, and the producer calls next for the first time, bringing us past the if(!setjmp), all that is left is to call the entry point function and report that we're DONE:

  (*p->f)(p->arg);
  longjmp(p->c->caller_context, DONE);

The longjmp brings us back to a context saved by some call to next, and then next sees the DONE returned by setjmp and returns false to the consumer.

And now to the stack switching. Basically, all we need is some inline assembly to change $sp - the stack pointer register, something most machines have. There are two complications though:

    Apart from $sp, there's also the frame pointer (at least in debug builds) - a register pointing into the stack and used to access local variables. So we have to switch that, too.
    Our parameters, such as f and arg, may themselves be on the stack. So how do we access them after switching the stack?

Actually I think that the clean way around these is to write start in pure assembly - where you know where everything is and how to access it and no compiler puts it in different places under different build settings. But I don't like assembly, so instead I did this:

    Use space right below the sp address (the stack grows downwards) to store our parameters (in a start_params structure). The beginning of this structure is kept in p. Together with the parameters, we save the stack pointer and frame pointer values:

    start_params* p = ((start_params*)sp)-1;
    p->c = c;
    p->f = f;
    p->arg = arg;
    get_sp(p->old_sp);
    get_fp(p->old_fp);

    Set the frame pointer to p, and the stack pointer to p - FRAME_SZ (I picked a random constant for FRAME_SZ that I thought was large enough for whatever the compiler may want to keep on the stack - in bytes, I'm reserving 5*sizeof(start_params)). The idea is, the frame pointer is the highest address of our stack frame, and the stack pointer is the lowest address.

    set_sp(p - FRAME_SZ);
    set_fp(p);

    Read the value of p back from the frame pointer register, because once we've changed the value of that register, we no longer trust any local variable value (because the compiler may have put it on the stack so it will try to access it through the frame pointer - which is no longer the same frame pointer that it has just used to store the variable, so it will get garbage.)

    get_fp(p);

    Be careful to access everything through p from on - and restore $sp and $fp before returning to the caller.

That's all; the get/set macros use the GNU inline assembly syntax to get/set the $rsp and $rbp registers of x86-64. The code worked with a bunch of compiler flags for me - but do tell if you see bugs in there.

So that's how coroutines work with setjmp/longjmp and a bit of inline assembly - hopefully, porting to another machine takes little more than rewriting the 4 asm macros.

Update: check out coroutines in standard C - no assembly whatsoever - by Tony Finch (the original article http://fanf.livejournal.com/105413.html and a refined version http://dotat.at/cgi/git/picoro.git not relying on C99 features). The trick is to allocate space for coroutine stacks on your own stack.

This approach may be more or less suitable for you than the kind of code appearing above depending on your situation (for instance, I wrote this stuff in a context where I had to allocate the stacks outside the caller's stack because of the OS stack protection features and my wish to yield in thread A and then call next in thread B).






