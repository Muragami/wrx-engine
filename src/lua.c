/*
	wrx-engine: lua linkage functions

	Jason A. Petrasko, muragami, muragami@wishray.com 2023

	MIT License
*/

#include "wrx.h"

// *********************************************************
// forward defines for lua functions
int lfwrxEmit(lua_State *L);

// *********************************************************
// forward defines for lua functions
void lwrxRegister(lua_State *L) {
	lua_getglobal(L, "wrx");
	lua_pushstring(L, WRX_VERSION);
		lua_setfield(L, -2, "version");
	lua_pushstring(L, WRX_NAME);
		lua_setfield(L, -2, "name");
	lua_pushstring(L, WRX_CODENAME);
		lua_setfield(L, -2, "codename");
	lua_newtable(L);
		lua_setfield(L, -2, "cfg");
	lua_pushcfunction(L, lfwrxEmit);
		lua_setfield(L, -2, "emit");
	lua_pop(L, 1);

	lua_pushnil(L);
		lua_setglobal(L, "require");
	lua_pushnil(L);
		lua_setglobal(L, "loadfile");
	lua_pushnil(L);
		lua_setglobal(L, "dofile");
}

const char *lwrxDataReader(lua_State *L, void *data, size_t *size)
{
	wrxData *p = data;

	if (p->flags & WRX_READ_FLAG) {
		*size = 0;
		p->flags -= WRX_READ_FLAG;
		return NULL;
	} else {
		*size = p->end;
		p->flags |= WRX_READ_FLAG;
		return p->memory;	
	}
	
}

int lwrxLoadString(wrxState *p, wrxData *src, const char *name) {
	return lua_load(p->L, lwrxDataReader, src, name, NULL);
}

void lwrxFieldToInteger(wrxState *p, int index, const char *name, int *v) {
	lua_getfield(p->L, index, name);
	*v = lua_tointeger(p->L, -1);
	lua_pop(p->L, 1);
}

void lwrxFieldToString(wrxState *p, int index, const char *name, char *buffer, int bsize) {
	lua_getfield(p->L, index, name);
	strncpy(buffer, lua_tostring(p->L, -1), bsize);
	lua_pop(p->L, 1);
}

void lwrxFieldToFloat(wrxState *p, int index, const char *name, float *v) {
	lua_getfield(p->L, index, name);
	*v = lua_tonumber(p->L, -1);
	lua_pop(p->L, 1);
}

void lwrxFieldToDouble(wrxState *p, int index, const char *name, double *v) {
	lua_getfield(p->L, index, name);
	*v = lua_tonumber(p->L, -1);
	lua_pop(p->L, 1);
}

int lfwrxEmit(lua_State *L) {
	const char *s = luaL_checkstring(L, 1);
	printf("%s\n", s);
	return 0;
}