/*
	wrx-engine: data routines/functions

	Jason A. Petrasko, muragami, muragami@wishray.com 2023

	MIT License
	hash dictionary MIT License taken from here: https://github.com/LexLoki/dict
*/

#include "wrx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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

//  --------------------------------------------------------------------------
//  Reference implementation for rfc.zeromq.org/spec:32/Z85
//
//  This implementation provides a Z85 codec as an easy-to-reuse C class 
//  designed to be easy to port into other languages.

//  --------------------------------------------------------------------------
//  Copyright (c) 2010-2013 iMatix Corporation and Contributors
//  
//  Permission is hereby granted, free of charge, to any person obtaining a 
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation 
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//  and/or sell copies of the Software, and to permit persons to whom the 
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in 
//  all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
//  DEALINGS IN THE SOFTWARE.
//  --------------------------------------------------------------------------

// from: https://github.com/zeromq/rfc/blob/master/src/spec_32.c

//  Basic language taken from CZMQ's prelude
typedef unsigned char byte;
#define streq(s1,s2) (!strcmp ((s1), (s2)))

//  Maps base 256 to base 85
static char encoder[85 + 1] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";

//  Maps base 85 to base 256
//  We chop off lower 32 and higher 128 ranges
static byte decoder[96] = {
    0x00, 0x44, 0x00, 0x54, 0x53, 0x52, 0x48, 0x00, 0x4B, 0x4C, 0x46, 0x41, 0x00, 0x3F, 0x3E, 0x45, 
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x40, 0x00, 0x49, 0x42, 0x4A, 0x47, 
    0x51, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x4D, 0x00, 0x4E, 0x43, 0x00, 
    0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x4F, 0x00, 0x50, 0x00, 0x00
};

//  --------------------------------------------------------------------------
//  Encode a byte array as a string
char *Z85_encode(byte *data, size_t size) {
    //  Accepts only byte arrays bounded to 4 bytes
    if (size % 4)
        return NULL;
    
    size_t encoded_size = size * 5 / 4;
    char *encoded = malloc (encoded_size + 1);
    unsigned int char_nbr = 0;
    unsigned int byte_nbr = 0;
    uint32_t value = 0;
    while (byte_nbr < size) {
        //  Accumulate value in base 256 (binary)
        value = value * 256 + data [byte_nbr++];
        if (byte_nbr % 4 == 0) {
            //  Output value in base 85
            unsigned int divisor = 85 * 85 * 85 * 85;
            while (divisor) {
                encoded [char_nbr++] = encoder [value / divisor % 85];
                divisor /= 85;
            }
            value = 0;
        }
    }
    assert (char_nbr == encoded_size);
    encoded [char_nbr] = 0;
    return encoded;
}

    
//  --------------------------------------------------------------------------
//  Decode an encoded string into a byte array; size of array will be
//  strlen (string) * 4 / 5.
byte *Z85_decode(char *string) {
    //  Accepts only strings bounded to 5 bytes
    if (strlen (string) % 5)
        return NULL;
    
    size_t decoded_size = strlen (string) * 4 / 5;
    byte *decoded = malloc (decoded_size);

    unsigned int byte_nbr = 0;
    unsigned int char_nbr = 0;
    uint32_t value = 0;
    while (char_nbr < strlen (string)) {
        //  Accumulate value in base 85
        value = value * 85 + decoder [(byte) string [char_nbr++] - 32];
        if (char_nbr % 5 == 0) {
            //  Output value in base 256
            unsigned int divisor = 256 * 256 * 256;
            while (divisor) {
                decoded [byte_nbr++] = value / divisor % 256;
                divisor /= 256;
            }
            value = 0;
        }
    }
    assert (byte_nbr == decoded_size);
    return decoded;
}
