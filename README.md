# cross platform fibers/coroutines in c++
here we present an example implementation for portable fibers/coroutines in c++.
in this example, a class called `Microfiber` is implemented.

[wikipedia (coroutine)](http://en.wikipedia.org/wiki/Coroutine):
*"coroutines are program components that generalize
subroutines to allow multiple entry points for suspending and resuming
of execution at certain locations."*

[wikipedia (fiber)](http://en.wikipedia.org/wiki/Fiber_(computer_science))</a>:
*"a fiber is a particularly lightweight thread of execution. 
Like threads, fibers share address space; where a distinction exists, it
is that fibers use co-operative multitasking while threads use
pre-emptive multitasking."*

## motivation
our motivation here is to create a cooperatively multthreaded construct,
that runs in a single thread...  that sounds strange, so let's explain.
we want a system with hundreds of processes but no syncronization
problems, and for each process to control how much time it gets (no
preemption).

an obvious use-case for this would be game actors.  where each actor
runs a function over the course of it's life time.  the 
benefits are thus: the actor's update function simply runs over the
lifetime of the game, over several frame updates. The function does not
reenter, so there is no complex tracking of state to get back to where we left off last time the
actor was serviced (common in systems that call actor.update() each
frame for example).   

## dependencies

the code depends only upon the POSIX [ucontext](http://en.wikipedia.org/wiki/Setcontext) interface.  `ucontext` is
similar to `setjmp`/`longjump` `C` functions, except that they _also_ preserve
the stack pointer, meaning that local variables are preserved.

the code *should* be portable to any POSIX compliant system, though not all unix systems support `ucontext` so YMMV.

the code has been tested under:
- WinXP SP3 32bit using vc++ 2005
- Linux 32bit and 64bit using gcc

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

## CONTACT / PULL REQUESTS / HELP WANTED</h2>

Please bang on this.  Submit pull requests.  Disscuss in the issue tracker.

TODO:
- The 64bit windows version needs work, stack manip
- Proper cmake build system needed


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

