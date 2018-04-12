/*
 *      win32-ucontext: Unix ucontext_t operations on Windows platforms
 *      Copyright(C) 2007 Panagiotis E. Hadjidoukas
 *
 *      Contact Email: phadjido@cs.uoi.gr, xdoukas@ceid.upatras.gr
 *
 *      win32-ucontext is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      win32-ucontext is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with QueueUserAPCEx in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "ucontext.h"


/* a return handler for func, cleans up func's input function parameters from the stack,
   and then takes the uc_link from the stack, and calls setcontext,
   effectively returning properly from the func
   **noinline: we need an actual function generated
   **naked: the compiler generates code without prolog and epilog code
   */
#define make_return_func(NUM_ARGS)                                               \
static __declspec(noinline) __declspec(naked) void return_func##NUM_ARGS()  \
{                                                                                \
    /* we're returning here from "func" (see input param to makecontext) */      \
    /* on the stack: the input arguments to func, and uc_link, */                \
    /* the goal: retrieve uc_link                              */                \
                                                                                 \
    /* I found the offset '32' by outputting the sp from makecontext */          \
    /* then outputting Esp here, then finding the difference (32).  */           \
    /* when GetThreadContext runs, it pushes this many bytes onto the */         \
    /* stack reads the ESP (stack pointer), then returns that value. */          \
    static const size_t offset_GetThreadCtx = 32;                                \
    /* makecontext always adds 8bytes per argument, so move the SP 8 for ea. */  \
    static const size_t offset_Args = NUM_ARGS * 8;                              \
    /* Get the current SP. */                                                    \
    static CONTEXT ctx;                                                          \
    static ucontext_t** link;                                                    \
    ctx.ContextFlags = CONTEXT_FULL;                                             \
    GetThreadContext( GetCurrentThread(), &ctx );                                \
    /* then read the uc_link from the stack, and setcontext to it */             \
    link = (ucontext_t**)((size_t)ctx.Esp + offset_GetThreadCtx + offset_Args);  \
    setcontext( *link );                                                         \
}

/* fast/assembly/non-portable version of the above function */
#define make_return_func_fast(NUM_ARGS)                                          \
static __declspec(noinline) __declspec(naked) void return_func##NUM_ARGS()  \
{                                                                                \
    /* we're returning here from "func" (see input param to makecontext) */      \
    static size_t uc_link = 0;                                                   \
    static int x = 0;                                                            \
    /* clean up the input arguments to func now, */                              \
    for (x = 0; x < NUM_ARGS; ++x)                                               \
    {                                                                            \
       /* makecontext always adds 8bytes per argument, so move the SP by 8 */    \
       /* TODO: use RAX for 64bit...  (not tested) */                            \
       _asm { _asm pop EAX }                                                     \
       _asm { _asm pop EAX }                                                     \
    }                                                                            \
    /* then read the uc_link from the stack, and setcontext to it */             \
    /* TODO: use RSP for 64bit, (not tested)... */                               \
    _asm { _asm mov [uc_link], ESP }                                             \
    setcontext( (ucontext_t*) *(size_t*)uc_link );                               \
}

/* generate a return handler for each number of arguments anticipated
   if more than size_array( return_func ) is needed,
   you'll need to modify this code */
make_return_func_fast(0)
make_return_func_fast(1)
make_return_func_fast(2)
make_return_func_fast(3)
make_return_func_fast(4)
make_return_func_fast(5)
make_return_func_fast(6)
make_return_func_fast(7)
make_return_func_fast(8)
make_return_func_fast(9)
make_return_func_fast(10)
size_t return_func[11] = {
    (size_t)return_func0,
    (size_t)return_func1,
    (size_t)return_func2,
    (size_t)return_func3,
    (size_t)return_func4,
    (size_t)return_func5,
    (size_t)return_func6,
    (size_t)return_func7,
    (size_t)return_func8,
    (size_t)return_func9,
    (size_t)return_func10,
};
