#example:src/bitcask.h src/hello.cc src/bitcask.cc
#	g++ -std=c++11  src/bitcask.cc src/hello.cc -o example

example: hello.o bitcask.o
	g++ -std=c++11  hello.o bitcask.o -o example

bitcask.o:src/bitcask.cc src/bitcask.h
	g++ -std=c++11 -g -c src/bitcask.cc 

hello.o:src/hello.cc src/bitcask.h
	g++ -std=c++11 -g -c src/hello.cc

clean:
	rm -rf *.o biu
