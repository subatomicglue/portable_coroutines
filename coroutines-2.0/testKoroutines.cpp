
#include <stdio.h>
#include <assert.h>
#include <vector>
#include "Koroutines.h"

///////////////////////////////////////////////

size_t stacksize = 8*1024;
size_t numthreads = 200;

void* Fiber0( void* userdata )
{
   for (int x = 0; x < 3; ++x)
   {
      userdata = (void*)(size_t(userdata) + 1);
      printf( "fiber0 %d %ld\n", x, (size_t)userdata );
      userdata = Yield( userdata );
   }
   return userdata;
}

void* Fiber1( void* userdata )
{
   for (int x = 0; x < 3; ++x)
   {
      userdata = (void*)(size_t(userdata) + 100);
      printf( "fiber1 %d %ld\n", x, (size_t)userdata );
      userdata = Yield( userdata );
   }
   return userdata;
}

void test0()
{
   Koroutine* fs0 = Create( Fiber0, stacksize, (void*)NULL );
   Koroutine* fs1 = Create( Fiber1, stacksize, (void*)NULL );
   void* arg0 = 0, *arg1 = 0;

   bool running = true;
   while (running)
   {
      running = false;
      if (IsRunning( fs0 )) { arg0 = Resume( fs0, arg0 ); running = true; }
      if (IsRunning( fs1 )) { arg1 = Resume( fs1, arg1 ); running = true; }
   }
   Delete( fs0 );
   Delete( fs1 );
}

void test1()
{
   std::vector<Koroutine*> co;
   co.reserve(numthreads);
   std::vector<unsigned long> co_data;
   co_data.reserve(numthreads);
   printf( "setup %d threads:\n", numthreads );
   for (size_t x = 0; x < numthreads; ++x)
   {
      printf( "." ); fflush( stdout );
      Koroutine* c = Create( ((x % 2) == 1) ? Fiber0 : Fiber1, stacksize, (void*)NULL );
      if (!c)
      {
         printf( "no koroutine created\n" ); fflush( stdout );
         exit( -1 );
      }
      co.push_back( c );
      co_data.push_back( 0 );
   }


   printf( "running:\n" );
   bool running = true;
   while (running)
   {
      running = false;
      for (size_t x = 0; x < co.size(); ++x)
      {
         if (IsRunning( co[x] ))
         {
            co_data[x] = (unsigned long)Resume( co[x], (void*)co_data[x] );
            running = true;
         }
      }
   }
   printf( "teardown:\n" );
   for (size_t x = 0; x < co.size(); ++x)
   {
      Delete( co[x] );
   }
}


int main( int argc, char** argv )
{
   if (argc == 1)
   {
      printf( "%s numthreads <stacksize>\n", argv[0] );
      exit(-1);
   }
   if (argc > 1)
   {
      numthreads = atoi( argv[1] );
   }
   if (argc > 2)
   {
      stacksize = atoi( argv[2] );
   }

   printf( "running Test1\n" );
   test1();

   printf( "running Test0\n" );
   test0();

   return 0;
}


