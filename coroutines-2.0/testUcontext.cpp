#include <stdio.h>

// set/get/make/swapcontext posix functions
// basically does setjmp and longjmp, but also save/restores stack
// http://en.wikipedia.org/wiki/Setcontext
// http://www.opengroup.org/onlinepubs/009695399/functions/makecontext.html
#ifdef WIN32
#  include "ucontext-win32/ucontext.h"
#elif defined USE_PORTABLE_UCONTEXT
extern "C"
{
#include "ucontext-portable/_build/headers/ucontext.h"
}
#else
   // system version
#  include <ucontext.h>
#endif

#if 1
#include <stdlib.h>
#include <assert.h>

#define handle_error(msg) \
           do { perror(msg); exit(0); } while (0)

#define USE_64BIT_MAKECONTEXT 1

static ucontext_t ctx[3];
typedef void (*makecontext_func)(); //< makecontext needs this type...

/*
 * nice example here:
 *     http://pubs.opengroup.org/onlinepubs/009695399/functions/makecontext.html
 * makecontext
       On architectures where int and pointer types are the same size (e.g.,
       x86-32, where both types are 32 bits), you may be able to get away
       with passing pointers as arguments to makecontext() following argc.
       However, doing this is not guaranteed to be portable, is undefined
       according to the standards, and won't work on architectures where
       pointers are larger than ints. starting with version
       2.8, glibc makes some changes to makecontext(), to permit this on
       some 64-bit architectures (e.g., x86-64).
*/

#if defined(__x86_64__) && !defined( USE_64BIT_MAKECONTEXT )
void f1(unsigned int hiArg, unsigned int loArg) // 2 int32s
{
   void* arg = (void*)((long long)hiArg << 32) | (long long)loArg));
#else
void f1(void* arg) // 32bit pointer
{
#endif
    puts("start f1");
    int retval = swapcontext(&ctx[1], &ctx[2]);
    assert( retval == 0 && "shouold be 0" );
    puts("finish f1");
}


#if defined(__x86_64__) && !defined( USE_64BIT_MAKECONTEXT )
void f2(unsigned int hiArg, unsigned int loArg) // 2 int32s
{
   void* arg = (void*)((long long)hiArg << 32) | (long long)loArg));
#else
void f2(void* arg) // 32bit pointer
{
#endif
    puts("start f2");
    int retval = swapcontext(&ctx[2], &ctx[1]);
    assert( retval == 0 && "shouold be 0" );
    puts("finish f2");
}


int main(void)
{
    char st1[8192];
    char st2[8192];
    int arg = 69;
      getcontext(&ctx[0]);

    puts("main 1 : create");
    if (getcontext(&ctx[1]) == -1)
       handle_error("getcontext");
    ctx[1].uc_stack.ss_sp = st1;
    ctx[1].uc_stack.ss_size = sizeof st1;
#ifdef WIN32
    ctx[1].uc_stack.ss_flags = 0;
    ctx[1].uc_link = &ctx[0];
#endif
#if defined(__x86_64__) && !defined( USE_64BIT_MAKECONTEXT )
    {
      unsigned int hiArg = (unsigned int)((long long)arg >> 32);
      unsigned int loArg = (unsigned int)((long long)arg & 0xFFFFFFFF);
      // makecontext only takes int32s, but arg is 64bit
      makecontext( &ctx[1], (makecontext_func)f1, 2, hiArg, loArg );
    }
#else
    makecontext( &ctx[1], (makecontext_func)f1, 1, arg );
#endif
    puts("main 2");


    puts("main 3 : create");
    if (getcontext(&ctx[2]) == -1)
       handle_error("getcontext");
    ctx[2].uc_stack.ss_sp = st2;
    ctx[2].uc_stack.ss_size = sizeof st2;
#ifdef WIN32
    ctx[2].uc_link = &ctx[1];
#endif
#if defined(__x86_64__) && !defined( USE_64BIT_MAKECONTEXT )
    {
       unsigned int hiArg = (unsigned int)((long long)arg >> 32);
       unsigned int loArg = (unsigned int)((long long)arg & 0xFFFFFFFF);
       // makecontext only takes int32s, but arg is 64bit
       makecontext( &ctx[2], (makecontext_func)f2, 2, hiArg, loArg );
    }
#else
    makecontext( &ctx[2], (makecontext_func)f2, 1, arg );
#endif
    puts("main 4 (jump!)");


    swapcontext(&ctx[0], &ctx[2]);
    puts("main 5 done");
    return 0;
}





#else




#include <stdio.h>


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


    getcontext(&ctx[1]);
    ctx[1].uc_stack.ss_sp = st1;
    ctx[1].uc_stack.ss_size = sizeof st1;
    ctx[1].uc_link = &ctx[0];
    makecontext(&ctx[1], f1, 0);


    getcontext(&ctx[2]);
    ctx[2].uc_stack.ss_sp = st2;
    ctx[2].uc_stack.ss_size = sizeof st2;
    ctx[2].uc_link = &ctx[1];
    makecontext(&ctx[2], f2, 0);


    swapcontext(&ctx[0], &ctx[2]);
    return 0;
}

#endif
