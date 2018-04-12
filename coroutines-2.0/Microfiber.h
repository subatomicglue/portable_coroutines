#ifndef MICROFIBER_H
#define MICROFIBER_H

#include <stdlib.h>
#include "Koroutines.h"

/// class for multiplatform cooperative threads (aka coroutines)
class Microfiber
{
public:
   /// functions look like this:
   ///     void* MyFiberFunc( void* arg ) {}
   typedef void* (*Function)(void*);
   ///     size_t MyFiberFunc( size_t arg ) {}
   typedef size_t (*FunctionI)(size_t);

   /// default constructor
   inline Microfiber()
   {
      mCoro = NULL;
      mState = DEAD;
   }
   ~Microfiber()
   {
      Delete( mCoro );
   }

   /// function constructor...
   inline Microfiber( Function fn ) { Init( fn ); }
   inline Microfiber( FunctionI fn ) { Init( (Function)fn ); }

   /// function + arg constructor
   inline Microfiber( Function fn, void* arg ) { Init( fn, arg ); }
   inline Microfiber( FunctionI fn, size_t arg ) { Init( (Function)fn, (void*)arg ); }

   /// Initialize the fiber using supplied function
   /// fn() is 1st executed on next call to Resume()...
   /// argument to fn(arg) comes from Resume
   inline void Init( Function fn )
   {
      mCoro = NULL;
      mFunction = fn;
      mState = INITIALIZED; // will use 1st Resume arg to fn()
   }
   inline void Init( FunctionI fn ) { Init( (Function)fn ); }

   /// Initialize the fiber using supplied function (and argument)
   /// fn() is 1st executed on next call to Resume()...
   /// argument to fn(arg) comes from _Init_
   inline void Init( Function fn, void* arg )
   {
      Init( fn );
      mState = RUNNING; // ready to go... arg for fn() is supplied.
      mCoro = ::Create( fn, 8 * 1024, arg );
   }
   inline void Init( FunctionI fn, size_t arg ) { Init( (Function)fn, (void*)arg ); }

   /// Yield is to be called from within a running Fiber
   /// to give up control to another Fiber
   inline static void* Yield( void* arg )
   {
      mYieldRetVal = arg;
      mYieldCalled = true;
      void* a = ::Yield( arg );
      return mYieldRetVal;
   }
   inline static size_t Yield( size_t arg ) { return (size_t)Yield( (void*)arg ); }

   /// Call resume to run the fn()...
   /// argument is passed to the function
   /// 1.) into the argument list, or
   /// 2.) via Yield()'s return value
   void* Resume( void* arg )
   {
      //assert( st1[sizeof( st1 ) - 1] == 0x69 );

      //printf( "Resume\n" );
      switch (mState)
      {
         case INITIALIZED:
            //printf( "INITIALIZED\n" );
            mState = RUNNING;
            mCoro = ::Create( mFunction, 8 * 1024, arg );
            break;
         case DEAD:
            return 0;
         case RUNNING:
            break;
      }

      mYieldRetVal = arg;
      mYieldCalled = false;
      if (IsRunning( mCoro ))
         ::Resume( mCoro, arg );
      //else
      //   mState = DEAD;

      // if we swapped back to here via Yield... (called from fn())
      if (mYieldCalled)
      {
         return mYieldRetVal;
      }

      // if we got here, it's because fn() terminated naturally,
      // rather than calling yield
      mState = DEAD;

      return 0;
   }
   inline size_t Resume( size_t arg ) { return (size_t)Resume( (void*)arg ); }


   enum State { INITIALIZED = 0, RUNNING = 1, DEAD = 2 };

   /// return INITIALIZED, RUNNING or DEAD...
   /// INIT    - function has been set
   /// RUNNING - context has been created for the function, able to run
   /// DEAD    - not set up yet, or, function has exited
   State GetState() const { return mState; }

private:
   Function mFunction;
   Koroutine* mCoro;
   static void* mYieldRetVal;
   static bool mYieldCalled;
   State mState;
};

// TODO: create a MicrofiberCtx out of these, this will let us have more than
//       one collection of fibers running at a time then (i.e. if we wanted
//       to multithread pools of fibers for example).
// ALSO: picoro lib would also need something similar, for its statics.
void* Microfiber::mYieldRetVal = 0;
bool Microfiber::mYieldCalled = false;


#endif
