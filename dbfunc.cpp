#include "dbfunc.h"
#include <string.h>

int 
tb_activity_reset(lua_State *L) {
	memset(&g_dbrecord._tb_activity_record, 0x00, sizeof(g_dbrecord._tb_activity_record));
	return 0;
}

int
tb_activity_setactivity_id(lua_State *L) {
	lua_Integer v = lua_tointeger(L, 1);
	g_dbrecord._tb_activity_record.activity_id = v;
	return 0;
}

int
tb_activity_setactivity_name(lua_State *L) {
	size_t len = 0;
	const char *v = lua_tolstring(L, 1, &len);
	if (len > sizeof(g_dbrecord._tb_activity_record.activity_name)-1 ) {
		len = sizeof(g_dbrecord._tb_activity_record.activity_name)-1;
	}
	memcpy(g_dbrecord._tb_activity_record.activity_name, v, len);
	return 0;
}

int
tb_activity_setstatus(lua_State *L) {
	lua_Integer v = lua_tointeger(L, 1);
	g_dbrecord._tb_activity_record.status = v;
	return 0;
}

int
tb_activity_getactivity_id(lua_State *L) {
	lua_pushinteger(L, g_dbrecord._tb_activity_record.activity_id);
	return 1;
}

int
tb_activity_getactivity_name(lua_State *L) {
	lua_pushstring(L, g_dbrecord._tb_activity_record.activity_name);
	return 1;
}

int
tb_activity_getstatus(lua_State *L) {
	lua_pushinteger(L, g_dbrecord._tb_activity_record.status);
	return 1;
}

int
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

int
tb_activity_prepare(lua_State *L) {
	const char *subsql = lua_tostring(L, 1);
	char sql[255] = {0};
	snprintf(sql, sizeof(sql), "select * from tb_activity where %s", subsql);
	g_statement._tb_activity_statement = cli_statement(g_session, sql);
	if ( g_statement._tb_activity_statement < 0 ) {
		return luaL_error(L, "Failed to prepare query(%s)", get_cli_error_msg(g_statement._tb_activity_statement));
	}

	memset(&g_dbrecord._tb_activity_record, 0x00, sizeof(g_dbrecord._tb_activity_record));
	memset(&g_dbprecord._tb_activity_record, 0x00, sizeof(g_dbprecord._tb_activity_record));

	int rc;
	if ((rc=cli_column(g_statement._tb_activity_statement, "activity_id", cli_int8, NULL, &g_dbrecord._tb_activity_record.activity_id)) != cli_ok
        || (rc=cli_column(g_statement._tb_activity_statement, "activity_name", cli_asciiz, NULL, g_dbrecord._tb_activity_record.activity_name)) != cli_ok 
		|| (rc=cli_column(g_statement._tb_activity_statement, "status", cli_int8, NULL, &g_dbrecord._tb_activity_record.status)) != cli_ok 
		) {
		return luaL_error(L, "Failed to call cli_column(%s)", get_cli_error_msg(rc));
    }
	return 0;
}

int
tb_activity_findsetactivity_id(lua_State *L) {
	g_dbprecord._tb_activity_record.activity_id = lua_tointeger(L, 1);

	int rc;
	if ( (rc = cli_parameter(g_statement._tb_activity_statement, "%activity_id", cli_int8, &g_dbprecord._tb_activity_record.activity_id)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }
	return 0;
}

int
tb_activity_findsetactivity_name(lua_State *L) {
	size_t len = 0;
	const char *v = lua_tolstring(L, 1, &len);
	if (len > sizeof(g_dbprecord._tb_activity_record.activity_name)-1 ) {
		len = sizeof(g_dbprecord._tb_activity_record.activity_name)-1;
	}
	memcpy(g_dbprecord._tb_activity_record.activity_name, v, len);

	int rc;
	if ( (rc = cli_parameter(g_statement._tb_activity_statement, "%activity_name", cli_asciiz, (void*)g_dbprecord._tb_activity_record.activity_name)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }
	return 0;
}

int
tb_activity_findsetstatus(lua_State *L) {
	g_dbprecord._tb_activity_record.status = lua_tointeger(L, 1);

	int rc;
	if ( (rc = cli_parameter(g_statement._tb_activity_statement, "%status", cli_int8, &g_dbprecord._tb_activity_record.status)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }
	return 0;
}

int
tb_activity_find(lua_State *L) {
	int rc = cli_fetch(g_statement._tb_activity_statement, cli_view_only);
    if ( rc < 0 ) { 
		return luaL_error(L, "Failed to call cli_fetch(%s)", get_cli_error_msg(rc));
    }
	lua_pushinteger(L, rc);
	return 1;
}

int
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

int
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

int
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

void create_table_object(lua_State *L) {
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
}


int g_session = 0;
struct dbrecord g_dbrecord;
struct dbprecord g_dbprecord;
struct dbstatement g_statement;
