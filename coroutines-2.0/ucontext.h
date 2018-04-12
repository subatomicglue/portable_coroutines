
struct ucontext_t
{
   ucontext_t *uc_link     //pointer to the context that will be resumed
                           //when this context returns
   sigset_t    uc_sigmask  //the set of signals that are blocked when this
                           //context is active
   stack_t     uc_stack    //the stack used by this context
   mcontext_t  uc_mcontext //a machine-specific representation of the saved
                           //context
};

int  getcontext(ucontext_t *);
int  setcontext(const ucontext_t *);
void makecontext(ucontext_t *, (void *)(), int, ...)
{
   corohandle c = coroutine( 
}

int  swapcontext(ucontext_t *, const ucontext_t *);
