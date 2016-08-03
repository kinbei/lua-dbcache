all : \
	fastdb \
	mysqlconnector \
	./dbcache.so

.PHONY : fastdb

mysqlconnector : 
	cd mysql-connector-c && cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$(CURDIR)/mysql-connector-c/output/ && make && make install

fastdb : 
	cd fastdb && ./configure --enable-diskless --enable-debug --prefix=$(CURDIR)/fastdb/output/ && make && make install

dbcache.so : ldbcache.cpp cleanupsem.c dbfunc.cpp queue.c dbfunc.h
	g++ -Wall -g -DDEBUG --shared -fPIC -o $@ $^ -I./lua -I./fastdb/output/include -L./fastdb/output/lib/ \
	-I./mysql-connector-c/output/include -L./mysql-connector-c/output/lib/ -lfastdb -lcli -lmysqlclient

clean :
	rm -rf ./dbcache.so
	cd fastdb && make clean && rm -rf output
	cd mysql-connector-c && make clean && rm -rf output
