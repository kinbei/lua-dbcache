all : \
	./mysql \
	fastdb

.PHONY : fastdb

fastdb : 
	cd fastdb && ./configure --enable-diskless --prefix=$(CURDIR)/fastdb/output/ && make && make install

./mysql : ./mysql.c
	gcc -Wall -g -DDEBUG -fPIC -o $@ $^ -I/opt/local/mysql/include/ -L/opt/local/mysql/lib/ -lmysqlclient

clean :
	rm ./mysql
	cd fastdb && make clean && rm -rf output
