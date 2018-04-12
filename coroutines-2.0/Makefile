CFLAGS = -pipe -O -g -std=c89 -pedantic -Wall -Wextra \
	-Wbad-function-cast -Wcast-align -Wcast-qual -Wconversion -Winline \
	-Wmissing-declarations -Wmissing-prototypes -Wnested-externs \
	-Wold-style-definition -Wpointer-arith -Wredundant-decls -Wshadow \
	-Wstrict-prototypes -Wunreachable-code -Wwrite-strings

default: ucontext-portable/_build testPicoro testYosefk testUcontextPortable testUcontextSystem \
	testMicrofiberPicoro testMicrofiberYosefk testMicrofiberUcontextSystem testMicrofiberUcontextPortable \
	testKoroutinesPicoro testKoroutinesYosefk testKoroutinesUcontextSystem testKoroutinesUcontextPortable


ucontext-portable/_build:
	cd ucontext-portable && make

testKoroutinesPicoro: testKoroutines.cpp KoroutinesPicoro.cpp Koroutines.h picoro.c picoro.h
	@echo "---=== $@ ===---"
	gcc ${CFLAGS} -c -o picoro.o picoro.c
	g++ -c -o testKoroutines.o testKoroutines.cpp
	g++ -c -o KoroutinesPicoro.o KoroutinesPicoro.cpp
	g++ -o testKoroutinesPicoro testKoroutines.o KoroutinesPicoro.o picoro.o

testKoroutinesUcontextSystem: testKoroutines.cpp KoroutinesUcontext.cpp Koroutines.h
	@echo "---=== $@ ===---"
	g++ -c -o testKoroutines.o testKoroutines.cpp
	g++ -D_XOPEN_SOURCE -c -o KoroutinesUcontext.o KoroutinesUcontext.cpp
	g++ -o testKoroutinesUcontextSystem testKoroutines.o KoroutinesUcontext.o

testKoroutinesUcontextPortable: testKoroutines.cpp KoroutinesUcontext.cpp Koroutines.h ./ucontext-portable/source/asm.S ./ucontext-portable/source/context.c
	@echo "---=== $@ ===---"
	cc -O3 -ggdb   -Wstrict-prototypes  -I. -I./ucontext-portable/source -fPIC -DBUILDING_CORO_DLL  -c ./ucontext-portable/source/context.c -o ./ucontext-portable/_build/objs/context.o
	cc -O3 -ggdb   -Wstrict-prototypes  -I. -I./source   -fPIC -DBUILDING_CORO_DLL  -c ./ucontext-portable/source/asm.S -o ./ucontext-portable/_build/objs/asm.o
	g++ -ggdb -c -o testKoroutines.o testKoroutines.cpp
	g++ -ggdb -DUSE_PORTABLE_UCONTEXT -c -o KoroutinesUcontext.o KoroutinesUcontext.cpp
	g++ -o testKoroutinesUcontextPortable testKoroutines.o KoroutinesUcontext.o ./ucontext-portable/_build/objs/asm.o ./ucontext-portable/_build/objs/context.o

testKoroutinesYosefk: testKoroutines.cpp KoroutinesYosefk.cpp Koroutines.h yosefk.cpp yosefk.h
	@echo "---=== $@ ===---"
	g++ -c -o yosefk.o yosefk.cpp
	g++ -c -o testKoroutines.o testKoroutines.cpp
	g++ -c -o KoroutinesYosefk.o KoroutinesYosefk.cpp
	g++ -o testKoroutinesYosefk testKoroutines.o KoroutinesYosefk.o yosefk.o

testYosefk: testYosefk.cpp yosefk.cpp yosefk.h
	@echo "---=== $@ ===---"
	g++ -c -o yosefk.o yosefk.cpp
	g++ -c -o testYosefk.o testYosefk.cpp
	g++ -o testYosefk testYosefk.o yosefk.o

testPicoro: testPicoro.cpp picoro.c picoro.h
	@echo "---=== $@ ===---"
	gcc ${CFLAGS} -c -o picoro.o picoro.c
	g++ -c -o testPicoro.o testPicoro.cpp
	g++ -o testPicoro testPicoro.o picoro.o

testUcontextSystem: testUcontext.cpp
	@echo "---=== ucontext SYSTEM! ===---"
	g++ -D_XOPEN_SOURCE -c -o testUcontext.o testUcontext.cpp
	g++ -o testUcontextSystem testUcontext.o

testUcontextPortable: testUcontext.cpp  ./ucontext-portable/source/asm.S ./ucontext-portable/source/context.c
	@echo "---=== ucontext PORTABLE! ===---"
	cc -O3 -g  -Wstrict-prototypes  -I. -I./ucontext-portable/source -fPIC -DBUILDING_CORO_DLL  -c ./ucontext-portable/source/context.c -o ./ucontext-portable/_build/objs/context.o
	cc -O3 -g  -Wstrict-prototypes  -I. -I./source   -fPIC -DBUILDING_CORO_DLL  -c ./ucontext-portable/source/asm.S -o ./ucontext-portable/_build/objs/asm.o
	g++ -DUSE_PORTABLE_UCONTEXT -c -o testUcontext.o -Iucontext-portable/_build/headers testUcontext.cpp
	g++ -o testUcontextPortable testUcontext.o ./ucontext-portable/_build/objs/asm.o ./ucontext-portable/_build/objs/context.o

testMicrofiberPicoro: testMicrofiber.cpp Microfiber.h Makefile
	@echo "---=== $@ ===---"
	gcc ${CFLAGS} -c -o picoro.o picoro.c
	g++ -c -o KoroutinesPicoro.o KoroutinesPicoro.cpp
	g++ -I. -c -o testMicrofiber.o testMicrofiber.cpp
	g++ -o testMicrofiberPicoro testMicrofiber.o picoro.o KoroutinesPicoro.o

testMicrofiberYosefk: testMicrofiber.cpp Microfiber.h Makefile
	@echo "---=== $@ ===---"
	g++ -c -o yosefk.o yosefk.cpp
	g++ -c -o KoroutinesYosefk.o KoroutinesYosefk.cpp
	g++ -I. -c -o testMicrofiber.o testMicrofiber.cpp
	g++ -o testMicrofiberYosefk testMicrofiber.o yosefk.o KoroutinesYosefk.o

testMicrofiberUcontextSystem: testMicrofiber.cpp Microfiber.h Makefile
	@echo "---=== $@ ===---"
	g++ -D_XOPEN_SOURCE -c -o KoroutinesUcontext.o KoroutinesUcontext.cpp
	g++ -I. -c -o testMicrofiber.o testMicrofiber.cpp
	g++ -o testMicrofiberUcontextSystem testMicrofiber.o KoroutinesUcontext.o

testMicrofiberUcontextPortable: testMicrofiber.cpp Microfiber.h Makefile ./ucontext-portable/source/asm.S ./ucontext-portable/source/context.c
	@echo "---=== $@ ===---"
	cc -O3 -g  -Wstrict-prototypes  -I. -I./ucontext-portable/source -fPIC -DBUILDING_CORO_DLL  -c ./ucontext-portable/source/context.c -o ./ucontext-portable/_build/objs/context.o
	cc -O3 -g  -Wstrict-prototypes  -I. -I./source   -fPIC -DBUILDING_CORO_DLL  -c ./ucontext-portable/source/asm.S -o ./ucontext-portable/_build/objs/asm.o
	g++ -DUSE_PORTABLE_UCONTEXT -c -o KoroutinesUcontext.o KoroutinesUcontext.cpp
	g++ -I. -c -o testMicrofiber.o testMicrofiber.cpp
	g++ -o testMicrofiberUcontextPortable testMicrofiber.o KoroutinesUcontext.o ./ucontext-portable/_build/objs/asm.o ./ucontext-portable/_build/objs/context.o


count:
	cat picoro.[hc] | egrep -vc '^(#|/\*| \*|	*}*$$)'

clean:
	cd ucontext-portable && make clean
	rm -rf *.o */*.o */*/*.o */*/*/*.o \
		./testMicrofiberYosefk ./testMicrofiberPicoro ./testMicrofiberUcontextPortable ./testMicrofiberUcontextSystem \
		./testKoroutinesYosefk ./testKoroutinesPicoro ./testKoroutinesUcontextPortable ./testKoroutinesUcontextSystem \
		./testYosefk ./testPicoro ./testUcontextPortable ./testUcontextSystem

tarball:
	rm -f coroutines.tgz
	tar -zcvf coroutines.tgz .

