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
#include "xthread.h"

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

#define WRX_ID_BITS_16	16
#define WRX_ID_BITS_24	24

#define WRX_READ_FLAG	0xF0000000

// types of data the system might store in a table
#define WRX_DATA_BINARY		0x0		// unknown binary data
#define WRX_DATA_UTF8		0x1		// UTF8 text, may contain 0, so not really a string
#define WRX_DATA_LUA5		0x2		// Lua 5.4 code UTF8 text
#define WRX_DATA_TABLE		0x3		// a wrx table (containing data)
#define WRX_DATA_STATE		0x4		// a wrx table (containing data)
// some flags for data objects
#define WRX_DATA_NODE		0x10		// this data is a node, and has data following it
#define WRX_DATA_ARRAY		0x20		// this data is a node, and has data following it
// invalid id and linkage
#define WRX_DATA_INVALID 	0xFFFFFFFF	// this data is invalid

// ********************************************************
// some common data structures
typedef struct {
	unsigned int bytes;
	unsigned int flags;
	unsigned int id;
	unsigned int count;
	unsigned int lock;
	unsigned int next;
	unsigned int last;
	void *memory;
} wrxData;

typedef struct {
	wrxData* data;
} wrxIdTreeNode;

typedef struct {
	void* child[256];
} wrxIdTreeLevel;

typedef struct {
	wrxIdTreeLevel* root;
	unsigned int idBits;
} wrxIdTree;

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
	Tigr* screen;
	lua_State* L;
	unsigned int idBits;
	unsigned int nextId;
	void* gTable;
	wrxIdTree* gTree[256];
	void *audio;
} wrxState;


// ********************************************************
// functions shared across source code
wrxState *wrxNewState();
int wrxStart(wrxState *p, const char *app);
int wrxStop(wrxState *p);
int wrxUpdate(wrxState *p);
int wrxRunning(wrxState *p);
const char* wrxGetError(wrxState *p);
int wrxError(wrxState *p, const char *fmt, ...);

void lwrxRegister(lua_State *L);
int lwrxLoadString(wrxState *p, wrxData *src, const char *name);
void lwrxFieldToInteger(wrxState *p, int index, const char *name, int *v);
void lwrxFieldToFloat(wrxState *p, int index, const char *name, float *v);
void lwrxFieldToDouble(wrxState *p, int index, const char *name, double *v);
void lwrxFieldToString(wrxState *p, int index, const char *name, char *buffer, int bsize);

void *dwrxNewTable(wrxState *p);
void *dwrxNewTree(wrxState *p, int id_bits);
void dwrxFreeTree(wrxIdTree *tree);
wrxData *dwrxReadFile(const char* fname);
void dwrxFreeData(wrxData *p);

// ********************************************************
#endif

