#include <string.h>
#include <stdlib.h>
#include <assert.h>

extern "C" {  
#include "lua.h"
#include "lauxlib.h"
}

#include "cleanupsem.h"
#include "dbfunc.h"

#define MB 1024*1024

#define FIELD_TABLE_OBJECT 1

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
	g_session = cli_create(memdbname, NULL, 0, cli_open_default, initsize*MB, incsize*MB, idxsize*MB, maxsize*MB);
	if ( g_session < 0 )
		return luaL_error(L, "Failed to create memdb(%s)", get_cli_error_msg(g_session));

	lua_pushinteger(L, g_session);
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
	lua_gettable(L, lua_upvalueindex(FIELD_TABLE_OBJECT));
	return 1;
}

static int
lloadtable(lua_State *L) {
	int rc = cli_create_table(g_session, "tb_activity", sizeof(tb_activity_descriptor)/sizeof(cli_field_descriptor), tb_activity_descriptor);
	if ( rc != cli_ok ) {		
		return luaL_error(L, "Failed to create table(%s)", get_cli_error_msg(rc));
	}
	return 0;
}

extern "C" int
luaopen_dbcache_core(lua_State *L) {
	luaL_checkversion(L);

	luaL_Reg l[] = {
		{ "opendb", linit },
		{ "loadtable", lloadtable },
		{ "cleanupsem", lcleanupsem },
		{ "begin", lbegin },
		{ "commit", lcommit },
		{ "rollback", lrollback },
		{ "closedb", lclose },
		{ "gettable", lgettable },
		{ NULL, NULL },
	};

	luaL_newlib(L, l);

	// the order is the same with macros: FIELD_* (define top)
	lua_newtable(L); // upvalue, all table object
	create_table_object(L);

	// sharing previous table as upvalue
	luaL_setfuncs(L, l, 1);

	return 1;
}
