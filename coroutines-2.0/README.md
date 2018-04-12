
# version 2.0 of portable coroutines in c/c++

This is v2.0.  Read [here](..) for background / explanation.

As with v1, this is an exploration.
You might find it interesting.

There is always `Boost::Context`, which seems to do a production quality job of this.

Features Explored:
- Windows Fibers (32/64bit limited to ~2000 fibers)
- POSIX ucontext (system, and deprecated in 2004)
- portable ucontext (provided, no macosx/win-x64)
- win32 ucontext (32bit only)
- [yosefk](readme-yosefk.txt) (no win-x64)
- [picoro](readme-picoro.txt) (ansi C no ASM, limited by stack)


## summary of coroutine methods explored

* `picoro` - very portable, ANSI C w/ no ASM, limited by stack
* `ucontext` - very portable in linux, macosx kinda broken, POSIX deprecated it!
* `ucontext-portable` - great, but no return functionality implemented
* `ucontext-win32` - win32 only, limited
* `yosefk` - great, unlimited, uses ASM, no WIN64 version yet (i'd love to solve this!)
* `fibers` - Uses Native windows Fibers: win32/64 only, limited by stack

## future work

* implement `yosefk` for `WIN64`...


