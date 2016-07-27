#include <string.h>
#include <stdlib.h>
#include <assert.h>

extern "C" {  
#include "lua.h"
#include "lauxlib.h"
}

#include "msvcint.h"
#include "fastdb/fastdb.h"
#include "fastdb/cli.h"
#include "cleanupsem.h"

#define MB 1024*1024

// memdbname, initsize, incsize, idxsize, maxsize
static int
linit(lua_State *L) {
	const char * memdbname = lua_tostring(L, 1);
	luaL_argcheck(L, memdbname != NULL, 1, "memdbname can't be null");
	size_t initsize = lua_tointeger(L, 2);
	if (initsize) {
		initsize = 1024;
	}
	size_t incsize = lua_tointeger(L, 3);
	if (incsize) {
		incsize = 20;
	}
	size_t idxsize = lua_tointeger(L, 4);
	if (idxsize) {
		idxsize = 1024;
	}
	size_t maxsize = lua_tointeger(L, 5);
		if (maxsize) {
		maxsize = 2048;
	}

	cleanupsem( memdbname );
	int session = cli_create(memdbname, NULL, 0, cli_open_default, initsize*MB, incsize*MB, idxsize*MB, maxsize*MB);
	if ( session < 0 )
		return luaL_error(L, "Failed to create memdb(%d)", session);

	lua_pushinteger(L, session);
	return 1;
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
	int r = cli_attach( session );
	if( r != cli_ok )
		return luaL_error(L, "Failed to begin(%d)", r);
	return 0;
}

static int
lcommit(lua_State *L) {
	int session = lua_tointeger(L, 1);
	int r = cli_commit(session);
	if( r != cli_ok )
		return luaL_error(L, "Failed to commit(%d)", r);
	r = cli_detach(session, cli_commit_on_detach);
	if( r != cli_ok )
		return luaL_error(L, "Failed to detach(%d)", r);
	return 0;
}

static int
lrollback(lua_State *L) {
	int session = lua_tointeger(L, 1);
	int r = cli_abort(session);
	if( r != cli_ok )
		return luaL_error(L, "Failed to rollback(%d)", r);
	r = cli_detach(session, cli_commit_on_detach);
	if( r != cli_ok )
		return luaL_error(L, "Failed to detach(%d)", r);
	return 0;
}

static int
lclose(lua_State *L) {
	const char * memdbname = lua_tostring(L, 1);
	luaL_argcheck(L, memdbname != NULL, 1, "memdbname can't be null");
	int session = lua_tointeger(L, 2);
	cli_close(session);
	cleanupsem( memdbname );
	return 0;
}

static int
lgettable(lua_State *L) {
	const char *tablename = lua_tostring(L, 1);

	lua_pushstring(L, tablename);
	lua_gettable(L, lua_upvalueindex(1));
	lua_rawget(L, -1);

	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/* table record define */
struct tb_activity_record {
	cli_int8_t activity_id;
	char *activity_name;
	cli_int8_t status;
};

/* table field define */
struct cli_field_descriptor tb_activity_descriptor[] = {
		{ cli_int8, cli_indexed, "activity_id", },
		{ cli_asciiz, 0, "activity_name" },
		{ cli_int8, cli_indexed, "status", },		
};

/*
static int
create_table(lua_State *L, int session) {
	int rc = cli_create_table(session, "tb_activity", sizeof(tb_activity_descriptor)/sizeof(cli_field_descriptor), tb_activity_descriptor);
	if ( rc != cli_ok ) {		
		return luaL_error(L, "Failed to create table with code(%d)", rc);
	}
	return 0;
}
*/

/* save temp value of per record */
tb_activity_record _tb_activity_record;

/////////////////////////////////////////////////////////////////////////
struct dbrecordset {
	tb_activity_record _tb_activity_record;
};
static struct dbrecordset g_dbrecord;

struct dbstatement {
	int _tb_activity_statement;
};
static struct dbstatement g_statement;

static int 
tb_activity_reset(lua_State *L) {
	memset(&g_dbrecord._tb_activity_record, 0x00, sizeof(g_dbrecord._tb_activity_record));
	return 0;
}

static int
tb_activity_setactivity_id(lua_State *L) {
	lua_Integer v = lua_tointeger(L, 1);
	g_dbrecord._tb_activity_record.activity_id = v;
	return 0;
}

static int
tb_activity_setactivity_name(lua_State *L) {
	size_t len = 0;
	const char *v = lua_tolstring(L, 1, &len);
	if (len < sizeof(g_dbrecord._tb_activity_record.activity_name)-1 ) {
		len = sizeof(g_dbrecord._tb_activity_record.activity_name)-1;
	}
	memcpy(g_dbrecord._tb_activity_record.activity_name, v, len);
	return 0;
}

static int
tb_activity_setstatus(lua_State *L) {
	lua_Integer v = lua_tointeger(L, 1);
	g_dbrecord._tb_activity_record.status = v;
	return 0;
}

static int
tb_activity_insert(lua_State *L) {
	int session = lua_tointeger(L, 1);
	int statement = cli_statement(session, "insert info tb_activity");
	if ( statement < 0 ) {
		return luaL_error(L, "Failed to create statement with code(%d)", statement);
	}

	int rc;
	if( ( rc = cli_column(statement, "activity_id", cli_int8, NULL, &g_dbrecord._tb_activity_record.activity_id ) != cli_ok ) ||
		( rc = cli_column(statement, "activity_name", cli_asciiz, NULL, g_dbrecord._tb_activity_record.activity_name ) != cli_ok ) ||
		( rc = cli_column(statement, "status", cli_int8, NULL, &g_dbrecord._tb_activity_record.status ) != cli_ok )
		) {
		return luaL_error(L, "Failed to call cli_column with code(%d)", rc);
	}

	cli_oid_t oid;
	rc = cli_insert(statement, &oid);
	if ( rc != cli_ok ) {
		return luaL_error(L, "Failed to insert with code(%d)", rc);
	}

	rc = cli_free(statement);
	if ( rc != cli_ok ) {
		return luaL_error(L, "Failed to free statement with code(%d)", rc);
	}

	return 0;
}

static int
tb_activity_prepare(lua_State *L) {
	const char *subsql = lua_tostring(L, 2);
	int session = lua_tointeger(L, 1);

	char sql[255] = {0};
	snprintf(sql, sizeof(sql), "select * from tb_activity where %s", subsql);
	g_statement._tb_activity_statement = cli_statement(session, sql);
	if ( g_statement._tb_activity_statement < 0 ) {
		return luaL_error(L, "Failed to prepare query with code(%d)", g_statement._tb_activity_statement);
	}

	return 0;
}

/*
static int
tb_activity_setactivity_id(lua_State *L) {
	int rc = cli_column(g_statement._tb_activity_statement, "activity_id", cli_int8, NULL, &g_dbrecord._tb_activity_record.activity_id);
	if ( rc != cli_ok ) {
		return luaL_error(L, "Failed to cli_column with code(%d)", rc);
	}
	return 0;
}
*/

/*
static int
tb_activity_setactivity_name(lua_State *L) {
	int rc = cli_column(g_statement._tb_activity_statement, "activity_name", cli_asciiz, NULL, g_dbrecord._tb_activity_record.activity_name);
	if ( rc != cli_ok ) {
		return luaL_error(L, "Failed to cli_column with code(%d)", rc);
	}
	return 0;
}
*/

/*
static int
tb_activity_setstatus(lua_State *L) {
	int rc = cli_column(g_statement._tb_activity_statement, "status", cli_int8, NULL, g_dbrecord._tb_activity_record.status);
	if ( rc != cli_ok ) {
		return luaL_error(L, "Failed to cli_column with code(%d)", rc);
	}
	return 0;
}*/

#define DECLARE_FUNCTION(tablename, funcname) \
do \
{ \
	lua_pushcfunction(L, tablename##_##funcname ); \
	lua_pushliteral(L, #funcname); \
	lua_rawset(L, -2); \
} \
while (0); \

extern "C" int
luaopen_dbcache_core(lua_State *L) {
	luaL_checkversion(L);

	luaL_Reg l[] = {
		{ "init", linit },
		{ "cleanupsem", lcleanupsem },
		{ "begin", lbegin },
		{ "commit", lcommit },
		{ "rollback", lrollback },
		{ "close", lclose },
		{ "gettable", lgettable },
		{ NULL, NULL },
	};

	luaL_newlib(L, l);

	lua_newtable(L); // upvalue

	// tb_activity
	lua_newtable(L);
	DECLARE_FUNCTION(tb_activity, reset);
	DECLARE_FUNCTION(tb_activity, setactivity_id);
	DECLARE_FUNCTION(tb_activity, setactivity_name);
	DECLARE_FUNCTION(tb_activity, setstatus);
	DECLARE_FUNCTION(tb_activity, insert);
	DECLARE_FUNCTION(tb_activity, prepare);
	lua_pushliteral(L, "tb_activity");
	lua_rawset(L, -2);

	// sharing previous table as upvalue
	luaL_setfuncs(L, l, 1);

	return 1;
}
