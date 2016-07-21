#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <mysql.h>

int main(void) {
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	const char * host = "192.168.8.196";
	const char * user = "root";
	const char * password = "hQK_DWbBuzl";
	const char * dbname = "1.2.2";
	unsigned int port = 4400;
	char * unix_socket = NULL;
	unsigned long client_flag = 0;


	conn = mysql_init(NULL);
	if ( !conn ) {
		fprintf(stderr, "init mysql failed \n");
		return EXIT_FAILURE;
	}

	conn = mysql_real_connect(conn, host, user, password, dbname, port, unix_socket, client_flag);
	if ( conn ) {
		printf("connect success ... \n");
	}
	else {
		fprintf(stderr, "connect failed .. %d:%s \n", errno, strerror(errno));
		return EXIT_FAILURE;
	}

	if ( mysql_query(conn, "select * from tb_gift") ) {
		fprintf(stderr, "failed to mysql_query .. %d:%s \n", errno, strerror(errno));
		return EXIT_FAILURE;
	}

	res = mysql_use_result(conn);
	while( (row = mysql_fetch_row(res)) ) {
		fprintf(stdout, "%s \t %s \t \n", row[0], row[1]);
	}

	mysql_free_result(res);
	mysql_close(conn);
	return EXIT_SUCCESS;
}
