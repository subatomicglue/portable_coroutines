#include <stdio.h>
#include <stdint.h>
// set/get/make/swapcontext posix functions
// basically does setjmp and longjmp, but also save/restores stack
// http://en.wikipedia.org/wiki/Setcontext
// http://www.opengroup.org/onlinepubs/009695399/functions/makecontext.html
#ifdef WIN32
   // local version
#  include "ucontext/ucontext.h"
#  ifdef Yield
#     undef Yield
#  endif
#elif defined(USE_NATIVE_UCONTEXT) && !defined(__APPLE__)
   // system version
#  include <ucontext.h> // coredumps on APPLE... :(
#else
extern "C"
{
#  include "taskimpl.h"
}
#endif


static ucontext_t ctx[3];


static void
f1 (void)
{
    puts("start f1");
    swapcontext(&ctx[1], &ctx[2]);
    puts("finish f1");

    // exit task
    swapcontext(&ctx[1], &ctx[0]);
}


static void
f2 (void)
{
    puts("start f2");
    swapcontext(&ctx[2], &ctx[1]);
    puts("finish f2");

    // exit task
    swapcontext(&ctx[2], &ctx[0]);
}


int
main (void)
{
    char st1[8192];
    char st2[8192];

   puts("main 1");
    getcontext(&ctx[1]);
    ctx[1].uc_stack.ss_sp = st1+8;
    ctx[1].uc_stack.ss_size = sizeof( st1 ) - 64;
    //ctx[1].uc_link = &ctx[0]; // not implemented in all versions of ucontext
  uint64_t z;
  uint32_t x, y;
   z = (uint64_t)6969;
   y = z;
   z >>= 16;	/* hide undefined 32-bit shift from 32-bit compilers */
   x = z>>16;
    makecontext(&ctx[1], f1, 2, y, x);
   puts("main 2");


   puts("main 3");
    getcontext(&ctx[2]);
    ctx[2].uc_stack.ss_sp = st2;
    ctx[2].uc_stack.ss_size = sizeof st2;
    //ctx[2].uc_link = &ctx[1]; // not implemented in all versions of ucontext
   z = (uint64_t)6969;
   y = z;
   z >>= 16;	/* hide undefined 32-bit shift from 32-bit compilers */
   x = z>>16;
    makecontext(&ctx[2], f2, 2, y, x);
   puts("main 4");

   puts("starting coop threads");

   // switch to thread2
   swapcontext(&ctx[0], &ctx[2]);
   puts("main 5");

   // switch to thread1
   swapcontext(&ctx[0], &ctx[1]);
   puts("main 6");

    return 0;
}

