/*
	wrx-engine: data routines/functions

	Jason A. Petrasko, muragami, muragami@wishray.com 2023

	MIT License
	hash dictionary MIT License taken from here: https://github.com/LexLoki/dict
*/

#include "wrx.h"
#include <stdlib.h>

#define HASHSIZE 101
#define K 31

typedef struct node {
	char *key;
	void *value;
	struct node *next;
} Node;

typedef struct dict {
	Node *table[HASHSIZE];
	int valueSize;
} Dict;

static inline unsigned int _hash(char *s) {
    unsigned int hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

Node *node_init(char *key, void *value, int valueSize) {
	Node *nd = (Node*)malloc(sizeof(Node));
	if (nd == NULL) exit(-1);
	nd->key = (char *)malloc((strlen(key)+1) * sizeof(char));
	if (nd->key == NULL) exit(-1);
	nd->value = malloc(valueSize);
	if(nd->value == NULL) exit(-1);
	memcpy(nd->value, value, valueSize);
	strcpy(nd->key, key);
	nd->next = NULL;
	return nd;
}

void node_free(Node *nd) {
	free(nd->key);
	free(nd->value);
	free(nd);
}

Dict *dict_init(int valueSize) {
	int i;
	Dict *d = (Dict*)malloc(sizeof(Dict));
	if(d == NULL) exit(-1);
	for(i = 0; i < HASHSIZE; d->table[i++] = NULL);
	d->valueSize = valueSize;
	return d;
}

void dict_set(Dict *d, char *key, void *value) {
	unsigned idx = _hash(key);
	Node *nd=d->table[idx];
	if (nd==NULL) {
		d->table[idx]=node_init(key, value, d->valueSize);
	} else {
		for (; strcmp(nd->key, key); nd = nd->next)
			if (nd->next == NULL) {
				nd->next=node_init(key, value, d->valueSize);
				return;
			}
		memcpy(nd->value, value, d->valueSize);
	}
}

void *dict_getValue(Dict *d, char *key) {
	Node *nd;
	for(nd=d->table[_hash(key)];nd!=NULL;nd=nd->next)
		if(!strcmp(nd->key,key)) return nd->value;
	return NULL;
}

void dict_remove(Dict *d, char *key) {
	unsigned idx = _hash(key);
	Node *nd=d->table[idx], *aux=NULL;
	for(; nd != NULL; aux = nd, nd = nd->next)
		if(!strcmp(nd->key, key)){
			(aux == NULL) ? (d->table[idx] = nd->next) : (aux->next = nd->next);
			node_free(nd);
			return;
		}
}

void dict_free(Dict *d) {
	Node *aux, *nd;
	int i;
	for(i=0; i < HASHSIZE; i++)
		for(nd = d->table[i]; nd != NULL; aux = nd, nd = nd->next, node_free(aux));
	free(d);
}

void *dwrxNewTable() {
	Dict *d = dict_init(sizeof(wrxData*));
	return d;
}

wrxData *dwrxReadFile(const char* fname) {
	wrxData *ret;
	PHYSFS_File *fp;
	unsigned long int len, rd;

	fp = PHYSFS_openRead(fname);
	if (fp == NULL) return NULL;

	ret = malloc(sizeof(wrxData));
	len = PHYSFS_fileLength(fp);
	ret->bytes = len + 1;
	ret->flags = 0;
	ret->memory = malloc(ret->bytes);

	rd = PHYSFS_readBytes(fp, ret->memory, ret->bytes);
	PHYSFS_close(fp);

	// null terminate in case we need that
	((char*)ret->memory)[rd] = 0;
	// put the actual length into end and not allocated byteas
	ret->end = rd;
	ret->id = 0;

	return ret;
}

void dwrxFreeData(wrxData *p) {
	free(p->memory);
	free(p);
}
