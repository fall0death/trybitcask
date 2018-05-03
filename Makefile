all:biu

example:src/bitcask.cc src/bitcask.h hello.cc
	g++ -std=c++11 src/bitcask.cc hello.cc -o example

biu:src/bitcask.cc src/bitcask.h
	g++ -std=c++11 -fPIC -shared -o biu.so src/bitcask.cc

clean:
	rm -rf *.o biu
