all : \
	fastdb \
	./dbcache.so

.PHONY : fastdb

fastdb : 
	cd fastdb && ./configure --enable-diskless --enable-debug --prefix=$(CURDIR)/fastdb/output/ && make && make install

dbcache.so : ldbcache.cpp cleanupsem.c dbfunc.cpp
	g++ -Wall -g -DDEBUG --shared -fPIC -o $@ $^ -I./lua -I./fastdb/output/include -L./fastdb/output/lib/ -lfastdb -lcli

clean :
	rm -rf ./dbcache.so
	cd fastdb && make clean && rm -rf output
