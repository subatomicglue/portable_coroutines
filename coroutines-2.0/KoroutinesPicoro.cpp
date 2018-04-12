
#include "stdlib.h"
#include "Koroutines.h"

#include <stdio.h>
#include <assert.h>

namespace picoro
{
#include "picoro.h"
}

struct Koroutine
{
   picoro::coro* co;
   KoroutineFunc func;
   void* userdata;
};

void Init() {}
void Release() {}

Koroutine* Create( KoroutineFunc f, size_t stacksize, void* arg )
{
   Koroutine* c = new Koroutine;
   c->co = picoro::coroutine( f, stacksize );
   c->func = f;
   c->userdata = arg;
   return c;
}

void Delete( Koroutine* &fs ) {}

void* Yield( void* arg )
{
   return picoro::yield( arg );
}
void* Resume( Koroutine* k, void* arg )
{
   k->userdata = arg;
   k->userdata = picoro::resume( k->co, arg );
   return k->userdata;
}
bool IsRunning( Koroutine* k )
{
   return picoro::resumable( k->co );
}


