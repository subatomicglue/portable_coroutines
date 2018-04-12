from:
http://dekorte.com/projects/opensource/libcoroutine/

NOTES:
- Coro setjmp implemetation included doesn't support x64 on macosx.
- portable ucontext also has problems when the coroutine returns.

x64 ABI
http://refspecs.linuxfoundation.org/elf/x86_64-abi-0.95.pdf

