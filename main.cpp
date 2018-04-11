

#include <setjmp.h>
#include <stdio.h>
#include <assert.h>

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


/// class for multiplatform cooperative threads (aka coroutines)
class Microfiber
{
public:
   /// functions look like this:
   ///     void MyFiberFunc( size_t arg ) {}
   typedef void (*Function)(size_t);

   /// default constructor
   inline Microfiber()
   {
      mState = DEAD;
   }

   /// function constructor...
   inline Microfiber( Function fn ) { Init( fn ); }
   inline Microfiber( Function fn, size_t arg ) { Init( fn, arg ); }

   /// Initialize the fiber using supplied function
   /// fn() is 1st executed on next call to Resume()...
   /// argument to fn(arg) comes from Resume
   inline void Init( Function fn )
   {
      mFunction = fn;
      mState = INITIALIZED; // will use 1st Resume arg to fn()
   }

   /// Initialize the fiber using supplied function (and argument)
   /// fn() is 1st executed on next call to Resume()...
   /// argument to fn(arg) comes from _Init_
   inline void Init( Function fn, size_t arg )
   {
      Init( fn );
      mState = RUNNING; // ready to go... arg for fn() is supplied.

      getcontext( &mFuncJmp );
      mFuncJmp.uc_stack.ss_sp = st1;
      mFuncJmp.uc_stack.ss_size = sizeof st1;
      //mFuncJmp.uc_link = &mMainJmp;
      uint64_t z = arg;
      uint32_t y = z;
      z >>= 16;	/* hide undefined 32-bit shift from 32-bit compilers */
      uint32_t x = z>>16;
      makecontext(&mFuncJmp, (FunctionCast)fn, 2, y, x);
   }

   /// Yield is to be called from within a running Fiber
   /// to give up control to another Fiber
   inline static size_t Yield( size_t arg )
   {
      mYieldRetVal = arg;
      mYieldCalled = true;
      swapcontext( &mFuncJmpFromYield, &mMainJmp );
      return mYieldRetVal;
   }

   /// Call resume to run the fn()...
   /// argument is passed to the function via Yield()'s return value
   size_t Resume( size_t arg )
   {
      //printf( "Resume\n" );
      switch (mState)
      {
         case INITIALIZED:
            {
            //printf( "INITIALIZED\n" );
            mState = RUNNING;

            getcontext( &mFuncJmp );
            mFuncJmp.uc_stack.ss_sp = st1+8; // pad
            mFuncJmp.uc_stack.ss_size = sizeof( st1 ) - 64; // pad
            //mFuncJmp.uc_link = &mMainJmp;
            uint64_t z = arg;
            uint32_t y = z;
            z >>= 16;	/* hide undefined 32-bit shift from 32-bit compilers */
            uint32_t x = z>>16;
            makecontext(&mFuncJmp, (FunctionCast)mFunction, 2, y, x);
            break;
            }
         case RUNNING:
            break;
         case DEAD:
            return 0;
      }

      mYieldRetVal = arg;
      mYieldCalled = false;
      //printf( "swapcontext\n" );
      swapcontext( &mMainJmp, &mFuncJmp );

      //printf( "hi\n" );
      // if we swapped back to here via Yield... (called from fn())
      if (mYieldCalled)
      {
      //printf( "hi2\n" );
         mFuncJmp = mFuncJmpFromYield;
         return mYieldRetVal;
      }
      //printf( "hi3\n" );

      // if we got here, it's because fn() terminated naturally,
      // rather than calling yield
      mState = DEAD;

      return 0;
   }

   enum State { INITIALIZED = 0, RUNNING = 1, DEAD = 2 };

   /// return INITIALIZED, RUNNING or DEAD...
   /// INIT    - function has been set
   /// RUNNING - context has been created for the function, able to run
   /// DEAD    - not set up yet, or, function has exited
   State GetState() const { return mState; }

private:
   typedef void (*FunctionCast)(); //< makecontext needs this type...
   Function mFunction;
   ucontext_t mFuncJmp;
   static ucontext_t mFuncJmpFromYield;
   static ucontext_t mMainJmp;
   char st1[8192];
   static size_t mYieldRetVal;
   static bool mYieldCalled;
   State mState;
};

// TODO: create a MicrofiberCtx out of these, this will let us have more than
//       one collection of fibers running at a time then (i.e. if we wanted
//       to multithread pools of fibers for example).
ucontext_t Microfiber::mFuncJmpFromYield;
ucontext_t Microfiber::mMainJmp;
size_t Microfiber::mYieldRetVal = 0;
bool Microfiber::mYieldCalled = false;


#ifndef TEST2










////////////////////////////////////////////////////////////////////////////////
// test:

// example fiber function...
static void IncrementFoo0( size_t iArg )
{
    // volatile is only there to make it possible to debug the
    // variable in release builds
    volatile size_t iFoo = 0;
    printf( "IncrementFoo0 %ld\n", iArg );
    iFoo += iArg;
    printf( "IncrementFoo0 hi %ld\n", iArg );
    iArg = Microfiber::Yield(iFoo);
    printf( "IncrementFoo0 hi 0 %ld\n", iArg );
    iFoo += iArg;
    printf( "IncrementFoo0 hi 1 %ld\n", iArg );
    iArg = Microfiber::Yield(iFoo);
    iFoo += iArg;
    printf( "IncrementFoo0 hi 2 %ld\n", iArg );
    iArg = Microfiber::Yield(iFoo);
    iFoo += iArg;
    printf( "IncrementFoo0  Returning\n" );
}

// little test...
int main()
{
    volatile size_t iResult = 0;
    volatile size_t iResult2 = 0;
    Microfiber oFiber0(IncrementFoo0, 1);
    Microfiber oFiber1(IncrementFoo0, 11);

    printf( "main resume 1, %ld %ld\n", iResult, iResult2 );
    iResult = oFiber0.Resume(1);
    iResult2 = oFiber1.Resume(11);
    printf( "main resume 2, %ld %ld\n", iResult, iResult2 );
    assert(iResult == 1);
    iResult2 = oFiber1.Resume(22);
    iResult = oFiber0.Resume(2);
    printf( "main resume 3, %ld %ld\n", iResult, iResult2 );
    assert(iResult == 3);
    iResult = oFiber0.Resume(3);
    iResult2 = oFiber1.Resume(33);
    printf( "main resume 4, %ld %ld\n", iResult, iResult2 );
    assert(iResult == 6);
    iResult2 = oFiber1.Resume(44);
    iResult = oFiber0.Resume(4);
    printf( "main resume 5, %ld %ld\n", iResult, iResult2 );
    //assert(iResult == 10); // no retvals in functions... :(
    iResult = oFiber0.Resume(5);
    iResult2 = oFiber1.Resume(55);
    printf( "main done\n" );
    assert(oFiber0.GetState() == Microfiber::DEAD); // Resuming a dead microfiber always returns 0
    return 0;
}

// scroll down for other main() (there are 2 in this file...)

#elif defined TEST2











////////////////////////////////////////////////////////////////////////////////
// bigger test...
#include <vector>

static void Fiber(size_t arg)
{
   int i = 0;
   // wait 1000
   for (int x = 0; x < 1000; ++x)
   {
      printf( "fiber waiting (id = %ld)\n", arg );
      Microfiber::Yield(1000);
   }

   for (int x = 0; x < 1000; ++x)
   {
      // counts down...
      printf( "fiber running (iter = %d id = %ld)\n", ++i, arg );
      Microfiber::Yield(0);
   }

   while (1)
   {
      printf( "fiber waiting (id = %ld)\n", arg );
      Microfiber::Yield(0);
   }
}

int main()
{
   std::vector<Microfiber*> fibers;

   // add a ton of fibers...
   for (int x = 0; x < 200; ++x)
   {
      fibers.push_back( new Microfiber(Fiber) );
   }

   // run those fibers...
   while (1)
   {
      int numFibers = fibers.size();
      for (int fiberIt = 0; fiberIt < numFibers; ++fiberIt)
      {
         Microfiber* f = fibers[fiberIt];
         size_t result = f->Resume( (unsigned long int)f );
      }
   }
   return 0;
}

// scroll up for other main() (there are 2 in this file...)

#endif
