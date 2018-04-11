all:
	rm -f core.*
	gcc -ggdb -m32 -c libtask/asm.s -o asm32.o
	gcc -ggdb -m32 -c libtask/context.c -o context32.o
	gcc -ggdb -m64 -c libtask/asm.s -o asm64.o
	gcc -ggdb -m64 -c libtask/context.c -o context64.o
	g++ -DTEST1 -ggdb -Ilibtask main.cpp asm64.o context64.o -o a
	g++ -DTEST2 -ggdb -Ilibtask main.cpp asm64.o context64.o -o a2
	g++ -ggdb -Ilibtask main-jmp.cpp asm64.o context64.o -o jmp
	g++ -ggdb -m32 -Ilibtask ucontext_test.cpp asm32.o context32.o -o ucontext_test32
	g++ -ggdb -m64 -Ilibtask ucontext_test.cpp asm64.o context64.o -o ucontext_test64

tarball:
	make clean
	rm -f coroutines.tgz
	cd ../ && tar --exclude=\.svn --exclude=Debug --exclude=*\.ncb --exclude=*\.suo --exclude=*\.user -zcvf coroutines.tgz coroutines/ && cd -
	mv ../coroutines.tgz .

clean:
	rm -fr a a2 jmp ucontext_test* *.dSYM *.o
