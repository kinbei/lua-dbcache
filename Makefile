all : \
	./dbcache.so

mysql-connector-c/Makefile :
	cd mysql-connector-c && cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$(CURDIR)/mysql-connector-c/output/ && make && make install

fastdb/Makefile :
	cd fastdb && ./configure --enable-diskless --enable-debug --prefix=$(CURDIR)/fastdb/output/ && make && make install
	
queue.o : queue.c cleanupsem.c
	gcc -Wall -g -DDEBUG -c --shared -fPIC $^

lua/Makefile :
	git submodule update --init

dbcache.so : ldbcache.cpp dbfunc.cpp queue.o cleanupsem.o | queue.o lua/Makefile fastdb/Makefile mysql-connector-c/Makefile
	g++ -Wall -g -DDEBUG --shared -fPIC -o $@ $^ -I./lua -I./fastdb/output/include -L./fastdb/output/lib/ \
	-I./mysql-connector-c/output/include -L./mysql-connector-c/output/lib/ -lfastdb -lcli -lmysqlclient

clean :
	rm -rf ./dbcache.so
	cd fastdb && make clean && rm -rf output
	cd mysql-connector-c && make clean && rm -rf output
