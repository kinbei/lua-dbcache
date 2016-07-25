#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "lua.h"
#include "lauxlib.h"

int
luaopen_dbcache_core(lua_State *L) {
	luaL_checkversion(L);

	luaL_Reg l[] = {
		{ "readuint8", lreaduint8 },
		{ "readuint16", lreaduint16 },
		{ "readuint32", lreaduint32 },
		{ "readuint64", lreaduint64 },
		{ "readnumber", lreadnumber },
		{ "readstring", lreadstring },
		{ "readsize", lreadsize },
				
		{ NULL, NULL },
	};

	luaL_newlib(L, l);
	return 1;
}
