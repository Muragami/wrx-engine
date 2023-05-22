/*
	wrx-engine: core functions

	Jason A. Petrasko, muragami, muragami@wishray.com 2023

	MIT License
*/

#include "wrx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int wrxError(wrxState *p, const char *fmt, ...) {
	va_list args;
  	va_start (args, fmt);
  	vsnprintf (p->error, WRX_LINE, fmt, args);
  	va_end (args);
  	printf("%s\n", p->error);
	return WRX_ERR;
}

// is this state running?
int wrxRunning(wrxState *p) { return WRX_RUNS(p->mode); }

// return last error string, or "" if no error
const char* wrxGetError(wrxState *p) { return p->error; }

// just intialize the state with a lua instance, and return it
wrxState *wrxNewState() {
	wrxState *ret;

	ret = calloc(1, sizeof(wrxState));

	if (ret == NULL) return NULL;

	ret->fpsTarget = 30.0f;
	ret->sleepUMS = 5000;
	ret->idBits = WRX_ID_BITS_16;
	ret->L = luaL_newstate();
	
	if (ret->L == NULL) {
		free(ret);
		return NULL;
	}

	// it might be a little crazy, but call the checkstack() because the docs say really should
	lua_checkstack (ret->L, 6);
	luaL_requiref(ret->L, "_G", luaopen_base, 1);
	luaL_requiref(ret->L, "coroutine", luaopen_coroutine, 1);
	luaL_requiref(ret->L, "math", luaopen_math, 1);
	luaL_requiref(ret->L, "string", luaopen_string, 1);
	luaL_requiref(ret->L, "table", luaopen_table, 1);
	luaL_requiref(ret->L, "utf8", luaopen_utf8, 1);
	lua_pop(ret->L, 6);

	if (PHYSFS_isInit() == 0) PHYSFS_init(NULL);

	// some default configuration
	lua_newtable(ret->L);
	lua_setglobal(ret->L, "wrx");
	lwrxRegister(ret->L);

	// internal stuff
	ret->gTable = dwrxNewTable(ret);
	for (int i = 0; i < 256; ret->gTree[i++] = NULL);
		
	return ret;
}

int wrxStart(wrxState *p, const char *app) {
	wrxData *srcFile;
	int ret, top, v;

	if (PHYSFS_mount(app, "/", 0) == 0) {
		// bad mount, so report the error
		return wrxError(p, "wrxStart() could not mount: %s", app);
	} else {
		top = lua_gettop(p->L);
		lua_getglobal(p->L, "wrx");
		// mount is ok, so now load the app, start with conf.lua
		srcFile = dwrxReadFile("conf.lua");
		if (srcFile == NULL)
			return wrxError(p, "wrxStart() could not locate conf.lua");
		ret = lwrxLoadString(p, srcFile, "conf.lua");
		if (ret != LUA_OK) {
			return wrxError(p, "wrxStart() lua error %s", lua_tostring(p->L, -1));
		}
		// read conf.lua, so let's execute that now
		ret = lua_pcall(p->L, 0, 0, 0);
		if (ret != LUA_OK) {
			return wrxError(p, "wrxStart() lua runtime error %s", lua_tostring(p->L, -1));	
		}
		// this install a configuration function, so call that now
		lua_getfield(p->L, -1, "conf");
		lua_getfield(p->L, -2, "cfg");
		ret = lua_pcall(p->L, 1, 0, 0);
		if (ret != LUA_OK) {
			return wrxError(p, "wrxStart() lua runtime error %s", lua_tostring(p->L, -1));	
		}
		lua_getfield(p->L, -1, "cfg");
		// now our wrx.cfg table is on top of stack, so probe it
		lwrxFieldToString(p, -1, "title", p->title, WRX_LINE);
		lwrxFieldToString(p, -1, "name", p->name, WRX_LINE);
		lwrxFieldToInteger(p, -1, "width", &p->width);
		lwrxFieldToInteger(p, -1, "height", &p->height);
		lwrxFieldToInteger(p, -1, "idBits", &v);
		if (v != 0) {
			
		}
		lwrxFieldToFloat(p, -1, "fps", &p->fpsTarget);

		lua_settop(p->L, top);
		dwrxFreeData(srcFile);

		// now main.lua
		srcFile = dwrxReadFile("main.lua");
		if (srcFile == NULL)
			return wrxError(p, "wrxStart() could not locate main.lua");
		ret = lwrxLoadString(p, srcFile, "main.lua");
		if (ret != LUA_OK) {
			return wrxError(p, "wrxStart() lua error %s", lua_tostring(p->L, -1));
		}
		// read main.lua, so let's execute that now
		ret = lua_pcall(p->L, 0, 0, 0);
		if (ret != LUA_OK) {
			return wrxError(p, "wrxStart() lua runtime error %s", lua_tostring(p->L, -1));	
		}

		// ok we have loaded the app, so now let's 
	}

	return WRX_OK;
}

// stop the state, so a later wrxRunning() will return 0
int wrxStop(wrxState *p) {
	if WRX_RUNS(p->mode) {
		p->mode = WRX_STOP;
		return WRX_OK;
	} else {
		return WRX_NOPE;
	}
}

// update the state
int wrxUpdate(wrxState *p) {
	// a touch of safety
	if (p == NULL) return WRX_NOPE;
	if (p->screen == NULL) return WRX_NOPE;
	// some value enforcement
	if (p->fpsTarget < 15.0f) p->fpsTarget = 15.0f;
	if (p->fpsTarget > 1000.0f) p->fpsTarget = 1000.0f;
	// basic updating
	p->clock = p->clock + tigrTime();
	p->dt = p->clock - p->drawClock;
	p->drawClock = p->clock + (1 / p->fpsTarget);
	// let's see if we have an open screen
	if (!tigrClosed(p->screen)) {
		tigrClear(p->screen, tigrRGB(0x0, 0x0, 0x0));
		tigrUpdate(p->screen);
	}
	// wait and sleep until we need to return
	while (p->clock < p->drawClock) {
    	usleep(p->sleepUMS);
        p->clock += tigrTime();
    }
    // make sure we correct if we overshot the drawClock for next update
    p->drawClock = p->clock;
	return WRX_OK;
}

