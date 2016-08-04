#include "dbfunc.h"
#include <string.h>

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

int 
tb_activity_reset(lua_State *L) {
	tb_activity_record &record = g_dbrecord._tb_activity_record;
	memset(&record, 0x00, sizeof(record));
	return 0;
}

int
tb_activity_setactivity_id(lua_State *L) {
	tb_activity_record &record = g_dbrecord._tb_activity_record;
	lua_Integer v = lua_tointeger(L, 1);
	record.activity_id = v;
	return 0;
}

int
tb_activity_setactivity_name(lua_State *L) {
	tb_activity_record &record = g_dbrecord._tb_activity_record;
	size_t len = 0;
	const char *v = lua_tolstring(L, 1, &len);
	memcpy(record.activity_name, v, MIN(len, sizeof(record.activity_name) - 2));
	record.activity_name[ MIN(len+1, sizeof(record.activity_name) - 1) ] = 0;
	return 0;
}

int
tb_activity_getactivity_id(lua_State *L) {
	tb_activity_record &record = g_dbrecord._tb_activity_record;
	lua_pushinteger(L, record.activity_id);
	return 1;
}

int
tb_activity_getactivity_name(lua_State *L) {
	tb_activity_record &record = g_dbrecord._tb_activity_record;
	lua_pushstring(L, record.activity_name); // todo use lua_pushlstring for binary data
	return 1;
}

int tb_activity_insert_db(lua_State *L) {
	tb_activity_record &record = g_dbrecord._tb_activity_record;
	sqldao dao;
	dao.sql = (char*)malloc(1000); // todo setbytes for binary data
	snprintf(dao.sql, 1000, "insert into tb_activity values (%ld, '%s');", record.activity_id, record.activity_name);
	queue_push(&g_sqlqueue, &dao);
	return 0;
}

int tb_activity_insert_mdb(lua_State *L) {
	tb_activity_record &record = g_dbrecord._tb_activity_record;
	int statement = cli_statement(g_session, "insert into tb_activity");
	if ( statement < 0 ) {
		return luaL_error(L, "Failed to create statement(%s)", get_cli_error_msg(statement));
	}

	int rc;
	rc = cli_column(statement, "activity_id", cli_int8, NULL, &record.activity_id );
	if ( rc != cli_ok ) {
		cli_free(statement);
		return luaL_error(L, "Failed to call cli_column(%s)", get_cli_error_msg(rc));
	}
	rc = cli_column(statement, "activity_name", cli_asciiz, NULL, record.activity_name );
	if ( rc != cli_ok ) {
		cli_free(statement);
		return luaL_error(L, "Failed to call cli_column(%s)", get_cli_error_msg(rc));
	}

	cli_oid_t oid;
	rc = cli_insert(statement, &oid);
	if ( rc != cli_ok ) {
		cli_free(statement);
		return luaL_error(L, "Failed to insert(%s)", get_cli_error_msg(rc));
	}

	rc = cli_free(statement);
	if ( rc != cli_ok ) {
		return luaL_error(L, "Failed to free statement(%s)", get_cli_error_msg(rc));
	}
	return 0;
}

int
tb_activity_insert(lua_State *L) {
	tb_activity_insert_mdb(L);
	tb_activity_insert_db(L);
	return 0;
}

int
tb_activity_prepare(lua_State *L) {
	tb_activity_record &record = g_dbrecord._tb_activity_record;
	tb_activity_record &precord = g_dbprecord._tb_activity_record;
	int &statement = g_statement._tb_activity_statement;
	int rc;

	if ( statement > 0 ) {
		rc = cli_free(statement);
		if ( rc != cli_ok ) {
			return luaL_error(L, "Failed to call cli_column(%s) activity_name", get_cli_error_msg(rc));
		}
	}

	const char *subsql = lua_tostring(L, 1);
	char sql[255] = {0};
	snprintf(sql, sizeof(sql), "select * from tb_activity where %s", subsql);
	statement = cli_statement(g_session, sql);
	if ( statement < 0 ) {
		return luaL_error(L, "Failed to prepare query(%s)", get_cli_error_msg(statement));
	}

	memset(&record, 0x00, sizeof(record));
	memset(&precord, 0x00, sizeof(precord));

	rc = cli_column(statement, "activity_id", cli_int8, NULL, &record.activity_id);
	if ( rc != cli_ok ) {
		cli_free(statement);
		return luaL_error(L, "Failed to call cli_column(%s) activity_id", get_cli_error_msg(rc));
	}
	rc = cli_column(statement, "activity_name", cli_asciiz, NULL, record.activity_name);
	if ( rc != cli_ok ) {
		cli_free(statement);
		return luaL_error(L, "Failed to call cli_column(%s) activity_name", get_cli_error_msg(rc));
	}

	// t["tb_activity"] = statement
	lua_pushstring(L, "tb_activity");
	lua_pushinteger(L, statement);
	lua_settable(L, lua_upvalueindex(1));
	return 0;
}

int
tb_activity_findsetactivity_id(lua_State *L) {
	tb_activity_record &precord = g_dbprecord._tb_activity_record;
	int &statement = g_statement._tb_activity_statement;
	precord.activity_id = lua_tointeger(L, 1);

	int rc;
	if ( (rc = cli_parameter(statement, "%activity_id", cli_int8, &precord.activity_id)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }
	return 0;
}

int
tb_activity_findsetactivity_name(lua_State *L) {
	tb_activity_record &precord = g_dbprecord._tb_activity_record;
	int &statement = g_statement._tb_activity_statement;
	size_t len = 0;
	const char *v = lua_tolstring(L, 1, &len);
	memcpy(precord.activity_name, v, MIN(len, sizeof(precord.activity_name)-2));
	precord.activity_name[sizeof(precord.activity_name)-1] = 0;

	int rc;
	if ( (rc = cli_parameter(statement, "%activity_name", cli_asciiz, (void*)precord.activity_name)) != cli_ok ) {
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }
	return 0;
}

int
tb_activity_find(lua_State *L) {
	int &statement = g_statement._tb_activity_statement;
	int rc = cli_fetch(statement, cli_view_only);
    if ( rc < 0 ) { 
		return luaL_error(L, "Failed to call cli_fetch(%s)", get_cli_error_msg(rc));
    }
	lua_pushinteger(L, rc);
	return 1;
}

int
tb_activity_next(lua_State *L) {
	int &statement = g_statement._tb_activity_statement;
	int rc;
	if ( (rc = cli_get_next(statement)) == cli_ok ) {
		lua_pushboolean(L, true);
	}
	else {
		lua_pushboolean(L, false);
	}
	return 1;
}

int
tb_activity_update(lua_State *L) {
	tb_activity_record &record = g_dbrecord._tb_activity_record;

	int rc;
	int statement = cli_statement(g_session, "select * from tb_activity where activity_id = %activity_id");
    if (statement < 0) { 
        return luaL_error(L, "Failed to call cli_statement(%s)", get_cli_error_msg(statement));
    }

	tb_activity_record r;
	memset(&r, 0x00, sizeof(r));

	rc = cli_column(statement, "activity_id", cli_int8, NULL, &r.activity_id);
	if ( rc != cli_ok ) {
		cli_free(statement);
		return luaL_error(L, "Failed to call cli_column(%s) activity_id", get_cli_error_msg(rc));
	}
	rc = cli_column(statement, "activity_name", cli_asciiz, NULL, r.activity_name);
	if ( rc != cli_ok ) {
		cli_free(statement);
		return luaL_error(L, "Failed to call cli_column(%s) activity_id", get_cli_error_msg(rc));
	}
    if ( (rc = cli_parameter(statement, "%activity_id", cli_int8, &record.activity_id)) != cli_ok ) {
		cli_free(statement);
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }

    rc = cli_fetch(statement, cli_for_update);
    if ( rc < 0 ) {
		cli_free(statement);
		return luaL_error(L, "Failed to call cli_fetch(%s)", get_cli_error_msg(rc));
    }

	while ((rc = cli_get_next(statement)) == cli_ok) { 
			r.activity_id = record.activity_id;
			memcpy(r.activity_name, record.activity_name, sizeof(r.activity_name));

			rc = cli_update(statement);
			if ( rc != cli_ok ) {
				cli_free(statement);
				return luaL_error(L, "Failed to call cli_update(%s)", get_cli_error_msg(rc));
			}

			sqldao dao;
			dao.sql = (char*)malloc(1000);
			snprintf(dao.sql, 1000, "update tb_activity set activity_name = '%s' where activity_id = %ld;", record.activity_name, record.activity_id);
			queue_push(&g_sqlqueue, &dao);
    }

	rc = cli_free(statement);
    if (rc != cli_ok) { 
        return luaL_error(L, "Failed to call cli_free(%s)", get_cli_error_msg(rc));
    }

	return 0;
}

int
tb_activity_remove(lua_State *L) {
	tb_activity_record &record = g_dbrecord._tb_activity_record;

	int rc;
	int statement = cli_statement(g_session, "select * from tb_activity where activity_id = %activity_id");
    if (statement < 0) { 
        return luaL_error(L, "Failed to call cli_statement(%s)", get_cli_error_msg(statement));
    }

	if ( (rc = cli_parameter(statement, "%activity_id", cli_int8, &record.activity_id)) != cli_ok ) {
		cli_free(statement);
        return luaL_error(L, "Failed to call cli_parameter(%s)", get_cli_error_msg(rc));
    }

    rc = cli_fetch(statement, cli_for_update);
    if ( rc < 0 ) {
		cli_free(statement);
        return luaL_error(L, "Failed to call cli_fetch(%s)", get_cli_error_msg(rc));
    }

	rc = cli_remove(statement);
	if ( rc != cli_ok ) {
		cli_free(statement);
		return luaL_error(L, "Failed to call cli_remove(%s)", get_cli_error_msg(rc));
	}

	sqldao dao;
	dao.sql = (char*)malloc(1000);
	snprintf(dao.sql, 1000, "delete from tb_activity where activity_id = %ld;", record.activity_id);
	queue_push(&g_sqlqueue, &dao);
	
	rc = cli_free(statement);
    if (rc != cli_ok) { 
        return luaL_error(L, "Failed to call cli_free(%s)", get_cli_error_msg(rc));
    }

	return 0;
}

#define DECLARE_FUNCTION(tablename, funcname) { #funcname, tablename##_##funcname  }

void create_table_objects(lua_State *L) {
	lua_newtable(L); // upvalue, store all statement that need to free
	int idx = lua_gettop(L);

	// tb_activity - begin
	lua_pushliteral(L, "tb_activity");
	luaL_Reg l[] = {
		DECLARE_FUNCTION(tb_activity, reset),
		DECLARE_FUNCTION(tb_activity, setactivity_id),
		DECLARE_FUNCTION(tb_activity, setactivity_name),
		DECLARE_FUNCTION(tb_activity, getactivity_id),
		DECLARE_FUNCTION(tb_activity, getactivity_name),
		DECLARE_FUNCTION(tb_activity, insert),
		DECLARE_FUNCTION(tb_activity, prepare),
		DECLARE_FUNCTION(tb_activity, find),
		DECLARE_FUNCTION(tb_activity, next),
		DECLARE_FUNCTION(tb_activity, findsetactivity_id),
		DECLARE_FUNCTION(tb_activity, findsetactivity_name),
		DECLARE_FUNCTION(tb_activity, update),
		DECLARE_FUNCTION(tb_activity, remove),
		{ NULL, NULL },
	};
	luaL_newlibtable(L, l);
	lua_pushvalue(L, idx);
	luaL_setfuncs(L, l, 1);
	// tb_activity - end

	lua_settable(L, -4);
}

int load_tables(lua_State *L) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	if ( mysql_real_query(g_mysqlconn, "begin", 5) != 0 ) {
		return luaL_error(L, "Failed to mysql_real_query, error(%s)", mysql_error(g_mysqlconn));
	}
	
	// load tb_activity begin
	if ( mysql_real_query(g_mysqlconn, "select * from tb_activity", 25) ) {
		return luaL_error(L, "Failed to mysql_real_query, error(%s)", mysql_error(g_mysqlconn));
	}
	res = mysql_use_result(g_mysqlconn);
	while ( (row = mysql_fetch_row(res)) ) {
		tb_activity_reset(L);
		tb_activity_record &record = g_dbrecord._tb_activity_record;
		record.activity_id = atoi(row[0]);
		size_t len = strlen(row[1]); // todo support binary
		memcpy(record.activity_name, row[1], MIN(len, sizeof(record.activity_name)-2));
		record.activity_name[sizeof(record.activity_name) - 1] = 0;
		tb_activity_insert_mdb(L);
	}
	// load tb_activity end

	if ( mysql_commit(g_mysqlconn) != 0 ) {
		return luaL_error(L, "Failed to mysql_commit, error(%s)", mysql_error(g_mysqlconn));
	}

	return 0;
}

void free_statement() {
	while ( queue_empty(&g_freestatement) ) {
		int statement = 0;
		queue_pop(&g_freestatement, &statement);
		if ( statement != 0 ) {
			cli_free(statement);
		}
	}
}

int g_session = 0;
struct dbrecord g_dbrecord;
struct dbprecord g_dbprecord;
struct dbstatement g_statement;

MYSQL *g_mysqlconn = NULL;

queue g_sqlqueue;
queue g_freestatement;
