#include <stdio.h>
// set/get/make/swapcontext posix functions
// basically does setjmp and longjmp, but also save/restores stack
// http://en.wikipedia.org/wiki/Setcontext
// http://www.opengroup.org/onlinepubs/009695399/functions/makecontext.html
#ifdef WIN32
   // local version
#  include "ucontext/ucontext.h"
#else
   // system version
#  include <ucontext.h>
#endif


static ucontext_t ctx[3];


static void
f1 (void)
{
    puts("start f1");
    swapcontext(&ctx[1], &ctx[2]);
    puts("finish f1");
}


static void
f2 (void)
{
    puts("start f2");
    swapcontext(&ctx[2], &ctx[1]);
    puts("finish f2");
}


int
main (void)
{
    char st1[8192];
    char st2[8192];

   puts("main 1");
    getcontext(&ctx[1]);
    ctx[1].uc_stack.ss_sp = st1;
    ctx[1].uc_stack.ss_size = sizeof st1;
    ctx[1].uc_link = &ctx[0];
    makecontext(&ctx[1], f1, 0);
   puts("main 2");


   puts("main 3");
    getcontext(&ctx[2]);
    ctx[2].uc_stack.ss_sp = st2;
    ctx[2].uc_stack.ss_size = sizeof st2;
    ctx[2].uc_link = &ctx[1];
    makecontext(&ctx[2], f2, 0);
   puts("main 4");


    swapcontext(&ctx[0], &ctx[2]);
   puts("main 5");
    return 0;
}

