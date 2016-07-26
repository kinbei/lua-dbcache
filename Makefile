all : \
	fastdb \
	./dbcache.so

.PHONY : fastdb

fastdb : 
	cd fastdb && ./configure --enable-diskless --enable-debug --prefix=$(CURDIR)/fastdb/output/ && make && make install

# ./mysql : ./mysql.c
#	gcc -Wall -g -DDEBUG -fPIC -o $@ $^ -I/opt/local/mysql/include/ -L/opt/local/mysql/lib/ -lmysqlclient

./dbcache.so : ./ldbcache.cpp ./cleanupsem.c
	g++ -Wall -g -DDEBUG --shared -fPIC -o $@ $^ -I./lua -I./fastdb/output/include -L./fastdb/output/lib/ -lfastdb -lcli

clean :
	rm -rf ./dbcache.so
	cd fastdb && make clean && rm -rf output
