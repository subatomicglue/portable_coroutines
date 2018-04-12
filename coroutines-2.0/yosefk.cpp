
/*
A coroutine is thus two contexts: the callee's (producer's), and the caller's (consumer's).

The tricky and non-portable part is initializing these contexts (jmp_buf's). The cleaner and portable part is jumping back and forth between them, so let's see that first:
*/

#include "yosefk.h"
#include "stdio.h"
enum { DIRECTINVOKE=0, WORKING=1, DONE=2 };

/* setjmp - If the return is from a direct invocation, setjmp returns 0.
 *       If the return is from a call to longjmp, setjmp returns a nonzero value.
 */

void yield( coroutine* c ) {
  if (!setjmp( c->callee_context )) {
    longjmp( c->caller_context, WORKING );
  }
}

int resume( coroutine* c ) {
  if (DONE == c->state) return DONE;
  int ret = setjmp( c->caller_context );
  c->state = (ret == WORKING);
  if (!ret /* == DIRECTINVOKE*/) {
    longjmp( c->callee_context, 1 );
  }
  else {
    return c->state;
  }
}


/*
So yield and resume are each just a pair of setjmp/longjmp calls. The trick with setjmp is, it returns 0
right after you call it - having saved the context (machine registers, basically) to your jmp_buf. 
You can later return to this context (restore register values) with longjmp.

Where exactly does longjmp bring you? To the point in code where setjmp has just returned. How do you
know that setjmp has just returned because longjmp brought you there - as opposed to returning directly
after you called setjmp? Because setjmp returns the non-zero value that was passed to longjmp, rather 
than 0 as it does when returning directly. So if(!setjmp) asks, "did I just call you - or was it a long
time ago and we're back from longjmp?"

As you can see, there's almost no difference between yield and resume - both say, "save our current 
context and then go to the other side's context". The only thing setting resume apart is, it reports
whether we're DONE or WORKING.

But, it looks like we're always WORKING, doesn't it? ret is what's passed to longjmp and yield always
passes WORKING. Which brings us to start - the one passing DONE, because, well, the code of start also
contains the part where we're finished.
*/

typedef struct {
  coroutine* c;
  func f;
  void* arg;
  void* old_sp;
  void* old_fp;
} start_params;


#ifdef _WIN64
// kevin:
// see https://github.com/halayli/lthread/blob/master/src/lthread.c
// for _switch function, might be a better alternative than set_sp set_fp
#include <intrin.h>
#include <windows.h>
extern "C" inline __int64 getRSP();
extern "C" inline __int64 getRBP();
extern "C" inline void setRSP(__int64 v);
extern "C" inline void setRBP(__int64 v);
extern "C" inline void setRSPRBP(__int64 v1, __int64 v2);
#define get_sp( var ) var = (void*)getRSP()
#define get_fp( var ) var = (start_params*)getRBP()
#define set_sp( var ) setRSP((__int64)(var))
#define set_fp( var ) setRBP((__int64)(var))
#define set_spfp( var1, var2 ) setRSPRBP((__int64)(var1), (__int64)(var2))
#elif _WIN32
#define get_sp(var) { __int32 t1918; __asm { \
           mov t1918, esp \
   }; var = (void*)t1918; }
#define get_fp(var) { __int32 t1918; __asm { \
      mov t1918, ebp \
   }; var = (start_params*)t1918; }
#define set_sp(var) { __int32 t1918 = (__int32)(var); __asm { \
   mov esp, t1918 \
}; }
#define set_fp(var) { __int32 t1918 = (__int32)(var); __asm { \
   mov ebp, t1918 \
}; }
#define set_spfp(p1, p2) set_sp( p1 ); set_fp( p2 )
#else
#define get_sp(p) \
  __asm__ volatile("movq %%rsp, %0" : "=r"(p))
#define get_fp(p) \
  __asm__ volatile("movq %%rbp, %0" : "=r"(p))
#define set_sp(p) \
  __asm__ volatile("movq %0, %%rsp" : : "r"(p))
#define set_fp(p) \
  __asm__ volatile("movq %0, %%rbp" : : "r"(p))
#define set_spfp(p1, p2) set_sp( p1 ); set_fp( p2 )
#endif

enum { FRAME_SZ=5 }; //fairly arbitrary

void start(coroutine* c, func f, void* arg, void* sp)
{
  c->state = WORKING;
  start_params* p = ((start_params*)sp)-1;

  //save params before stack switching
  p->c = c;
  p->f = f;
  p->arg = arg;
  /*
  CONTEXT context;
  memset( &context, 0, sizeof( CONTEXT ) );
  EXCEPTION_RECORD econtext;
  memset( &econtext, 0, sizeof( EXCEPTION_RECORD ) );
  RtlCaptureContext( &context );
  p->old_sp = (void*)context.Rsp;
  p->old_fp = (void*)context.Rbp;
  */
  get_sp(p->old_sp);
  get_fp(p->old_fp);

  //char** retaddr = (char**)_AddressOfReturnAddress();
  //*retaddr = (char*)(p - FRAME_SZ);
  /*RtlCaptureContext( &context );
  context.Rsp = DWORD64(p - 5);
  context.Rbp = (DWORD64)p;
  econtext.ExceptionAddress = (void*)context.Rsp;
  RtlRestoreContext( &context, &econtext );
  */
  //set_spfp( p - FRAME_SZ, p );
  set_sp(p - FRAME_SZ);
  set_fp(p); //effectively clobbers p
  //(and all other locals)
  /*RtlCaptureContext( &context );
  p = (start_params*)context.Rbp;
  */
  get_fp(p); //…so we read p back from $fp

  //and now we read our params from p
  if(!setjmp(p->c->callee_context)) {
    //set_spfp( p->old_sp, p->old_fp );
     /*RtlCaptureContext( &context );
     context.Rsp = (DWORD64)p->old_sp;
     context.Rbp = (DWORD64)p->old_fp;
     econtext.ExceptionAddress = (void*)context.Rsp;
     RtlRestoreContext( &context, &econtext );
    */
    set_sp(p->old_sp);
    set_fp(p->old_fp);
    return;
  }
  (*p->f)(p->arg);
  longjmp(p->c->caller_context, DONE);
}

/*
This is longer, uglier, non-portable and perhaps not strictly correct (please tell me if you spot bugs - I have close to zero experience with x86-64, it's just what I test on at home.)

So why so ugly? Because we need a separate stack for the coroutine (the producer), and C doesn't have a way to give it a stack. And without a separate stack, our setjmp will save the registers alright - and then we go back to the consumer, and write happily to places on the stack.

By the time we longjmp back to the producer, all the local variables it kept on the stack (as opposed to in registers) may be long overwritten. That's why out of the box, setjmp/longjmp work fine as a try-catch/throw - but not as resume/yield.

If, however, we:

    Allocate space for a separate stack
    Make the stack pointer point into that stack before setjmp
    Restore the stack pointer after setjmp
    Return to the caller (consumer)

…then we'll have created a jmp_buf with a stack pointer pointing to our separate space - jmp_buf remembers the stack pointer among the other registers. And then we can jump as we please back and forth between the producer and the consumer - they no longer overwrite each other's on-stack variables.

The actual stack switching is the ugly part of start. Once we accomplish that, and the producer calls resume for the first time, bringing us past the if(!setjmp), all that is left is to call the entry point function and report that we're DONE:

  (*p->f)(p->arg);
  longjmp(p->c->caller_context, DONE);

The longjmp brings us back to a context saved by some call to resume, and then resume sees the DONE returned by setjmp and returns false to the consumer.

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
*/


