/*
	wrx-engine: data routines/functions

	Jason A. Petrasko, muragami, muragami@wishray.com 2023

	MIT License
*/

#include "wrx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

#include "../include/CImg.h"

// *****************************************************************************
// hash table implementation from: https://github.com/nomemory/open-adressing-hash-table-c
#define OA_HASH_LOAD_FACTOR 		(0.75)			// 75%
#define OA_HASH_GROWTH_FACTOR 		(1 << 2)		// x4
#define OA_HASH_INIT_CAPACITY 		(1 << 8)		// 256

typedef struct oa_key_ops_s {
    uint32_t (*hash)(const void *data, void *arg);
    void* (*cp)(const void *data, void *arg);
    void (*free)(void *data, void *arg);
    bool (*eq)(const void *data1, const void *data2, void *arg);
    void *arg;
} oa_key_ops;

typedef struct oa_val_ops_s {
    void* (*cp)(const void *data, void *arg);
    void (*free)(void *data, void *arg);
    void *arg;
} oa_val_ops;

typedef struct oa_pair_s {
    uint32_t hash;
    void *key;
    void *val;
} oa_pair;

typedef struct oa_hash_s {
    size_t capacity;
    size_t size;
    oa_pair **buckets;
    void (*probing_fct)(struct oa_hash_s *htable, size_t *from_idx);
    oa_key_ops key_ops;
    oa_val_ops val_ops;
} oa_hash;

oa_hash* oa_hash_new(oa_key_ops key_ops, oa_val_ops val_ops, void (*probing_fct)(struct oa_hash_s *htable, size_t *from_idx));
oa_hash* oa_hash_new_lp(oa_key_ops key_ops, oa_val_ops val_ops);
void oa_hash_free(oa_hash *htable);
void oa_hash_put(oa_hash *htable, const void *key, const void *val);
void *oa_hash_get(oa_hash *htable, const void *key);
void oa_hash_delete(oa_hash *htable, const void *key);
void oa_hash_print(oa_hash *htable, void (*print_key)(const void *k), void (*print_val)(const void *v));

// Pair related
oa_pair *oa_pair_new(uint32_t hash, const void *key, const void *val, void *arg);

// String operations
uint32_t oa_string_hash(const void *data, void *arg);
void* oa_string_cp(const void *data, void *arg);
bool oa_string_eq(const void *data1, const void *data2, void *arg);
void oa_string_free(void *data, void *arg);
void oa_string_print(const void *data);

enum oa_ret_ops { REMOVE, PUT, GET };

static size_t oa_hash_getidx(oa_hash *htable, size_t idx, uint32_t hash_val, const void *key, enum oa_ret_ops op);
static inline void oa_hash_lp_idx(oa_hash *htable, size_t *idx);
inline static void oa_hash_grow(oa_hash *htable);
static inline bool oa_hash_should_grow(oa_hash *htable);
static inline bool oa_hash_is_tombstone(oa_hash *htable, size_t idx);
static inline void oa_hash_put_tombstone(oa_hash *htable, size_t idx);

oa_hash* oa_hash_new(
    oa_key_ops key_ops, 
    oa_val_ops val_ops, 
    void (*probing_fct)(struct oa_hash_s *htable, size_t *from_idx)) 
{
    oa_hash *htable;
    
    htable = (oa_hash*)malloc(sizeof(*htable));
    if (NULL==htable) {
        fprintf(stderr,"malloc() failed in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);  
    }

    htable->size = 0;
    htable->capacity = OA_HASH_INIT_CAPACITY;
    htable->val_ops = val_ops;
    htable->key_ops = key_ops;
    htable->probing_fct = probing_fct;

    htable->buckets = (oa_pair **)malloc(sizeof(*(htable->buckets)) * htable->capacity);
    if (NULL==htable->buckets) {
        fprintf(stderr,"malloc() failed in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);  
    }

    for(int i = 0; i < htable->capacity; i++) {
        htable->buckets[i] = NULL;
    }

    return htable;
}

oa_hash* oa_hash_new_lp(oa_key_ops key_ops, oa_val_ops val_ops) {
    return oa_hash_new(key_ops, val_ops, oa_hash_lp_idx);
}

void oa_hash_free(oa_hash *htable) {
    for(int i = 0; i < htable->capacity; i++) {
        if (NULL!=htable->buckets[i]) {
            htable->key_ops.free(htable->buckets[i]->key, htable->key_ops.arg);
            htable->val_ops.free(htable->buckets[i]->val, htable->val_ops.arg);
        }
        free(htable->buckets[i]);
    }
    free(htable->buckets);
    free(htable);
}

inline static void oa_hash_grow(oa_hash *htable) {
    uint32_t old_capacity;
    oa_pair **old_buckets;
    oa_pair *crt_pair;

    uint64_t new_capacity_64 = (uint64_t) htable->capacity * OA_HASH_GROWTH_FACTOR;
    if (new_capacity_64 > SIZE_MAX) {
        fprintf(stderr, "re-size overflow in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);
    }

    old_capacity = htable->capacity;
    old_buckets = htable->buckets;

    htable->capacity = (uint32_t) new_capacity_64;
    htable->size = 0;
    htable->buckets = (oa_pair **)malloc(htable->capacity * sizeof(*(htable->buckets)));

    if (NULL == htable->buckets) {
        fprintf(stderr,"malloc() failed in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);  
    }

    for(int i = 0; i < htable->capacity; i++) {
        htable->buckets[i] = NULL;
    };

    for(size_t i = 0; i < old_capacity; i++) {
        crt_pair = old_buckets[i];
        if (NULL!=crt_pair && !oa_hash_is_tombstone(htable, i)) {
            oa_hash_put(htable, crt_pair->key, crt_pair->val);
            htable->key_ops.free(crt_pair->key, htable->key_ops.arg);
            htable->val_ops.free(crt_pair->val, htable->val_ops.arg);
            free(crt_pair);
        }
    }

    free(old_buckets);
}

inline static bool oa_hash_should_grow(oa_hash *htable) {
    return (htable->size / htable->capacity) > OA_HASH_LOAD_FACTOR;
}

void oa_hash_put(oa_hash *htable, const void *key, const void *val) {

    if (oa_hash_should_grow(htable)) {
        oa_hash_grow(htable);
    }

    uint32_t hash_val = htable->key_ops.hash(key, htable->key_ops.arg);
    size_t idx = hash_val % htable->capacity;

    if (NULL==htable->buckets[idx]) {
        // Key doesn't exist & we add it anew
        htable->buckets[idx] = oa_pair_new(
                hash_val, 
                htable->key_ops.cp(key, htable->key_ops.arg),
                htable->val_ops.cp(val, htable->val_ops.arg),
                htable->key_ops.arg
        );
    } else {
        // // Probing for the next good index
        idx = oa_hash_getidx(htable, idx, hash_val, key, PUT);

        if (NULL==htable->buckets[idx]) {
            htable->buckets[idx] = oa_pair_new(
                hash_val, 
                htable->key_ops.cp(key, htable->key_ops.arg),
                htable->val_ops.cp(val, htable->val_ops.arg),
                htable->key_ops.arg
            );
        } else {
            // Update the existing value
            // Free the old values
            htable->val_ops.free(htable->buckets[idx]->val, htable->val_ops.arg);
            htable->key_ops.free(htable->buckets[idx]->key, htable->key_ops.arg);
            // Update the new values
            htable->buckets[idx]->val = htable->val_ops.cp(val, htable->val_ops.arg);
            htable->buckets[idx]->key = htable->val_ops.cp(key, htable->key_ops.arg);
            htable->buckets[idx]->hash = hash_val;
        }
   }
    htable->size++;
}

inline static bool oa_hash_is_tombstone(oa_hash *htable, size_t idx) {
    if (NULL==htable->buckets[idx]) {
        return false;
    }
    if (NULL==htable->buckets[idx]->key && 
        NULL==htable->buckets[idx]->val && 
        0 == htable->buckets[idx]->key) {
            return true;
    }        
    return false;
}

inline static void oa_hash_put_tombstone(oa_hash *htable, size_t idx) {
    if (NULL != htable->buckets[idx]) {
        htable->buckets[idx]->hash = 0;
        htable->buckets[idx]->key = NULL;
        htable->buckets[idx]->val = NULL;
    }
}

void *oa_hash_get(oa_hash *htable, const void *key) {
    uint32_t hash_val = htable->key_ops.hash(key, htable->key_ops.arg);
    size_t idx = hash_val % htable->capacity;

    if (NULL==htable->buckets[idx]) {
        return NULL;
    }

    idx = oa_hash_getidx(htable, idx, hash_val, key, GET);

    return (NULL==htable->buckets[idx]) ?
         NULL : htable->buckets[idx]->val;
}

void oa_hash_delete(oa_hash *htable, const void *key) {
    uint32_t hash_val = htable->key_ops.hash(key, htable->key_ops.arg);
    size_t idx = hash_val % htable->capacity;
    
    if (NULL==htable->buckets[idx]) {
        return;
    }

    idx = oa_hash_getidx(htable, idx, hash_val, key, REMOVE);
    if (NULL==htable->buckets[idx]) {
        return;
    }

    htable->val_ops.free(htable->buckets[idx]->val, htable->val_ops.arg);
    htable->key_ops.free(htable->buckets[idx]->key, htable->key_ops.arg);

    oa_hash_put_tombstone(htable, idx);
}

void oa_hash_print(oa_hash *htable, void (*print_key)(const void *k), void (*print_val)(const void *v)) {
    oa_pair *pair;

    printf("Hash Capacity: %llu\n", htable->capacity);
    printf("Hash Size: %llu\n", htable->size);

    printf("Hash Buckets:\n");
    for(int i = 0; i < htable->capacity; i++) {
        pair = htable->buckets[i];
        printf("\tbucket[%d]:\n", i);
        if (NULL!=pair) {
            if (oa_hash_is_tombstone(htable, i)) {
                printf("\t\t TOMBSTONE");
            } else {
                printf("\t\thash=%08X, key=", pair->hash);
                print_key(pair->key);
                printf(", value=");
                print_val(pair->val);
            }
        }
        printf("\n");
    }
}
static size_t oa_hash_getidx(oa_hash *htable, size_t idx, uint32_t hash_val, const void *key, enum oa_ret_ops op) {
    do {
        if (op==PUT && oa_hash_is_tombstone(htable, idx)) {
            break;
        }
        if (htable->buckets[idx]->hash == hash_val && 
            htable->key_ops.eq(key, htable->buckets[idx]->key, htable->key_ops.arg)) {
            break;
        }
        htable->probing_fct(htable, &idx);
    } while(NULL!=htable->buckets[idx]);
    return idx;
}

// Probing functions

static inline void oa_hash_lp_idx(oa_hash *htable, size_t *idx) {
    (*idx)++;
    if ((*idx)==htable->capacity) {
        (*idx) = 0;
    }
}

// Pair related

oa_pair *oa_pair_new(uint32_t hash, const void *key, const void *val, void *arg) {
    oa_pair *p;
    p = (oa_pair*)malloc(sizeof(*p));
    if (NULL==p) {
        wrxError((wrxState*)arg, "malloc() failed in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);  
    }
    p->hash = hash;
    p->val = (void*) val;
    p->key = (void*) key;
    return p;
}

// String operations

static uint32_t oa_hash_fmix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x3243f6a9U;
    h ^= h >> 16;
    return h;
}

uint32_t oa_string_hash(const void *data, void *arg) {
    
    //djb2
    uint32_t hash = (const uint32_t) 5381;
    const char *str = (const char*) data;
    char c;
    while((c=*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return oa_hash_fmix32(hash);
}


void* oa_string_cp(const void *data, void *arg) {
    const char *input = (const char*) data;
    size_t input_length = strlen(input) + 1;
    char *result;
    result = (char*)malloc(sizeof(*result) * input_length);
    if (NULL==result) {
        wrxError((wrxState*)arg, "malloc() failed in file %s at line # %d", __FILE__,__LINE__);
        return NULL;
    }
    strcpy(result, input);
    return result;
}

bool oa_string_eq(const void *data1, const void *data2, void *arg) {
    const char *str1 = (const char*) data1;
    const char *str2 = (const char*) data2;
    return !(strcmp(str1, str2)) ? true : false;    
}

void oa_string_free(void *data, void *arg) {
    free(data);
}

void oa_string_print(const void *data) {    
    printf("%s", (const char*) data);
}

void* oa_data_cp(const void *data, void *arg) {
	const wrxInfo *p = (wrxInfo*)data;
	wrxInfo *ret = (wrxInfo*)malloc(sizeof(wrxInfo));
	if (ret == NULL) {
		wrxError((wrxState*)arg, "malloc() failed in file %s at line # %d", __FILE__, __LINE__);
		return NULL;
	}
	memcpy(ret, p, sizeof(wrxInfo));
    switch (p->form) {
        case WRX_FORM_DATA:
                ret->data.memory = malloc(p->data.bytes);
                if (ret->data.memory == NULL) {
                    wrxError((wrxState*)arg, "malloc() failed in file %s at line # %d", __FILE__, __LINE__);
                    return NULL;
                }
                memcpy(ret->data.memory, p->data.memory, p->data.bytes);
            break;
        case WRX_FORM_MEMIO:
                ret->io.mem = (char*)malloc(p->io.length);
                if (ret->io.mem == NULL) {
                    wrxError((wrxState*)arg, "malloc() failed in file %s at line # %d", __FILE__, __LINE__);
                    return NULL;
                }
                memcpy(ret->io.mem, p->io.mem, p->io.length);
            break;
        case WRX_FORM_TABLE:
            // TODO
            break;
    }
	
    return ret;
}

void oa_data_free(void *data, void *arg) {
	wrxInfo *p = (wrxInfo*)data;
    dwrxFreeInfo(p);
}


oa_key_ops oa_key_ops_string = { oa_string_hash, oa_string_cp, oa_string_free, oa_string_eq, NULL};
oa_val_ops oa_val_ops_data = { oa_data_cp, oa_data_free, NULL};


// *****************************************************************************

void *dwrxNewTable(wrxState *p) {
	oa_hash *ret = oa_hash_new(oa_key_ops_string, oa_val_ops_data, oa_hash_lp_idx);
	ret->val_ops.arg = ret->key_ops.arg = p;
	return ret;
}

void *dwrxNewTree(wrxState *p, int id_bits) {
	wrxIdTree *ret = (wrxIdTree*)calloc(sizeof(wrxIdTree), 1);
	ret->idBits = id_bits;
	return ret;
}

void dwrxFreeTree(wrxIdTree *tree) {

}

wrxInfo *dwrxReadFile(const char* fname) {
	wrxInfo *ret;
	PHYSFS_File *fp;
	unsigned long int len, rd;

	fp = PHYSFS_openRead(fname);
	if (fp == NULL) return NULL;

	ret = (wrxInfo*)calloc(1, sizeof(wrxInfo));
	len = PHYSFS_fileLength(fp);
	ret->data.bytes = len + 1;
	ret->data.flags = WRX_DATA_BINARY;
	ret->data.memory = malloc(ret->data.bytes);

	rd = PHYSFS_readBytes(fp, ret->data.memory, ret->data.bytes);
	PHYSFS_close(fp);

	// null terminate in case we need that
	((char*)ret->data.memory)[rd] = 0;
	// put the actual length into end and not allocated byteas
	ret->data.count = rd;
	ret->data.id = 0;

    ret->form = WRX_FORM_DATA;
	return ret;
}

void dwrxFreeInfo(wrxInfo *p) {
    switch (p->form) {
        case WRX_FORM_DATA:
                free(p->data.memory);
            break;
        case WRX_FORM_MEMIO:
                free(p->io.mem);
            break;
        case WRX_FORM_TABLE:
            // TODO
            break;
    }
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
    char *encoded = (char*)malloc(encoded_size + 1);
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
    byte *decoded = (byte*)malloc(decoded_size);

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
