
#include <stdlib.h>
#include <cstring>
#include "Koroutines.h"

#include <stdio.h>
#include <assert.h>

#include "yosefk.h"

struct Koroutine
{
   coroutine* co;
   KoroutineFunc func;
   void* userdata;
   int stacksize;
   char* stack;
};

void Init() {}
void Release() {}

Koroutine* Create( KoroutineFunc f, size_t stacksize, void* arg )
{
   Koroutine* c = new Koroutine;
   assert( c );
   c->co = new coroutine();
   assert( c->co );
   c->stack = new char[stacksize];
   assert( c->stack );
   memset( c->stack, 0, stacksize );
   c->func = f;
   c->userdata = arg;
   start( c->co, f, arg, c->stack + stacksize );
   return c;
}

void Delete( Koroutine* &fs )
{
   delete [] fs->stack; fs->stack = NULL; fs->stacksize = 0;
   delete fs->co; fs->co = NULL;
   delete fs;     fs = NULL;
}

static Koroutine* mCurrentKoroutine = NULL;

void* Yield( void* arg )
{
   yield( mCurrentKoroutine->co );
   return arg;
}

void* Resume( Koroutine* k, void* arg )
{
   mCurrentKoroutine = k;
   k->userdata = arg;
   if (k->co->state)
      resume( k->co );
   return k->userdata;
}
bool IsRunning( Koroutine* k )
{
   return k && k->co && k->co->state;
}


