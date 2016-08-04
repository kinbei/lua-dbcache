#ifndef DBFUNC_H
#define DBFUNC_H

extern "C" {  
#include "lua.h"
#include "lauxlib.h"
#include "queue.h"
}

#include "fastdb/cli.h"
#include "global.h"
#include "mysql.h"

// todo define in global.h
#define FIELD_TABLE_OBJECT 1
#define FIELD_TABLE_FREESTATEMENT 2

struct tb_activity_record {
	cli_int8_t activity_id;
	char activity_name[100];
};

static cli_field_descriptor tb_activity_descriptor[] = {
		{ cli_int8, cli_indexed, "activity_id", },
		{ cli_asciiz, 0, "activity_name" },
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

int tb_activity_getactivity_id(lua_State *L);
int tb_activity_getactivity_name(lua_State *L);

int tb_activity_insert_db(lua_State *L);
int tb_activity_insert_mdb(lua_State *L);
int tb_activity_insert(lua_State *L);

int tb_activity_prepare(lua_State *L);
int tb_activity_findsetactivity_id(lua_State *L);
int tb_activity_findsetactivity_name(lua_State *L);
int tb_activity_find(lua_State *L);
int tb_activity_next(lua_State *L);
int tb_activity_update(lua_State *L);
int tb_activity_remove(lua_State *L);

void create_table_objects(lua_State *L);
int load_tables(lua_State *L);
void free_statement();

//////////////////////////////////////////////////////////////////////////////////////////////////
extern int g_session;
extern struct dbrecord g_dbrecord;
extern struct dbprecord g_dbprecord;
extern struct dbstatement g_statement;
extern MYSQL *g_mysqlconn;
struct sqldao {
	char* sql;
};
extern queue g_sqlqueue;
extern queue g_freestatement;

#endif
