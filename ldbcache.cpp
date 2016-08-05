#include <string.h>
#include <stdlib.h>
#include <assert.h>

extern "C" {  
#include "lua.h"
#include "lauxlib.h"
#include "cleanupsem.h"
}

#include "dbfunc.h"

#define MB 1024*1024

// 1.memdbname, 2.mysqlhost, 3.mysqluser, 4.mysqlpassword, 5.mysqldbname, 6.mysqldbport, 7.initsize(optional), 8.incsize(optional), 9.idxsize(optional), 10.maxsize(optional)
static int
lopendb(lua_State *L) {
	const char * memdbname = lua_tostring(L, 1);
	luaL_argcheck(L, memdbname != NULL, 1, "memdbname can't be null");

	const char *mysqlhost = lua_tostring(L, 2);
	luaL_argcheck(L, mysqlhost != NULL, 2, "mysqlhost can't be null");

	const char *mysqluser = lua_tostring(L, 3);
	luaL_argcheck(L, mysqluser != NULL, 3, "mysqluser can't be null");

	const char *mysqlpassword = lua_tostring(L, 4);
	luaL_argcheck(L, mysqlpassword != NULL, 4, "mysqlpassword can't be null");

	const char *mysqldbname = lua_tostring(L, 5);
	luaL_argcheck(L, mysqldbname != NULL, 5, "mysqldbname can't be null");

	size_t mysqldbport = lua_tointeger(L, 6);
	luaL_argcheck(L, mysqldbport != 0, 6, "mysqldbport can't be null");

	size_t initsize = lua_tointeger(L, 7);
	if (!initsize) {
		initsize = 100;
	}
	size_t incsize = lua_tointeger(L, 8);
	if (!incsize) {
		incsize = 20;
	}
	size_t idxsize = lua_tointeger(L, 9);
	if (!idxsize) {
		idxsize = 100;
	}
	size_t maxsize = lua_tointeger(L, 10);
	if (!maxsize) {
		maxsize = 200;
	}

	// mysql 
	g_mysqlconn = mysql_init(NULL);
	if ( !g_mysqlconn ) {
		return luaL_error(L, "Failed to call mysql_init");
	}

	// Set auto reconnect option
	my_bool reconnect = 1;
	mysql_options(g_mysqlconn, MYSQL_OPT_RECONNECT, &reconnect);

	char * unix_socket = NULL;
	unsigned long client_flag = 0;
	
	if ( mysql_real_connect(g_mysqlconn, mysqlhost, mysqluser, mysqlpassword, mysqldbname, mysqldbport, unix_socket, client_flag) == NULL ) {
		return luaL_error(L, "Failed to connect mysql(%s:%d) error(%s)", mysqlhost, mysqldbport, mysql_error(g_mysqlconn));
	}

	// Set autocommit mode off
	my_bool autocommit = 0;
	if ( mysql_autocommit(g_mysqlconn, autocommit) ) {
		return luaL_error(L, "mysql_autocommit failed error(%s)", mysql_error(g_mysqlconn));
	}

	// Set the default character set for the current connection
	if ( mysql_set_character_set(g_mysqlconn, "utf8mb4") != 0 ) {
		return luaL_error(L, "mysql_set_character_set failed error(%s)", mysql_error(g_mysqlconn));
	}

	cleanupsem( memdbname );
	g_session = cli_create(memdbname, NULL, 0, cli_open_default, initsize*MB, incsize*MB, idxsize*MB, maxsize*MB);
	if ( g_session < 0 )
		return luaL_error(L, "Failed to create memdb(%s)", get_cli_error_msg(g_session));

	lua_pushinteger(L, g_session);
	return 1;
}

static int
lclosedb(lua_State *L) {
	const char * memdbname = lua_tostring(L, 1);
	luaL_argcheck(L, memdbname != NULL, 1, "memdbname can't be null");
	int session = lua_tointeger(L, 2);
	cli_close(session);
	cleanupsem( memdbname );

	if ( g_mysqlconn != NULL ) {
		mysql_close(g_mysqlconn);
		g_mysqlconn = NULL;
	}
	return 0;
}

static int
lcleanupsem(lua_State *L) {
	const char * memdbname = lua_tostring(L, 1);
	luaL_argcheck(L, memdbname != NULL, 1, "memdbname can't be null");
	cleanupsem( memdbname );
	return 0;
}

static int
lbegin(lua_State *L) {
	int session = lua_tointeger(L, 1);
	int rc = cli_attach( session );
	if( rc != cli_ok )
		return luaL_error(L, "Failed to begin(%s)", get_cli_error_msg(rc));
	return 0;
}

static int
lcommit(lua_State *L) {
	int session = lua_tointeger(L, 1);
	int rc = cli_commit(session);
	if( rc != cli_ok )
		return luaL_error(L, "Failed to commit(%s)", get_cli_error_msg(rc));
	rc = cli_detach(session, cli_commit_on_detach);
	if( rc != cli_ok )
		return luaL_error(L, "Failed to detach(%s)", get_cli_error_msg(rc));
	return 0;
}

static int
lrollback(lua_State *L) {
	int session = lua_tointeger(L, 1);
	int rc = cli_abort(session);
	if( rc != cli_ok )
		return luaL_error(L, "Failed to rollback(%s)", get_cli_error_msg(rc));
	rc = cli_detach(session, cli_commit_on_detach);
	if( rc != cli_ok )
		return luaL_error(L, "Failed to detach(%s)", get_cli_error_msg(rc));
	return 0;
}

static int
lgettable(lua_State *L) {
	const char *tablename = lua_tostring(L, 1);
	lua_pushstring(L, tablename);
	lua_gettable(L, lua_upvalueindex(FIELD_TABLE_OBJECT));
	return 1;
}

static int
lloadtable(lua_State *L) {
	int rc = cli_create_table(g_session, "tb_activity", sizeof(tb_activity_descriptor)/sizeof(cli_field_descriptor), tb_activity_descriptor);
	if ( rc != cli_ok ) {		
		return luaL_error(L, "Failed to create table(%s)", get_cli_error_msg(rc));
	}

	load_tables(L);
	return 0;
}

static int
ltickcount(lua_State *L) {
	if ( !g_mysqlconn ) {
		return luaL_error(L, "Not connect mysql yet!");
	}

	if ( queue_empty(&g_sqlqueue) ) {
		mysql_ping(g_mysqlconn);
	}

	if ( mysql_query(g_mysqlconn, "begin;") ) {
		return luaL_error(L, "failed to mysql_query(%s)", mysql_error(g_mysqlconn));
	}

	while( !queue_empty(&g_sqlqueue) ) {
		sqldao dao;
		queue_pop(&g_sqlqueue, &dao);
		
		if ( mysql_query(g_mysqlconn, dao.sql) ) {
			return luaL_error(L, "failed to mysql_query(%s)", mysql_error(g_mysqlconn));
		}
	}

	if ( mysql_commit(g_mysqlconn) != 0 ) {
		return luaL_error(L, "failed to mysql_commit(%s)", mysql_error(g_mysqlconn));
	}

	return 0;
}

static int
lfreestatement(lua_State *L) {
	lua_pushnil(L);
	while( lua_next(L, lua_upvalueindex(FIELD_TABLE_FREESTATEMENT)) ) {
		int statement = lua_tointeger(L, -1);
		int rc = cli_free(statement);
		if ( rc != cli_ok ) {
			return luaL_error(L, "Failed to free statement(%s)", get_cli_error_msg(rc));
		}
		lua_pop(L, 1);
	}
	return 0;
}

extern "C" int
luaopen_dbcache_core(lua_State *L) {
	queue_init(&g_sqlqueue, sizeof(struct sqldao));
	queue_init(&g_freestatement, sizeof(int));

	luaL_checkversion(L);

	luaL_Reg l[] = {
		{ "opendb", lopendb },
		{ "loadtable", lloadtable },
		{ "cleanupsem", lcleanupsem },
		{ "begin", lbegin },
		{ "commit", lcommit },
		{ "rollback", lrollback },
		{ "closedb", lclosedb },
		{ "gettable", lgettable },
		{ "tickcount", ltickcount },
		{ "freestatement", lfreestatement },
		{ NULL, NULL },
	};

	luaL_newlib(L, l);

	// the order is the same with macros: FIELD_* (define top)
	lua_newtable(L); // upvalue, all table object
	create_table_objects(L);

	// sharing previous tables as upvalue
	luaL_setfuncs(L, l, 2);

	return 1;
}
