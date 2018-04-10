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

#ifndef UCONTEXT_DOT_H
#define UCONTEXT_DOT_H

#include <assert.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct __stack {
	void *ss_sp;
	size_t ss_size;
	int ss_flags;
} stack_t;

typedef CONTEXT mcontext_t;
typedef unsigned long __sigset_t;

typedef struct __ucontext {
	unsigned long int	uc_flags;
	struct __ucontext	*uc_link;
	stack_t				uc_stack;
	mcontext_t			uc_mcontext; 
	__sigset_t			uc_sigmask;
} ucontext_t;

int getcontext(ucontext_t *ucp);
int setcontext(const ucontext_t *ucp);
int makecontext(ucontext_t *, void (*)(), int, ...);
int swapcontext(ucontext_t *, const ucontext_t *);

__inline int getcontext(ucontext_t *ucp)
{
    int ret;

    // get machine context
    ucp->uc_mcontext.ContextFlags = CONTEXT_FULL;
    ret = GetThreadContext( GetCurrentThread(), &ucp->uc_mcontext );

    return ret == 0 ? -1: 0;
}

__inline int setcontext(const ucontext_t *ucp)
{
    int ret;
    // save machine context (already set)
    ret = SetThreadContext( GetCurrentThread(), &ucp->uc_mcontext );
    return ret == 0 ? -1: 0;
}


/* return helpers, for uc_link */
extern size_t return_func[11];


__inline int makecontext(ucontext_t *ucp, void (*func)(), int argc, ...)
{
    int i;
    va_list ap;

    /* Stack grows down */
    char* sp = (char*)(size_t) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size;
    //char* sp_orig = sp;
#ifdef DEBUG
    memset( ucp->uc_stack.ss_sp, 0x69, ucp->uc_stack.ss_size );
#endif
    /* Reserve stack space for the uc_link ptr */
    sp -= sizeof(size_t);

    /* Reserve stack space for the arguments (maximum possible: argc*(8 bytes per argument)) */
    sp -= argc*8;

    /* Reserve stack space for the "uc_link" return function (called after func returns) */
    sp -= sizeof(size_t);

    if ( sp < (char*)ucp->uc_stack.ss_sp) {
        /* errno = ENOMEM;*/
        return -1;
    }

    /* Set the instruction and the stack pointer */
    ucp->uc_mcontext.Eip = (size_t) func;
    ucp->uc_mcontext.Esp = (size_t) sp;

    /* Save/Restore the full machine context */
    ucp->uc_mcontext.ContextFlags = CONTEXT_FULL;

    /* Copy the return func that will handle "uc_link" (added below) */
    assert( argc < (sizeof( return_func ) / sizeof( return_func[0] )) &&
            "increase return_func[], not enough functions available - "
            "or... reduce number of arguments to makecontext" );
    *(size_t*)sp = (size_t)return_func[argc];
    sp += sizeof(size_t);

    /* Copy the arguments */
    va_start (ap, argc);
    for (i=0; i<argc; i++)
    {
        memcpy(sp, ap, 8);
        ap +=8;
        sp += 8;
    }
    va_end(ap);

    /* Copy the uc_link pointer value for someplace to go when func() returns */
    *(size_t*)sp = (size_t)ucp->uc_link;

    /*
    for (int x = 0; x < ucp->uc_stack.ss_size/4; ++x)
    {
        printf( "%02x", (*((sp_orig-1)-x) & 0xff) );
        if ((x % 4) == 3) printf( " " );
        if ((x % 40) == 39)
        {
            printf( "\n" );
        }
    }printf( "\n" );printf( "\n" );printf( "\n" );
    */
    return 0;
}

__inline int swapcontext(ucontext_t *oucp, const ucontext_t *ucp)
{
    int ret;
    if ((oucp == NULL) || (ucp == NULL))
    {
        /*errno = EINVAL;*/
        return -1;
    }

    ret = getcontext(oucp);
    if (ret == 0)
    {
        ret = setcontext(ucp);
    }
    return ret;
}

#ifdef __cplusplus
} // extern "C"
#endif 

#endif

