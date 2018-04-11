# ucontext (for WIN32)

this is a copy of the WIN32 version of ucontext by xdoukas from codeproject.
it has been extended to support proper uc_link return functionality,
as well as better inlining of functions.

## future work

- needs a 64bit version.  (currently relies on EAX, etc...)
  64bit version needs ASM for windows compiler, rather than inline ASM...


