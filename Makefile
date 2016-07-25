ROOT=$(cd `dirname $0`; pwd)

all : \
	./mysql \
	fastdb

.PHONY : fastdb

fastdb : 
	cd fastdb && ./configure --enable-diskless --prefix=${ROOT}/fastdb/libs

./mysql : ./mysql.c
	gcc -Wall -g -DDEBUG -fPIC -o $@ $^ -I/opt/local/mysql/include/ -L/opt/local/mysql/lib/ -lmysqlclient

clean :
	rm ./mysql
