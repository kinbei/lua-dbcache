all : \
	./mysql

./mysql : ./mysql.c
	gcc -Wall -g -DDEBUG -fPIC -o $@ $^ -I/opt/local/mysql/include/ -L/opt/local/mysql/lib/ -lmysqlclient

clean :
	rm ./mysql
