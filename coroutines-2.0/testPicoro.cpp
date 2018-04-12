
#include <stdio.h>
#include <assert.h>
#include "picoro.h"


void* test1(void *arg) {
   int localstack_var = 45;
   for( int x = 86; 80 <= x; --x)
   {
      assert( localstack_var == 45 );
      printf( "test1 %d 0x%.8lX\n", x, (unsigned long)arg );
      arg = yield( (void*)1 );
   }
   return (void*)100;
}

void* test2(void *arg) {
   long int localstack_var = 3456245;
   for( int x = 0; x < 4; ++x)
   {
      assert( localstack_var == 3456245 );
      printf( "test2 %d 0x%.8lX\n", x, (unsigned long)arg );
      arg = yield( (void*)2 );
   }
   return (void*)101;
}

int main(void) {
   coro* c1 = coroutine( &test1, 8 * 1024 ); printf( "created #1\n" );
   coro* c2 = coroutine( &test2, 8 * 1024 ); printf( "created #2\n" );
   void* arg = (void*)3;
   bool done = false;
   printf( "Go!\n" );
   while (!done)
   {
      if (resumable( c1 ))
         arg = resume( c1, arg );
      if (resumable( c2 ))
         arg = resume( c2, arg );
      done = !resumable( c1 ) && !resumable( c2 );
   }
   return 0;
}

