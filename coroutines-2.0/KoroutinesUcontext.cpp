
#include "stdlib.h"
#include "Koroutines.h"

#include <stdio.h>
#include <assert.h>

// set/get/make/swapcontext posix functions
// basically does setjmp and longjmp, but also save/restores stack
// http://en.wikipedia.org/wiki/Setcontext
// http://www.opengroup.org/onlinepubs/009695399/functions/makecontext.html
#ifdef WIN32
#  include "ucontext-win32/ucontext.h"
#elif defined USE_PORTABLE_UCONTEXT
extern "C"
{
#  include "ucontext-portable/_build/headers/ucontext.h"
}
#else
   // system version
#  include <ucontext.h>
#endif

//#include <setjmp.h>


struct Koroutine
{
   typedef void (*FunctionCast)(); //< makecontext needs this type...
   KoroutineFunc func;
   ucontext_t mFuncJmp;
   char st1[8192]; // stack size
   void* userdata;
   bool running;
};

static Koroutine* mCurrentKoroutine = NULL;
static ucontext_t mFuncJmpFromYield;
static ucontext_t mMainJmp;
static size_t mYieldRetVal;
static bool mYieldCalled;

void Init() {}
void Release() {}

Koroutine* Create( KoroutineFunc f, size_t stacksize, void* arg )
{
   Koroutine* k = new Koroutine;
   getcontext( &k->mFuncJmp );
   k->mFuncJmp.uc_stack.ss_sp = k->st1;
   k->mFuncJmp.uc_stack.ss_size = sizeof( k->st1 );
#if defined(WIN32) || defined(USE_PORTABLE_UCONTEXT)
   //k->mFuncJmp.ss_flags = 0;
   k->mFuncJmp.uc_link = (__ucontext*)&mMainJmp;
#endif
   makecontext( &k->mFuncJmp, (Koroutine::FunctionCast)f, 1, arg);
   k->func = f;
   k->userdata = arg;
   k->running = true;
   return k;
}

void Delete( Koroutine* &fs ) {}

void* Yield( void* arg )
{
   mYieldRetVal = (size_t)arg;
   mYieldCalled = true;
   swapcontext( &mFuncJmpFromYield, &mMainJmp );
   return (void*)mYieldRetVal;
}
void* Resume( Koroutine* k, void* arg )
{
   mYieldRetVal = (size_t)arg;
   mYieldCalled = false;
   //printf( "swapcontext\n" );
   swapcontext( &mMainJmp, &k->mFuncJmp );

   // if we swapped back to here via Yield... (called from fn())
   if (mYieldCalled)
   {
      //printf( "hi2\n" );
      k->mFuncJmp = mFuncJmpFromYield;
      return (void*)mYieldRetVal;
   }
   k->running = false;
   return NULL;
}
bool IsRunning( Koroutine* k )
{
   return k && k->running;
}


