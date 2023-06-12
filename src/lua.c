/*
	wrx-engine: lua linkage functions

	Jason A. Petrasko, muragami, muragami@wishray.com 2023

	MIT License
*/

#include "wrx.h"
#include <string.h>

// *********************************************************
// forward defines for lua functions
int lfwrxEmit(lua_State *L);
int lfwrxShare(lua_State *L);
int lfwrxPushIO(lua_State *L, void *mem, unsigned long bytes, unsigned int local);

// *********************************************************
// back to the code
luaL_Reg wrxFuncTable[] = {
	{ "emit", lfwrxEmit },
	{ "share", lfwrxShare },
	{ NULL, NULL } };

void lwrxRegister(lua_State *L) {
	// get the global table
	lua_getglobal(L, "wrx");
	// set the version string
	lua_pushstring(L, WRX_VERSION);
		lua_setfield(L, -2, "version");
	// set the name string
	lua_pushstring(L, WRX_NAME);
		lua_setfield(L, -2, "name");
	// set the codename string		
	lua_pushstring(L, WRX_CODENAME);
		lua_setfield(L, -2, "codename");
	// make a table to hold our config
	lua_newtable(L);
		lua_setfield(L, -2, "cfg");

	// register our functions into global wrx
	int i = 0;
	while (wrxFuncTable[i].name != NULL) {
		lua_pushcfunction(L, wrxFuncTable[i].func);
		lua_setfield(L, -2, wrxFuncTable[i].name);
		i++;
	}

	// all done with wrx, pop it
	lua_pop(L, 1);

	// remove unsafe functions
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
		*size = p->count;
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

// *********************************************************
// lua multithreading shares
int lwrxIndexShare(lua_State *L) {
	return 0;
}

int lwrxNewIndexShare(lua_State *L) {
	return 0;
}

// *********************************************************
// wrx functions
int lfwrxEmit(lua_State *L) {
	const char *s = luaL_checkstring(L, 1);
	printf("%s\n", s);
	return 0;
}

int lfwrxShare(lua_State *L) {
	// first the metatable
	lua_newtable(L);
	// now the actual table
	lua_newtable(L);
	lua_pushcfunction(L, lwrxIndexShare);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lwrxNewIndexShare);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
	return 1;
}

/*
    Simple portable memory io for lua/luajit.
    muragami, muragami@wishray.com, Jason A. Petrasko 2023
    MIT License: https://opensource.org/licenses/MIT

    while most of the code is obviously in C, some of the state information
    is held in the memio lua table, specifically:

        integer index 1 = lightuserdata to the memory block
        integer index 2 = size in bytes of the memory block
        integer index 3 = current pos byte in the memory block
        integer index 4 = integer mode, normally 8 but can be: 8, 16, 24, 32, 48, 56, 64

    integer mode is the bits to push or consume on calls to get(), put(), which
    read/write little endian integers into the memory butter
*/

int lfwrxmiFree(lua_State *L);
int lfwrxmiLines(lua_State *L);
int lfwrxmiRead(lua_State *L);
int lfwrxmiWrite(lua_State *L);
int lfwrxmiSeek(lua_State *L);
int lfwrxmiCopy(lua_State *L);
int lfwrxmiTell(lua_State *L);
int lfwrxmiGet(lua_State *L);
int lfwrxmiPut(lua_State *L);
int lfwrxmiSet(lua_State *L);

int lfwrxPushIO(lua_State *L, void *mem, unsigned long bytes, unsigned int local) {
    lua_newtable(L);
    lua_pushcfunction(L, lfwrxmiLines);
    lua_setfield(L, -2, "lines");
    lua_pushcfunction(L, lfwrxmiRead);
    lua_setfield(L, -2, "read");
    lua_pushcfunction(L, lfwrxmiWrite);
    lua_setfield(L, -2, "write");
    lua_pushcfunction(L, lfwrxmiSeek);
    lua_setfield(L, -2, "seek");
    lua_pushcfunction(L, lfwrxmiCopy);
    lua_setfield(L, -2, "copy");
    lua_pushcfunction(L, lfwrxmiTell);
    lua_setfield(L, -2, "tell");
    lua_pushcfunction(L, lfwrxmiGet);
    lua_setfield(L, -2, "get");
    lua_pushcfunction(L, lfwrxmiPut);
    lua_setfield(L, -2, "put");
    lua_pushcfunction(L, lfwrxmiSet);
    lua_setfield(L, -2, "set");
    wrxMemIO *p = calloc(1, sizeof(wrxMemIO));
    p->mem = mem;
    p->length = bytes;
    p->imode = 8;
    p->local = local;
    lua_pushlightuserdata(L, p);
    lua_rawseti(L, -2, 1);
    return 1;
}


int lfwrxmiLineReaderFunc(lua_State *L) {
    lua_pushvalue(L, lua_upvalueindex(1));
    char *p = NULL, *ret;
    unsigned int len;
    wrxMemIO *m = (wrxMemIO*)lua_topointer(L, -1);
    lua_pop(L, 1);
    char *e = m->mem + m->length;
    char *s, *stop;
    p = m->mem + m->pos;
    s = p;
    while (p != e) {
        if (*p == '\r' || *p == '\n') {
            stop = p - 1;
            // consume any following empty lines
            while (p != e) {
                if (!(*p == '\r' || *p == '\n')) break;
                p++;
            }
            m->pos += p - s;
            if (stop == p) {
                // no string to return, return an empty
                lua_pushlstring(L, "", 0);
                return 1;
            } else {
                len = stop - s;
                ret = malloc(len);
                memcpy(ret, s, len);
                lua_pushlstring(L, ret, len);
                free(ret);
                return 1;
            }
        }
        p++;
    }
    if (p == s) {
        lua_pushnil(L);
        return 1;
    } else {
        len = p - s;
        ret = malloc(len);
        memcpy(ret, s, len);
        lua_pushlstring(L, ret, len);
        free(ret);
        return 1;
    }
}

/*
    for line in memio:lines() do end
*/
int lfwrxmiLines(lua_State *L) {
    lua_rawgeti(L, 1, 1);
    lua_pushcclosure(L, lfwrxmiLineReaderFunc, 1);
    return 1;
}

int lfwrxmiRead(lua_State *L) {

}

int lfwrxmiWrite(lua_State *L) {

}

/*
    -> memio:seek(whence, offset)

    seek to, from whence:
        "set": position 0
        "cur": current position
        "end": end of the memory block

    offset is from that point, and may be nil meaning 0
*/
int lfwrxmiSeek(lua_State *L) {
    lua_rawgeti(L, 1, 1);
    wrxMemIO *m = (wrxMemIO*)lua_topointer(L, -1);
    lua_pop(L,1);
    const char *w = luaL_checkstring(L, 2);
    int offset = 0;
    if (lua_isnumber(L, 3)) offset = lua_tointeger(L, 3);
    if (!strcmp(w, "set")) {
        if (offset < 0) luaL_error(L, "memio:seek() would seek passed beginning of memory block");
        m->pos = offset;
        if (m->pos > m->length) luaL_error(L, "memio:seek() would seek passed end of memory block");
        lua_pushinteger(L, m->pos);
        lua_pushvalue(L, -1);
        lua_rawseti(L, 1, 2);
        return 1;
    } else if (!strcmp(w, "cur")) {
        if (offset < 0 && abs(offset) > m->pos) luaL_error(L, "memio:seek() would seek passed beginning of memory block");
        m->pos += offset;
        if (m->pos > m->length) luaL_error(L, "memio:seek() would seek passed end of memory block");
        lua_pushinteger(L, m->pos);
        lua_pushvalue(L, -1);
        lua_rawseti(L, 1, 2);
        return 1;
    } else if (!strcmp(w, "end")) {
        if (offset > 0) luaL_error(L, "memio:seek() would seek passed end of memory block");
        m->pos = m->length + offset;
        if (m->pos > m->length) luaL_error(L, "memio:seek() would seek passed end of memory block");
        lua_pushinteger(L, m->pos);
        lua_pushvalue(L, -1);
        lua_rawseti(L, 1, 2);
        return 1;
    } else luaL_error(L, "memio:seek() improperly formatted call, bad whence argument");
    return 0;
}

/*
    -> memio:copy()

    returns an memio table that is a duplicate of this memory block (duplicates the memory too)

    -> memio:copy(start, end)

    returns an memio table that is a duplicate of this memory block, but only from start to end
*/
int lfwrxmiCopy(lua_State *L) {
    unsigned int t = lua_gettop(L);
    lua_rawgeti(L, 1, 1);
    wrxMemIO *m = (wrxMemIO*)lua_topointer(L, -1);
    lua_pop(L,1);
    
    if (t == 1) {
        unsigned char *n = malloc(m->length);
        if (n == NULL) luaL_error(L, "memio:copy() memory allocation failure");
        memcpy(n, m->mem, m->length);
        lfwrxPushIO(L, n, m->length, 1);
        return 1;
    } else if (t == 3) {
        unsigned int s = luaL_checkinteger(L, 2);
        unsigned int e = luaL_checkinteger(L, 3);
        if (e > m->length) luaL_error(L, "memio:copy() would exceed source memory length");
        if (e < s) luaL_error(L, "memio:copy() end is before start");
        unsigned int len = e - s + 1;
        unsigned char *n = malloc(len);
        memcpy(n, m->mem + s, len);
        lfwrxPushIO(L, n, len, 1);
        return 1;
    } else luaL_error(L, "memio:copy() improperly formatted call");
    lua_pushnil(L);
    return 1;
}

/*
    memio:tell(opts)

    opts is a character string telling what you want to know:
        "p" = current position byte
        "t" = total bytes
        "r" = remaining bytes
        "i" = integer bit width
*/
int lfwrxmiTell(lua_State *L) {
    if (!luaL_checkstring(L, 2)) luaL_error(L, "memio:tell() takes a string parameter");
    lua_rawgeti(L, 1, 1);
    wrxMemIO *m = (wrxMemIO*)lua_topointer(L, -1);
    lua_pop(L,1);
    const char *s = lua_tostring(L,2);
    switch (s[0]) {
        case 'p': 
            lua_pushinteger(L, m->pos);
            return 1;
        break;
        case 't': 
            lua_pushinteger(L, m->length);
            return 1; 
        break;
        case 'r':
            lua_pushinteger(L, m->length - m->pos);
            return 1;
        break;
        case 'i':
            lua_pushinteger(L, m->imode);
            return 1;
        break;
    }
    lua_pushnil(L);
    return 1;
}

/*
    -> memio:get()

    read the next integer width bytes from the memory and return that value

    -> memio:get(t, s, e)

    read the next integer width bytes into table t, starting at integer index s and ending at e, such that

        memio:get(t, 1, 10)

    reads 10 integer width byte number from the memory into table t[1] to t[10]

    -> memio:get(f, arg, rep)

    read rep integer width byte numbers, calling function f each time as so: f(memio, intval, arg, cnt)
*/
int lfwrxmiGet(lua_State *L) {
    unsigned int t = lua_gettop(L);
    lua_rawgeti(L, 1, 1);
    wrxMemIO *m = (wrxMemIO*)lua_topointer(L, -1);
    lua_pop(L, 1);
    unsigned int b = m->imode >> 3;
    unsigned long long v = 0, mul = 1;

    if (t == 1) {
        while (b-- > 0) {
            if (m->pos < m->length) {
                v += (unsigned long long)m->mem[m->pos] * mul;
                m->pos++;
                mul *= 256;
            }
        }
        lua_pushinteger(L, v);
        return 1;
    } else if (t == 4) {
        if (lua_istable(L, 2)) {
            unsigned int s = luaL_checkinteger(L, 3);
            unsigned int e = luaL_checkinteger(L, 4);
            for (unsigned int i = s; i <= e; i++) {
                unsigned int cb = b;
                v = 0;
                mul = 1;
                while (cb-- > 0) {
                    if (m->pos < m->length) {
                        v += (unsigned long long)m->mem[m->pos] * mul;
                        m->pos++;
                        mul *= 256;
                    }
                }
                lua_pushinteger(L, v);
                lua_rawseti(L, 2, i);
            }
        } else if (lua_isfunction(L, 2)) {
            unsigned int r = luaL_checkinteger(L, 4);
            unsigned int i = 0;
            while (r-- > 0) {
                unsigned int cb = b;
                v = 0;
                mul = 1;
                while (cb-- > 0) {
                    if (m->pos < m->length) {
                        v += (unsigned long long)m->mem[m->pos] * mul;
                        m->pos++;
                        mul *= 256;
                    }
                }
                lua_pushvalue(L, 2);
                lua_pushvalue(L, 1);
                lua_pushinteger(L, v);
                lua_pushvalue(L, 3);
                lua_pushinteger(L, ++i);
                lua_call(L, 4, 0);
            }
        } else luaL_error(L, "memio:get(,,) improperly formatted call");
    } else luaL_error(L, "memio:get() improperly formatted call");
    return 0;
}

/*
    -> memio:put(v)

    puts v as integer width bytes into the memory

    -> memio:get(t, s, e)

    read the next integer width bytes from table t, starting at integer index s and ending at e, such that

        memio:get(t, 1, 10)

    puts 10 integer width byte number from the table t[1] to t[10] into the memory buffer
*/
int lfwrxmiPut(lua_State *L) {
    unsigned long long v = 0, cv;
    unsigned int b;
    lua_rawgeti(L, 1, 1);
    wrxMemIO *m = (wrxMemIO*)lua_topointer(L, -1);
    lua_pop(L,1);
    b = m->imode >> 3;

    if (luaL_checkinteger(L, 2)) {
        v = lua_tointeger(L, 2);
        for (int i = 0; i < b; i++) {
            if (m->pos < m->length) {
                cv = v & (0xFF << (i * 8)) >> (i * 8);
                m->mem[m->pos] = (unsigned char)cv;
                m->pos++;
            }
        }
    } else if (lua_istable(L, 2)) {
        unsigned int s = luaL_checkinteger(L, 3);
        unsigned int e = luaL_checkinteger(L, 4);
        for (unsigned int i = s; i <= e; i++) {
            lua_rawgeti(L, 2, i);
            v = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
            for (int i = 0; i < b; i++) {
                if (m->pos < m->length) {
                    cv = v & (0xFF << (i * 8)) >> (i * 8);
                    m->mem[m->pos] = (unsigned char)cv;
                    m->pos++;
                }
            }
        }
    } else luaL_error(L, "memio:put() improperly formatted call");
    return 0;
}

/*
    memio:set(what, value)

    what is a character string saying what to set:
        "p" = current position byte
        "i" = integer bit width
*/
int lfwrxmiSet(lua_State *L) {
    if (!luaL_checkstring(L, 2)) luaL_error(L, "memio:tell() takes a string parameter");
    const char *s = lua_tostring(L,2);
    unsigned long v = lua_tointeger(L, 3);
    lua_rawgeti(L, 1, 1);
    wrxMemIO *m = (wrxMemIO*)lua_topointer(L, -1);
    lua_pop(L,1);
    switch (s[0]) {
        case 'p': 
            if (v > m->length) luaL_error(L, "memio:set() tried to set position passed end of memory block");
            if (v < 0) luaL_error(L, "memio:set() tried to set position before the start of memory block");
            m->pos = v;
        break;
        case 'i':
            switch (v) {
                case 8:
                case 16:
                case 24:
                case 32:
                case 48:
                case 56:
                case 64:
                    m->imode = v;
                break;
                default:
                    luaL_error(L, "memio:set('i',) called with invalid bit width");
                break;
            }
        break;
    }
    return 0;
}

int lfwrxmiFree(lua_State *L) {
    lua_rawgeti(L, 1, 1);
    wrxMemIO *m = (wrxMemIO*)lua_topointer(L, -1);
    lua_pop(L,1);
    if (m->local) {
        free (m->mem);
        free(m);
    } else luaL_error(L, "memio:free() attempted to be called on remote memory");
    return 0;
}


