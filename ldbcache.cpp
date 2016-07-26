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

/* table record define */
struct tb_activity_record {
	cli_int8_t activity_id;
	char *activity_name;
};

/* table field define */
struct cli_field_descriptor tb_activity_descriptor[] = {
		{ cli_int8, cli_indexed, "activity_id", },
		{ cli_asciiz, 0, "activity_name" },
};

static int
ltest(lua_State *L) {
	int session = lua_tointeger(L, 1);

	int r = cli_create_table(session, "tb_activity", sizeof(tb_activity_descriptor)/sizeof(cli_field_descriptor), tb_activity_descriptor);
	if ( r != cli_ok ) {		
		return luaL_error(L, "Failed to insert(%d)", r);
	}
	
	struct tb_activity_record t;
	memset(&t, 0x00, sizeof(t));
	t.activity_id = 0xFFFFFFFFFFFFFFFF;
	t.activity_name = "this is a activity name";
	printf("before insert 0x%08X \n", t.activity_name);
	// snprintf(t.activity_name, sizeof(t.activity_name), "%s", "this is activity name");

	r = cli_insert_struct(session, "tb_activity", &t, NULL);
	if ( r != cli_ok ) {
		return luaL_error(L, "Failed to insert(%d)", r);
	}

	int statement = cli_prepare_query(session, 
		"select * from tb_activity where activity_id = %li");
	if ( statement < 0 ) {
		return luaL_error(L, "Failed to prepare query with code(%d)", statement);
	}
	
	struct tb_activity_record rt;
	memset(&rt, 0x00, sizeof(rt));
	r = cli_execute_query(statement, cli_view_only, &rt, (cli_int8_t)0xFFFFFFFFFFFFFFFF);
	if (r < 0) {
		return luaL_error(L, "Failed to execute query with code(%d)", r);
	}

	int i = 0;
	int n = r;
	for ( i = 0; i < n; i++ ) {
		r = cli_get_next(statement);
		if (r != cli_ok) {
			return luaL_error(L, "Failed to get next with code(%d)", r);
		}
		
		printf("after insert 0x%08X \n", rt.activity_name);
		printf("Record activity_id(0x%llX) activity_name(%s) \n", rt.activity_id, rt.activity_name);
	}

	return 0;
}

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
		{ "test", ltest },
		{ NULL, NULL },
	};

	luaL_newlib(L, l);
	return 1;
}
