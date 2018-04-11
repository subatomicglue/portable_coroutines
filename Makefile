all:
	rm -f core.*
	gcc -D_XOPEN_SOURCE -ggdb -c libtask/asm.s -o asm.o
	gcc -D_XOPEN_SOURCE -ggdb -c libtask/context.c -o context.o
	g++ -D_XOPEN_SOURCE -DTEST1 -ggdb -Ilibtask main.cpp asm.o context.o -o a
	g++ -D_XOPEN_SOURCE -DTEST2 -ggdb -Ilibtask main.cpp asm.o context.o -o a2
	g++ -D_XOPEN_SOURCE -ggdb -Ilibtask main-jmp.cpp asm.o context.o -o jmp
	g++ -D_XOPEN_SOURCE -ggdb -Ilibtask ucontext_test.cpp asm.o context.o -o ucontext_test

tarball:
	make clean
	rm -f coroutines.tgz
	cd ../ && tar --exclude=\.svn --exclude=Debug --exclude=*\.ncb --exclude=*\.suo --exclude=*\.user -zcvf coroutines.tgz coroutines/ && cd -
	mv ../coroutines.tgz .

clean:
	rm -fr a a2 jmp ucontext_test *.dSYM *.o
