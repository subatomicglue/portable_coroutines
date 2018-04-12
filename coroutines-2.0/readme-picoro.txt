
Tony Finch - Coroutines in less than 20 lines of standard C
Allocates coroutine stacks onto the stack - portable, # coros limited by stack
http://fanf.livejournal.com/105413.html
http://dotat.at/cgi/git/picoro.git

====================

Coroutines in less than 20 lines of standard C
fanf wrote on 22nd January 2010 at 19:42


Here's a bit of evil hackery to brighten your weekend.

It's quite simple to switch between coroutines using standard C: use setjmp() to save the state of the current coroutine, and longjmp() to jump to the target coroutine. It's also useful to be able to transfer a bit of data while transferring control. The function coto() (short for "coroutine goto") does this. First it saves its argument somewhere the target coroutine can find it, then it calls setjmp() which saves the current coroutine's state and returns 0, so it goes on to longjmp() to the target coroutine, where setjmp() returns 1 so coto() returns the saved argument.

    static void *coarg;

    void *coto(jmp_buf here, jmp_buf there, void *arg) {
	coarg = arg;
        if (setjmp(here)) return(coarg);
	longjmp(there, 1);
    }

The other thing you need to be able to do is create coroutines. This requires allocating space for the coroutine's stack, initializing a stack frame there, and jumping to it. This is usually considered impossible to do without special support from the operating system or the run-time system - for example, see this paper (postscript). However if we can assume the stack is laid out contiguously in memory and that variable length arrays are allocated on the stack, we can do it using pure standard C99. (If your compiler doesn't support variable length arrays, alloca() might be available as an alternative.)

Each coroutine's stack is a chunk of the memory area used for the normal process stack. To allocate a new chunk of stack, we add the chunk size to the current top of stack (which allows space for future function calls by the current topmost coroutine) to give us a target location for the new topmost coroutine's stack. We then need to move the stack pointer to this new location, which we can do by creating a variable length array of the appropriate length. We can use the address of a local variable to approximate the current stack pointer when calculating this length. Finally, a simple function call creates the coroutine's initial stack frame and jumps to it.

The function cogo() (short for "coroutine start") implements this. It also saves the current coroutine's state before starting the new one.

    #define STACKDIR - // set to + for upwards and - for downwards
    #define STACKSIZE (1<<12)

    static char *tos; // top of stack

    void *cogo(jmp_buf here, void (*fun)(void*), void *arg) {
        if (tos == NULL) tos = (char*)&arg;
        tos += STACKDIR STACKSIZE;
        char n[STACKDIR (tos - (char*)&arg)];
        coarg = n; // ensure optimizer keeps n
        if (setjmp(here)) return(coarg);
	fun(arg);
	abort();
    }

Here's a little test program to demonstrate these functions in action.

    #define MAXTHREAD 10000

    static jmp_buf thread[MAXTHREAD];
    static int count = 0;

    static void comain(void *arg) {
	int *p = arg, i = *p;
	for (;;) {
		printf("coroutine %d at %p arg %p\n", i, (void*)&i, arg);
		int n = arc4random() % count;
		printf("jumping to %d\n", n);
		arg = coto(thread[i], thread[n], (char*)arg + 1);
	}
    }

    int main(void) {
	while (++count < MAXTHREAD) {
		printf("spawning %d\n", count);
		cogo(thread[0], comain, &count);
	}
	return 0;
    }

