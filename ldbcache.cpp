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

#define FIELD_TABLE_OBJECT 1

#define CASE_ERROR_CODE(x) \
	case x: \
		return #x; \
		break; \

static const char*
get_cli_error_msg(int cli_code) {
	switch(cli_code) {
		CASE_ERROR_CODE(cli_bad_address);
		CASE_ERROR_CODE(cli_connection_refused);
		CASE_ERROR_CODE(cli_database_not_found);
		CASE_ERROR_CODE(cli_bad_statement);
		CASE_ERROR_CODE(cli_parameter_not_found);
		CASE_ERROR_CODE(cli_unbound_parameter);
		CASE_ERROR_CODE(cli_column_not_found);
		CASE_ERROR_CODE(cli_incompatible_type);
		CASE_ERROR_CODE(cli_network_error);
		CASE_ERROR_CODE(cli_runtime_error);
		CASE_ERROR_CODE(cli_bad_descriptor);
		CASE_ERROR_CODE(cli_unsupported_type);
		CASE_ERROR_CODE(cli_not_found);
		CASE_ERROR_CODE(cli_not_update_mode);
		CASE_ERROR_CODE(cli_table_not_found);
		CASE_ERROR_CODE(cli_not_all_columns_specified);
		CASE_ERROR_CODE(cli_not_fetched);
		CASE_ERROR_CODE(cli_already_updated);
		CASE_ERROR_CODE(cli_table_already_exists);
		CASE_ERROR_CODE(cli_not_implemented);
		CASE_ERROR_CODE(cli_xml_parse_error);
		CASE_ERROR_CODE(cli_backup_failed);
		default:
			return "Unknown error";
			break;
	}
	return "";
}

static int g_session = 0;

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
	int r = cli_attach( session );
	if( r != cli_ok )
		return luaL_error(L, "Failed to begin(%s)", get_cli_error_msg(r));
	return 0;
}

static int
lcommit(lua_State *L) {
	int session = lua_tointeger(L, 1);
	int r = cli_commit(session);
	if( r != cli_ok )
		return luaL_error(L, "Failed to commit(%s)", get_cli_error_msg(r));
	r = cli_detach(session, cli_commit_on_detach);
	if( r != cli_ok )
		return luaL_error(L, "Failed to detach(%s)", get_cli_error_msg(r));
	return 0;
}

static int
lrollback(lua_State *L) {
	int session = lua_tointeger(L, 1);
	int r = cli_abort(session);
	if( r != cli_ok )
		return luaL_error(L, "Failed to rollback(%s)", get_cli_error_msg(r));
	r = cli_detach(session, cli_commit_on_detach);
	if( r != cli_ok )
		return luaL_error(L, "Failed to detach(%s)", get_cli_error_msg(r));
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

//////////////////////////////////////////////////////////////////////////////////////////////////
/* table record define */
struct tb_activity_record {
	cli_int8_t activity_id;
	char activity_name[100];
	cli_int8_t status;
};

/* table field define */
struct cli_field_descriptor tb_activity_descriptor[] = {
		{ cli_int8, cli_indexed, "activity_id", },
		{ cli_asciiz, 0, "activity_name" },
		{ cli_int8, cli_indexed, "status", },		
};

static int
lloadtable(lua_State *L) {
	int rc = cli_create_table(g_session, "tb_activity", sizeof(tb_activity_descriptor)/sizeof(cli_field_descriptor), tb_activity_descriptor);
	if ( rc != cli_ok ) {		
		return luaL_error(L, "Failed to create table(%s)", get_cli_error_msg(rc));
	}
	return 0;
}

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
	if (len > sizeof(g_dbrecord._tb_activity_record.activity_name)-1 ) {
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
tb_activity_getactivity_id(lua_State *L) {
	lua_pushinteger(L, g_dbrecord._tb_activity_record.activity_id);
	return 1;
}

static int
tb_activity_getactivity_name(lua_State *L) {
	lua_pushstring(L, g_dbrecord._tb_activity_record.activity_name);
	return 1;
}

static int
tb_activity_getstatus(lua_State *L) {
	lua_pushinteger(L, g_dbrecord._tb_activity_record.status);
	return 1;
}

static int
tb_activity_insert(lua_State *L) {
	int statement = cli_statement(g_session, "insert into tb_activity");
	if ( statement < 0 ) {
		return luaL_error(L, "Failed to create statement(%s)", get_cli_error_msg(statement));
	}

	int rc;
	if( ( rc = cli_column(statement, "activity_id", cli_int8, NULL, &g_dbrecord._tb_activity_record.activity_id ) != cli_ok ) ||
		( rc = cli_column(statement, "activity_name", cli_asciiz, NULL, g_dbrecord._tb_activity_record.activity_name ) != cli_ok ) ||
		( rc = cli_column(statement, "status", cli_int8, NULL, &g_dbrecord._tb_activity_record.status ) != cli_ok )
		) {
		return luaL_error(L, "Failed to call cli_column(%s)", get_cli_error_msg(rc));
	}

	cli_oid_t oid;
	rc = cli_insert(statement, &oid);
	if ( rc != cli_ok ) {
		return luaL_error(L, "Failed to insert(%s)", get_cli_error_msg(rc));
	}

	rc = cli_free(statement);
	if ( rc != cli_ok ) {
		return luaL_error(L, "Failed to free statement(%s)", get_cli_error_msg(rc));
	}

	return 0;
}

static tb_activity_record find_record;

static int
tb_activity_prepare(lua_State *L) {
	const char *subsql = lua_tostring(L, 1);
	char sql[255] = {0};
	snprintf(sql, sizeof(sql), "select * from tb_activity where %s", subsql);
	g_statement._tb_activity_statement = cli_statement(g_session, sql);
	if ( g_statement._tb_activity_statement < 0 ) {
		return luaL_error(L, "Failed to prepare query(%s)", get_cli_error_msg(g_statement._tb_activity_statement));
	}

	memset(&g_dbrecord._tb_activity_record, 0x00, sizeof(g_dbrecord._tb_activity_record));
	memset(&find_record, 0x00, sizeof(find_record));

	int rc;
	if ((rc=cli_column(g_statement._tb_activity_statement, "activity_id", cli_int8, NULL, &g_dbrecord._tb_activity_record.activity_id)) != cli_ok
        || (rc=cli_column(g_statement._tb_activity_statement, "activity_name", cli_asciiz, NULL, g_dbrecord._tb_activity_record.activity_name)) != cli_ok 
		|| (rc=cli_column(g_statement._tb_activity_statement, "status", cli_int8, NULL, &g_dbrecord._tb_activity_record.status)) != cli_ok 
		) {
		return luaL_error(L, "Failed to call cli_column(%s)", get_cli_error_msg(rc));
    }
	return 0;
}

static int
tb_activity_findsetactivity_id(lua_State *L) {
	find_record.activity_id = lua_tointeger(L, 1);

	int rc;
	if ( (rc = cli_parameter(g_statement._tb_activity_statement, "%activity_id", cli_int8, &find_record.activity_id)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }
	return 0;
}

static int
tb_activity_findsetactivity_name(lua_State *L) {
	size_t len = 0;
	const char *v = lua_tolstring(L, 1, &len);
	if (len > sizeof(find_record.activity_name)-1 ) {
		len = sizeof(find_record.activity_name)-1;
	}
	memcpy(find_record.activity_name, v, len);

	int rc;
	if ( (rc = cli_parameter(g_statement._tb_activity_statement, "%activity_name", cli_asciiz, (void*)find_record.activity_name)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }
	return 0;
}

static int
tb_activity_findsetstatus(lua_State *L) {
	find_record.status = lua_tointeger(L, 1);

	int rc;
	if ( (rc = cli_parameter(g_statement._tb_activity_statement, "%status", cli_int8, &find_record.status)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }
	return 0;
}

static int
tb_activity_find(lua_State *L) {
	int rc = cli_fetch(g_statement._tb_activity_statement, cli_view_only);
    if ( rc < 0 ) { 
		return luaL_error(L, "Failed to call cli_fetch(%s)", get_cli_error_msg(rc));
    }
	lua_pushinteger(L, rc);
	return 1;
}

static int
tb_activity_next(lua_State *L) {
	int rc;
	if ( (rc = cli_get_next(g_statement._tb_activity_statement)) == cli_ok ) {
		lua_pushboolean(L, true);
	}
	else {
		lua_pushboolean(L, false);
	}
	return 1;
}

static int
tb_activity_update(lua_State *L) {
	//todo freee statement when error occur
	int rc;
	int statement = cli_statement(g_session, "select * from tb_activity where activity_id = %activity_id");
    if (statement < 0) { 
        return luaL_error(L, "Failed to call cli_statement(%s)", get_cli_error_msg(statement));
    }

	tb_activity_record r;
	memset(&r, 0x00, sizeof(r));

	if ((rc=cli_column(statement, "activity_id", cli_int8, NULL, &r.activity_id)) != cli_ok
        || (rc=cli_column(statement, "activity_name", cli_asciiz, NULL, r.activity_name)) != cli_ok
		|| (rc=cli_column(statement, "status", cli_int8, NULL, &r.status)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_column(%s)", get_cli_error_msg(rc));
    }

    if ( (rc = cli_parameter(statement, "%activity_id", cli_int8, &g_dbrecord._tb_activity_record.activity_id)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }

    rc = cli_fetch(statement, cli_for_update);
    if ( rc < 0 ) { 
		return luaL_error(L, "Failed to call cli_fetch(%s)", get_cli_error_msg(rc));
    }

	while ((rc = cli_get_next(statement)) == cli_ok) { 
			r.activity_id = g_dbrecord._tb_activity_record.activity_id;
			memcpy(r.activity_name, g_dbrecord._tb_activity_record.activity_name, sizeof(r.activity_name));
			r.status = g_dbrecord._tb_activity_record.status;

			rc = cli_update(statement);
			if ( rc != cli_ok ) {
				return luaL_error(L, "Failed to call cli_update(%s)", get_cli_error_msg(rc));
			}
    }

	rc = cli_free(statement);
    if (rc != cli_ok) { 
        return luaL_error(L, "Failed to call cli_free(%s)", get_cli_error_msg(rc));
    }

	return 0;
}

static int
tb_activity_remove(lua_State *L) {
	//todo freee statement when error occur
	int rc;
	int statement = cli_statement(g_session, "select * from tb_activity where activity_id = %activity_id");
    if (statement < 0) { 
        return luaL_error(L, "Failed to call cli_statement(%s)", get_cli_error_msg(statement));
    }

	if ( (rc = cli_parameter(statement, "%activity_id", cli_int8, &g_dbrecord._tb_activity_record.activity_id)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }

    rc = cli_fetch(statement, cli_for_update);
    if ( rc < 0 ) { 
        return luaL_error(L, "Failed to call cli_fetch(%s)", get_cli_error_msg(rc));
    }

	rc = cli_remove(statement);
	if ( rc != cli_ok ) {
		return luaL_error(L, "Failed to call cli_remove(%s)", get_cli_error_msg(rc));
	}

	rc = cli_free(statement);
    if (rc != cli_ok) { 
        return luaL_error(L, "Failed to call cli_free(%s)", get_cli_error_msg(rc));
    }

	return 0;
}

#define DECLARE_FUNCTION(tablename, funcname) \
do \
{ \
	lua_pushliteral(L, #funcname); \
	lua_pushcfunction(L, tablename##_##funcname ); \
	lua_settable(L, -3); \
} \
while (0); \

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

	// tb_activity
	lua_pushliteral(L, "tb_activity");
	lua_newtable(L);
	DECLARE_FUNCTION(tb_activity, reset);
	DECLARE_FUNCTION(tb_activity, setactivity_id);
	DECLARE_FUNCTION(tb_activity, setactivity_name);
	DECLARE_FUNCTION(tb_activity, setstatus);
	DECLARE_FUNCTION(tb_activity, getactivity_id);
	DECLARE_FUNCTION(tb_activity, getactivity_name);
	DECLARE_FUNCTION(tb_activity, getstatus);
	DECLARE_FUNCTION(tb_activity, insert);
	DECLARE_FUNCTION(tb_activity, prepare);
	DECLARE_FUNCTION(tb_activity, find);
	DECLARE_FUNCTION(tb_activity, next);
	DECLARE_FUNCTION(tb_activity, findsetactivity_id);
	DECLARE_FUNCTION(tb_activity, findsetactivity_name);
	DECLARE_FUNCTION(tb_activity, findsetstatus);
	DECLARE_FUNCTION(tb_activity, update);
	DECLARE_FUNCTION(tb_activity, remove);
	lua_settable(L, -3);

	// sharing previous table as upvalue
	luaL_setfuncs(L, l, 1);

	return 1;
}
