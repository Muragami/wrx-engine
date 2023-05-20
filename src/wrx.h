/*
	wrx-engine

	Jason A. Petrasko, muragami, muragami@wishray.com 2023

	MIT License
*/

#ifndef _WRX_ENGINE_HEADER
#define _WRX_ENGINE_HEADER
// ********************************************************

#include <unistd.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <physfs.h>
#include "tigr.h"

// ********************************************************
// some common values
#define WRX_LINE		256
#define WRX_STOP		0
#define WRX_NOPE		0
#define WRX_OK			1
#define WRX_ERR			-1
#define WRX_ERROR(x)	(x < 1)
#define WRX_RUNS(x)		(x > 0)
#define WRX_STOPPED(x)	(x == 0)
#define WRX_NOPPED(x)	(x == 0)

#define WRX_VERSION		"01.00.01"
#define WRX_NAME		"WRX_ENGINE"
#define WRX_CODENAME	"Hydrogen"

#define WRX_READ_FLAG	0xF0000000

// ********************************************************
// some common data structures
typedef struct {
	float clock;
	float dt;
	float fpsTarget;
	float drawClock;
	int mode;
	int width;
	int height;
	int sleepUMS;
	char title[WRX_LINE];
	char name[WRX_LINE];
	char error[WRX_LINE];
	Tigr *screen;
	lua_State* L;
	void *global;
} wrxState;

typedef struct {
	unsigned int bytes;
	unsigned int flags;
	unsigned int id;
	unsigned int end;
	void *memory;
} wrxData;

// ********************************************************
// functions shared across source code
wrxState *wrxNewState();
int wrxStart(wrxState *p, const char *app);
int wrxStop(wrxState *p);
int wrxUpdate(wrxState *p);
int wrxRunning(wrxState *p);
const char* wrxGetError(wrxState *p);

void lwrxRegister(lua_State *L);
int lwrxLoadString(wrxState *p, wrxData *src, const char *name);
void lwrxFieldToInteger(wrxState *p, int index, const char *name, int *v);
void lwrxFieldToFloat(wrxState *p, int index, const char *name, float *v);
void lwrxFieldToDouble(wrxState *p, int index, const char *name, double *v);
void lwrxFieldToString(wrxState *p, int index, const char *name, char *buffer, int bsize);

void *dwrxNewTable();
wrxData *dwrxReadFile(const char* fname);
void dwrxFreeData(wrxData *p);

// ********************************************************
#endif

