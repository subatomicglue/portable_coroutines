all:
	rm -f core.*
	g++ -D_XOPEN_SOURCE -DTEST1 -ggdb main.cpp -o a
	g++ -D_XOPEN_SOURCE -DTEST2 -ggdb main.cpp -o a2
	g++ -D_XOPEN_SOURCE -ggdb main-jmp.cpp -o jmp
	g++ -D_XOPEN_SOURCE -ggdb ucontext_test.cpp -o ucontext_test

tarball:
	make clean
	rm -f coroutines.tgz
	cd ../ && tar --exclude=\.svn --exclude=Debug --exclude=*\.ncb --exclude=*\.suo --exclude=*\.user -zcvf coroutines.tgz coroutines/ && cd -
	mv ../coroutines.tgz .

clean:
	rm -f a a2 jmp ucontext_test
