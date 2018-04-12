
// KEVIN: I ran with 2000 fibers in x64 release mode, 8*1024 stack size.

#include <windows.h>
#include <intrin.h>
#include <assert.h>
#include <stdio.h>

#ifdef Yield
#undef Yield
#endif

#include "Koroutines.h"


void* pfiber = NULL;

struct Koroutine
{
   Koroutine() : running( true ) {};
   KoroutineFunc func;
   void* fiber;
   bool running;
   void* userdata;
   void* parent;
};
static void __stdcall FiberRunner( void* fs )
{
   Koroutine* f = (Koroutine*)fs;
   f->func( f->userdata );
   f->running = false;
   SwitchToFiber( f->parent );
}


void* Yield( void* arg )
{
   Koroutine* fs = (Koroutine*)GetFiberData();
   fs->userdata = arg;
   SwitchToFiber( pfiber );
   fs = (Koroutine*)GetFiberData();
   return fs->userdata;
}
/*
void Init()
{
   if (!pfiber)
      pfiber = ConvertThreadToFiber( NULL );
   assert( pfiber );
}
*/

static char *                      //   return error message
getLastErrorText(                  // converts "Lasr Error" code into text
char *pBuf,                        //   message buffer
size_t bufSize)                     //   buffer size
{
     size_t retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|
                           FORMAT_MESSAGE_FROM_SYSTEM|
                           FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           NULL,
                           GetLastError(),
                           LANG_NEUTRAL,
                           (LPTSTR)&pBuf,
                           0,
                           NULL );
     assert( retSize < bufSize );
     return pBuf;
}

Koroutine* Create( KoroutineFunc f, size_t stacksize, void* arg )
{
   if (!pfiber)
      pfiber = ConvertThreadToFiber( NULL );
   assert( pfiber );

   Koroutine* fs = new Koroutine;
   if (!fs) return NULL;
   fs->func = f;
   fs->fiber = CreateFiberEx( stacksize, stacksize, 0/*FIBER_FLAG_FLOAT_SWITCH*/, FiberRunner, fs);
   if (!fs->fiber)
   {
      const int bufsize = 1024;
      char buf[bufsize];
      LPCWSTR b = (LPCWSTR)getLastErrorText( buf, bufsize );
      OutputDebugString( b );
      printf( "CreateFiber: %s\n(The number of fibers a process can create is limited by the available virtual memory. By default, every fiber has 1 megabyte of reserved stack space. Therefore, you can create at most 2028 fibers.)\n", (char*)b ); fflush( stdout );
      __debugbreak();
      delete fs;
      return NULL;
   }
   fs->parent = pfiber;
   fs->userdata = arg;
   return fs;
}


void* Resume( Koroutine* fs, void* arg )
{
   assert( fs );
   fs->userdata = arg;
   SwitchToFiber( fs->fiber );
   return fs->userdata;
}

void Delete( Koroutine* &fs )
{
   if (fs)
   {
      DeleteFiber( fs->fiber );
      delete fs;
      fs = NULL;
   }
}

bool IsRunning( Koroutine* co )
{
   return co && co->running;
}

/*
void Release()
{
   //DeleteFiber( pfiber );
   //pfiber = NULL;
}
*/

