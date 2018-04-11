
// this illustrates why setjmp and longjmp do not work (alone) for coroutines
// the problem is that the stack is not preserved across fibers, so we get
// garbage in local variables...
// to solve this we'd like to save the stack off and restore it, between swaps.
// we could write assembly code (non portable), or use the POSIX swapcontext()
// see main.cpp for portable implementation (may need porting to non-POSIX
// systems)

#include <setjmp.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
//#include <ucontext.h> // set/get/make/swapcontext posix functions

class Microfiber
{
   public:
   Microfiber( size_t (*foo)(size_t) )
   {
      mNeedToCallFunc = true;
      mFunction = foo;
   }
   static size_t Yield( size_t arg )
   {
      printf( "yield(%ld) ", arg );
      size_t retval = setjmp( mFuncJmpTemp );
      if (0 == retval)
      {
         printf( "longjmp %ld\n", arg );
         longjmp( mMainJmp, arg );
      }
      printf( " Yield returns %ld\n", retval );
      return retval;
   }

   size_t Resume( size_t arg )
   {
      printf( "Resume(%ld) ", arg );
      size_t retval = setjmp( mMainJmp );
      memcpy( mFuncJmpLocal, mFuncJmpTemp, sizeof( jmp_buf ) );
      if (0 == retval)
      {
         if (!mNeedToCallFunc)
         {
            printf( "longjmp %ld\n", arg );
            longjmp( mFuncJmpLocal, arg );
         }
         else
         {
            mNeedToCallFunc = false;
            printf( "mFunction %ld\n", arg );
            return mFunction( arg );
         }
      }
      printf( " Resume returns %ld\n", retval );
      return retval;
   }
   size_t (*mFunction)(size_t);
   jmp_buf mFuncJmpLocal;
   static jmp_buf mFuncJmpTemp;
   static jmp_buf mMainJmp;
   bool mNeedToCallFunc;
};

jmp_buf Microfiber::mFuncJmpTemp;
jmp_buf Microfiber::mMainJmp;

static size_t IncrementFoo0(size_t iArg)
{// volatile is only there to make it possible to debug the variable in release builds
    volatile size_t iFoo = 0;
    printf( "IncrementFoo0 hi\n" );
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
    return iFoo;
}

static size_t IncrementFoo1(size_t iArg)
{// volatile is only there to make it possible to debug the variable in release builds
    volatile size_t iFoo = 0;
    printf( "IncrementFoo1 hi\n" );
    iFoo += iArg;
    printf( "IncrementFoo1 hi %ld  iFoo==%ld\n", iArg, iFoo );
    iArg = Microfiber::Yield(iFoo);
    printf( "IncrementFoo1 hi 0 %ld iFoo==%ld\n", iArg, iFoo );
    iFoo += iArg;
    printf( "IncrementFoo1 hi 1 %ld iFoo==%ld\n", iArg, iFoo );
    iArg = Microfiber::Yield(iFoo);
    iFoo += iArg;
    printf( "IncrementFoo1 hi 2 %ld\n", iArg );
    iArg = Microfiber::Yield(iFoo);
    iFoo += iArg;
    printf( "IncrementFoo1  Returning\n" );
    return iFoo;
}

int main()
{
    volatile size_t iResult = 0;
    volatile size_t iResult2 = 0;
    Microfiber oFiber0(IncrementFoo0);
    Microfiber oFiber1(IncrementFoo1);

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
    assert(iResult == 10);
    iResult = oFiber0.Resume(5);
    iResult2 = oFiber1.Resume(55);
    printf( "main done\n" );
    assert(iResult == 0); // Resuming a dead microfiber always returns 0
    return 0;
}
