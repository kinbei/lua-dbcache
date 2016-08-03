#ifndef DBFUNC_H
#define DBFUNC_H

extern "C" {  
#include "lua.h"
#include "lauxlib.h"
}
#include "fastdb/cli.h"
#include "global.h"

#include "mysql.h"
#include "queue.h"

struct tb_activity_record {
	cli_int8_t activity_id;
	char activity_name[100];
	cli_int8_t status;
};

static cli_field_descriptor tb_activity_descriptor[] = {
		{ cli_int8, cli_indexed, "activity_id", },
		{ cli_asciiz, 0, "activity_name" },
		{ cli_int8, cli_indexed, "status", },		
};

//////////////////////////////////////////////////////////////////////////////////////////////////
struct dbrecord {
	tb_activity_record _tb_activity_record;
};

struct dbprecord {
	tb_activity_record _tb_activity_record;
};

struct dbstatement {
	int _tb_activity_statement;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
int tb_activity_reset(lua_State *L);
int tb_activity_setactivity_id(lua_State *L);
int tb_activity_setactivity_name(lua_State *L);
int tb_activity_setstatus(lua_State *L);
int tb_activity_getactivity_id(lua_State *L);
int tb_activity_getactivity_name(lua_State *L);
int tb_activity_getstatus(lua_State *L);
int tb_activity_insert_db(lua_State *L);
int tb_activity_insert_mdb(lua_State *L);
int tb_activity_insert(lua_State *L);
int tb_activity_prepare(lua_State *L);
int tb_activity_findsetactivity_id(lua_State *L);
int tb_activity_findsetactivity_name(lua_State *L);
int tb_activity_findsetstatus(lua_State *L);
int tb_activity_find(lua_State *L);
int tb_activity_next(lua_State *L);
int tb_activity_update(lua_State *L);
int tb_activity_remove(lua_State *L);

void create_table_objects(lua_State *L);
int load_tables(lua_State *L);

//////////////////////////////////////////////////////////////////////////////////////////////////
extern int g_session;
extern struct dbrecord g_dbrecord;
extern struct dbprecord g_dbprecord;
extern struct dbstatement g_statement;

extern MYSQL *g_mysqlconn;

#define INSERT_SQL 1
#define UPDATE_SQL 2
#define DELETE_SQL 3

struct sqldao {
	char* sql;
	int type;  // update type of sql
};

extern queue g_sqlqueue;
#endif
