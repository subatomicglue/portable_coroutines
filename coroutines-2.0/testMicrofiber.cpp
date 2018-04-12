
#include <stdio.h>
#include <assert.h>
#include <vector>

#include "Microfiber.h"

int numfibers = 200;

#if defined(__x86_64__)
void* big = (void*)0x1234567812345678;
#endif
#if defined(__i386__)
void* big = (void*)0x12345678;
#endif

// example fiber function...
static void* IncrementFoo1( void* iArg )
{
    if (iArg != big) printf( "%lu %lu\n", (size_t)iArg, (size_t)big );
   assert( iArg == big );
    // volatile is only there to make it possible to debug the
    // variable in release builds
    volatile size_t iFoo = 0;
    printf( "IncrementFoo1 enter    | Resume Gave Me:0x%lx | Now Yielding with:0x%lx\n", (size_t)iArg, iFoo );
    iArg = (void*)Microfiber::Yield((void*)iFoo);
    iFoo += (size_t)iArg;
    printf( "IncrementFoo1 reenter1 | Resume Gave Me:0x%lx | Now Yielding with:0x%lx\n", (size_t)iArg, iFoo );
    iArg = (void*)Microfiber::Yield((void*)iFoo);
    iFoo += (size_t)iArg;
    printf( "IncrementFoo1 reenter2 | Resume Gave Me:0x%lx | Now Yielding with:0x%lx\n", (size_t)iArg, iFoo );
    iArg = (void*)Microfiber::Yield((void*)iFoo);
    iFoo += (size_t)iArg;
    printf( "IncrementFoo1 reenter3 | Resume Gave Me:0x%lx | Now Yielding with:0x%lx\n", (size_t)iArg, iFoo );
    iArg = (void*)Microfiber::Yield((void*)iFoo);
    iFoo += (size_t)iArg;
    printf( "IncrementFoo1 reenter4 | Resume Gave Me:0x%lx | Now Yielding with:0x%lx\n", (size_t)iArg, iFoo );
    iArg = (void*)Microfiber::Yield((void*)iFoo);
    iFoo += (size_t)iArg;
    printf( "IncrementFoo1 reenter5 | Resume Gave Me:0x%lx | Now Returning NULL\n", (size_t)iArg );
    return NULL;
}

// example fiber function... (size_t version, verify Microfiber supports both)
static size_t IncrementFoo0( size_t iArg )
{
    if (iArg != (size_t)big) printf( "%lu %lu\n", (size_t)iArg, (size_t)big );
    assert( iArg == (size_t)big );

    // volatile is only there to make it possible to debug the
    // variable in release builds
    volatile size_t iFoo = 0;
    printf( "IncrementFoo0 enter    | Resume Gave Me:%ld | Now Yielding with:%ld\n", iArg, iFoo );
    iArg = Microfiber::Yield(iFoo);
    iFoo += iArg;
    printf( "IncrementFoo0 reenter1 | Resume Gave Me:%ld | Now Yielding with:%ld\n", iArg, iFoo );
    iArg = Microfiber::Yield(iFoo);
    iFoo += iArg;
    printf( "IncrementFoo0 reenter2 | Resume Gave Me:%ld | Now Yielding with:%ld\n", iArg, iFoo );
    iArg = Microfiber::Yield(iFoo);
    iFoo += iArg;
    printf( "IncrementFoo0 reenter3 | Resume Gave Me:%ld | Now Yielding with:%ld\n", iArg, iFoo );
    iArg = Microfiber::Yield(iFoo);
    iFoo += iArg;
    printf( "IncrementFoo0 reenter4 | Resume Gave Me:%ld | Now Yielding with:%ld\n", iArg, iFoo );
    iArg = Microfiber::Yield(iFoo);
    iFoo += iArg;
    printf( "IncrementFoo0 reenter5 | Resume Gave Me:%ld | Now Returning 0\n", iArg );
    return 0;
}

// little test... with asserts to verify correctness.
int test1()
{
    printf( "starting test1()\n" ); fflush(stdout);
    void* iResult1 = 0x0;
    void* iResult2 = 0x0;
    void* iResume1 = 0x0;
    void* iResume2 = 0x0;
    int x = 0;
    Microfiber oFiber0(IncrementFoo1, (void*)big);
    Microfiber oFiber1(IncrementFoo1, (void*)big);

    iResume1 = (void*)big; iResume2 = (void*)big;
    printf( "test1() %d | result1:0x%lx result2:0x%lx | oFiber0.Resume(0x%lx) oFiber1.Resume(0x%lx)\n", ++x, (size_t)iResult1, (size_t)iResult2, (size_t)iResume1, (size_t)iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == (void*)0x0 && iResult2 == (void*)0x0);

    iResume1 = (void*)0x2; iResume2 = (void*)0x22;
    printf( "test1() %d | result1:0x%lx result2:0x%lx | oFiber0.Resume(0x%lx) oFiber1.Resume(0x%lx)\n", ++x, (size_t)iResult1, (size_t)iResult2, (size_t)iResume1, (size_t)iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == (void*)0x2 && iResult2 == (void*)0x22);

    iResume1 = (void*)0x3; iResume2 = (void*)0x33;
    printf( "test1() %d | result1:0x%lx result2:0x%lx | oFiber0.Resume(0x%lx) oFiber1.Resume(0x%lx)\n", ++x, (size_t)iResult1, (size_t)iResult2, (size_t)iResume1, (size_t)iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == (void*)0x5 && iResult2 == (void*)0x55);

    iResume1 = (void*)0x4; iResume2 = (void*)0x44;
    printf( "test1() %d | result1:0x%lx result2:0x%lx | oFiber0.Resume(0x%lx) oFiber1.Resume(0x%lx)\n", ++x, (size_t)iResult1, (size_t)iResult2, (size_t)iResume1, (size_t)iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == (void*)0x9 && iResult2 == (void*)0x99);

    iResume1 = (void*)0x5; iResume2 = (void*)0x55;
    printf( "test1() %d | result1:0x%lx result2:0x%lx | oFiber0.Resume(0x%lx) oFiber1.Resume(0x%lx)\n", ++x, (size_t)iResult1, (size_t)iResult2, (size_t)iResume1, (size_t)iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == (void*)0xe && iResult2 == (void*)0xee);

    iResume1 = (void*)0x9; iResume2 = (void*)0x99;
    printf( "test1() %d | result1:0x%lx result2:0x%lx | oFiber0.Resume(0x%lx) oFiber1.Resume(0x%lx)\n", ++x, (size_t)iResult1, (size_t)iResult2, (size_t)iResume1, (size_t)iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == (void*)0x0 && iResult2 == (void*)0x0);

    printf( "test1() %d | result1:%lx result2:%lx | done\n", x++, (size_t)iResult1, (size_t)iResult2 ); fflush(stdout);
    assert(oFiber0.GetState() == Microfiber::DEAD); // Resuming a dead microfiber always returns 0
    return 0;
}

int test0()
{
    printf( "starting test0\n" ); fflush(stdout);
    volatile size_t iResult1 = 0;
    volatile size_t iResult2 = 0;
    volatile size_t iResume1 = 0;
    volatile size_t iResume2 = 0;
    int x = 0;
    Microfiber oFiber0(IncrementFoo0, (size_t)big);
    Microfiber oFiber1(IncrementFoo0, (size_t)big);

    iResume1 = (size_t)big; iResume2 = (size_t)big;
    printf( "test0() %d | result1:%ld result2:%ld | oFiber0.Resume(%ld) oFiber1.Resume(%ld)\n", x++, iResult1, iResult2, iResume1, iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == 0 && iResult2 == 0);

    iResume1 = 2; iResume2 = 22;
    printf( "test0() %d | result1:%ld result2:%ld | oFiber0.Resume(%ld) oFiber1.Resume(%ld)\n", x++, iResult1, iResult2, iResume1, iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == 2 && iResult2 == 22);

    iResume1 = 3; iResume2 = 33;
    printf( "test0() %d | result1:%ld result2:%ld | oFiber0.Resume(%ld) oFiber1.Resume(%ld)\n", x++, iResult1, iResult2, iResume1, iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == 5 && iResult2 == 55);

    iResume1 = 4; iResume2 = 44;
    printf( "test0() %d | result1:%ld result2:%ld | oFiber0.Resume(%ld) oFiber1.Resume(%ld)\n", x++, iResult1, iResult2, iResume1, iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == 9 && iResult2 == 99);

    iResume1 = 5; iResume2 = 55;
    printf( "test0() %d | result1:%ld result2:%ld | oFiber0.Resume(%ld) oFiber1.Resume(%ld)\n", x++, iResult1, iResult2, iResume1, iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == 14 && iResult2 == 154);

    iResume1 = 9; iResume2 = 99;
    printf( "test0() %d | result1:%ld result2:%ld | oFiber0.Resume(%ld) oFiber1.Resume(%ld)\n", x++, iResult1, iResult2, iResume1, iResume2 ); fflush(stdout);
    iResult1 = oFiber0.Resume(iResume1); iResult2 = oFiber1.Resume(iResume2);
    assert(iResult1 == 0 && iResult2 == 0);

    printf( "test0() %d | result1:%ld result2:%ld | done\n", x++, iResult1, iResult2 ); fflush(stdout);
    assert(oFiber0.GetState() == Microfiber::DEAD); // Resuming a dead microfiber always returns 0
    return 0;
}

// bigger test...
static void* Test2Fiber(void* iArg)
{
   size_t arg = (size_t)iArg;
   int i = 0;
   // wait 1000
   for (int x = 0; x < 4; ++x)
   {
      printf( "fiber waiting (id = %ld)\n", arg );
      Microfiber::Yield((void*)10);
   }

   for (int x = 0; x < 8; ++x)
   {
      // counts down...
      printf( "fiber running (iter = %d id = %ld)\n", ++i, arg );
      Microfiber::Yield((void*)0);
   }

   for (int x = 0; x < 5; ++x)
   {
      printf( "fiber waiting (id = %ld)\n", arg );
      Microfiber::Yield((void*)0);
   }
   return NULL;
}

// a more general fiber manager, let's add 100's, watch them go!
int test2()
{
   std::vector<Microfiber*> fibers;

   // add a ton of fibers...
   for (int x = 0; x < numfibers; ++x)
   {
      fibers.push_back( new Microfiber( Test2Fiber ) );
   }

   // run those fibers...
   bool anyrunning = true;
   while (anyrunning)
   {
      anyrunning = false;
      size_t numFibers = fibers.size();
      for (size_t fiberIt = 0; fiberIt < numFibers; ++fiberIt)
      {
         Microfiber* f = fibers[fiberIt];
         void* result = f->Resume( (void*)fiberIt );
         if (f->GetState() == Microfiber::RUNNING)
            anyrunning = true;
      }
   }
   return 0;
}


int main( int argc, char** argv )
{
   if (argc >= 3)
   {
      numfibers = atoi( argv[2] );
   }
   if (argc >= 2)
   {
      if (strcmp( argv[1], "2" ) == 0)
         return test2();
      if (strcmp( argv[1], "1" ) == 0)
         return test1();
      if (strcmp( argv[1], "0" ) == 0)
         return test0();
   }
   else
   {
      printf( "specify which test to run, 0, 1 or 2\n" );
   }

   return 0;
}

