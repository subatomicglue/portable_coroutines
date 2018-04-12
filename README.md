# cross platform fibers/coroutines in c++ v1.0

here we present an example implementation for portable fibers/coroutines in c++.
in this example, a class called `Microfiber` is implemented (see [`main.cpp`](main.cpp)).

[wikipedia (coroutine)](http://en.wikipedia.org/wiki/Coroutine):
*"coroutines are program components that generalize
subroutines to allow multiple entry points for suspending and resuming
of execution at certain locations."*

[wikipedia (fiber)](http://en.wikipedia.org/wiki/Fiber_(computer_science))</a>:
*"a fiber is a particularly lightweight thread of execution. 
Like threads, fibers share address space; where a distinction exists, it
is that fibers use co-operative multitasking while threads use
pre-emptive multitasking."*

NOTES (added 2018):
- Since this released in 2008, there was a 2nd iteration beyond this (~2012), which you can find in [v2.0](coroutines-2.0)
- Both are an exploration / example level code.
- You might find them interesting.
- There is always [`Boost::Context`](https://www.boost.org/doc/libs/1_66_0/libs/context/doc/html/index.html), which seems to do a production quality job of this.
- Feel free to send pull requests / discuss in the issue tracker if you want to play.  This stuff is interesting.

## motivation
our motivation here is to create a cooperatively multthreaded construct,
that runs in a single (system) thread...  that sounds strange, so let's explain.
we want a system with hundreds of "processes" (tasks really) but no
syncronization problems, and for each process to control how much time
it gets

Benefits:
- no preemption
   - tasks context switch manually
   - no syncronization to worry about - no semaphore/mutex/etc.
- but just like with preemption
   - switching can `yield` and `resume` in the middle of the task.
- complete control over what (system) thread the tasks run inside of

One use-case for this would be game actors.  where each actor
runs a function over the course of it's life time.  the 
benefits are thus: the actor's update function simply runs over the
lifetime of the game, over several frame updates. The function does not
need to reenter, so there is no complex tracking of state to get back to
where we left off last time the actor was serviced (common in systems that
call actor.update() each frame for example).


## dependencies

the code depends only upon the POSIX [ucontext](http://en.wikipedia.org/wiki/Setcontext) interface.  `ucontext` is
similar to `setjmp`/`longjump` `C` functions, except that they _also_ preserve
the stack pointer, meaning that local variables are preserved.

the code *should* be portable to any POSIX compliant system, though not all unix systems support `ucontext` so YMMV.

the code has been tested under:
- WinXP SP3 32bit using vc++ 2005
- Linux 32bit and 64bit using gcc
- MacOSX 32bit and 64bit using clang's gcc/g++ aliases (clang-900.0.39.2)

NOTE: POSIX group _has_ deprecated ucontext, however, there is no replacement in the wild!  We have (here in the repo) source for `ucontext`, derived from Russ Cox @ MIT's version from Plan9 `libthread` and his `libtask` as well as xdoukas's win32 `ucontext`.  Several platforms supported for `ucontext` through these efforts.

## what does it look like?
```
static void Fiber(size_t arg)
{
   int i = 0; // local variables are preserved across Yields!
   // wait for 1000
   for (int x = 0; x < 1000; ++x)
   {
      printf( "fiber waiting (id = %d)\n", arg );
      Microfiber::Yield(0); // change threads/suspend
   }

   // counts down...
   for (int x = 0; x < 1000; ++x)
   {
      printf( "fiber running (iter = %d id = %d)\n", ++i, arg );
      Microfiber::Yield(0); // change threads/suspend
   }

   while (1)
   {
      printf( "fiber waiting (id = %d)\n", arg );
      Microfiber::Yield(0); // change threads/suspend
   }
}

// main entry point
int main()
{
   // add a ton of fibers that each run a Fiber() function...
   std::vector<Microfiber*> fibers;
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
         fibers[fiberIt]->Resume( 0 );
      }
   }
   return 0;
}
```

## usage - class `Microfiber`
```
/// class for multiplatform cooperative threads (aka coroutines)
class Microfiber
{
public:
   /// functions look like this:
   ///     void MyFiberFunc( size_t arg ) {}
   typedef void (*Function)(size_t);

   /// default constructor
   Microfiber();

   /// function constructor...
   Microfiber( Function fn );
   Microfiber( Function fn, size_t arg );

   /// Initialize the fiber using supplied function
   /// fn() is 1st executed on next call to Resume()...
   /// argument to fn(arg) comes from Resume
   void Init( Function fn );

   /// Initialize the fiber using supplied function (and argument)
   /// fn() is 1st executed on next call to Resume()...
   /// argument to fn(arg) comes from _Init_
   void Init( Function fn, size_t arg );

   /// Yield is to be called from within a running Fiber
   /// to give up control to another Fiber
   static size_t Yield( size_t arg );

   /// Call resume to run the fn()...
   /// argument is passed to the function via Yield()'s return value
   size_t Resume( size_t arg );

   enum State { INITIALIZED = 0, RUNNING = 1, DEAD = 2 };

   /// return INITIALIZED, RUNNING or DEAD...
   /// INIT    - function has been set
   /// RUNNING - context has been created for the function, able to run
   /// DEAD    - not set up yet, or, function has exited
   State GetState() const;
};
```

-------------------------------------------------------------------------




## what's included

**main-jmp.cpp**

this illustrates why `setjmp` and `longjmp` do not work (alone) for coroutines
the problem is that the stack is not preserved across fibers, so we get
garbage in local variables...

to solve this we'd like to save the stack off and restore it, between swaps.
we could write assembly code to save/restore the stack (non portable), or use
the POSIX `swapcontext()` (which we do, in `main.cpp`)


**main.cpp**

portable implementation of `Microfiber` (coroutines), built on POSIX `swapcontext`
(may need porting to non-POSIX systems, `WIN32` version is supplied)

[win32-ucontext](http://www.codeproject.com/KB/threads/ucontext.aspx) (for WIN32)
this is a copy of the `WIN32` version of `ucontext` by [xdoukas](http://www.codeproject.com/script/Membership/Profiles.aspx?mid=81879) from [codeproject](http://www.codeproject.com).
it has been extended to support proper [uc_link](http://www.opengroup.org/onlinepubs/009695399/functions/makecontext.html) `return` functionality,
and faster calls through inline code.

## future work

for systems where impossible to provide the POSIX swapcontext routines,
the Microfiber implementation inside `main-jmp.cpp` (which depends only on standard C's `setjmp`/`longjump`) can be fixed with stack save/restore routines.  Write these in machine specific assembly, or try looking at the WIN32 ucontext directory for inspiration - they use the system's threading calls to save/load the stack
   (i.e. `Set`/`GetThreadContext` on `WIN32`)

Please bang on this.  Submit pull requests.  Disscuss in the issue tracker.

TODO:
- The 64bit windows version needs implementation, asm version (see `libthread` for x64 flavors for other OSs for inspiration)
- Proper cmake build system needed
  - cross compiling to Win32 from Mac/Linux would be nice [see this toolchain](https://github.com/subatomicglue/cross-compile-macosx-clang-windows-msvc) which could be used.
- `uc_link` isn't implemented in the `libtask` version of `ucontext`, for proper return handling from the task.  (see `win32` version for inspiration)
- integrate / unify `libthread` and `libtask` and win32 `ucontext` directories into one portable `ucontext` implementation.   Maybe remove whatever POSIX group hates about it's API (they did deprecate it), probably the varargs on makecontext :)
- testing on multiple systems, 32 vs 64bit on each.
  - Windows 32/64
  - MacOSX 32/64
  - Linux 32/64 (Ubuntu, etc.)
  - others...

## license
```
c++ coroutines - example of cooperative threading w/ Microfiber impl
Copyright (c) 2008 kevin meinert all rights reserved

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301  USA
```

