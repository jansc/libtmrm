/*
 * tmrm_internal.h - libtmrm private API
 * http://libtmrm.ravn.no
 *
 * This file is licensed under the 
 * GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Copyright (C) 2008-2009 Jan Schreiber, http://purl.org/net/jans
 * Copyright (C) 2008-2009 Ravn Webveveriet AS, NO http://www.ravn.no
 *
 * Parts of the code are based on the Redland library, http://librdf.org
 *
 * Copyright (C) 2000-2007 David Beckett http://purl.org/net/dajobe/
 * Copyright (C) 2000-2005 University of Bristol, UK http://www.bristol.ac.uk/
 */ 

#ifndef TMRM_INTERNAL_H
#define TMRM_INTERNAL_H

/**
 * @file tmrm_internal.h
 * @brief libtmrm private API, common macros.
 * Contains some common macros for debugging, loggin, memory allocation,
 * and private helper functions.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libtmrm_private libtmrm private routines
 * @{
 * @}
 */
#include <stdio.h>

#include <assert.h>

/**
 * Internal macros
 * @defgroup tmrm_macros Macros
 * @ingroup libtmrm_private 
 * @{
 */
#if defined(__cplusplus) 
#  include <climits> 
   const int INT_DIGITS = (sizeof(int) * CHAR_BIT + 2) / 3 + 2; 
#else 
#  include <limits.h> 
#  define INT_DIGITS (sizeof(int) * CHAR_BIT + 2) / 3 + 2 
#endif 

#define TMRM_API

/* Debugging messages */
#define TMRM_DEBUG1(msg) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__); } while(0)
#define TMRM_DEBUG2(msg, arg1) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1);} while(0)
#define TMRM_DEBUG3(msg, arg1, arg2) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2);} while(0)
#define TMRM_DEBUG4(msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2, arg3);} while(0)
#define TMRM_DEBUG5(msg, arg1, arg2, arg3, arg4) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2, arg3, arg4);} while(0)

#define TMRM_ASSERT assert

/* #define TMRM_ASSERT_DIE abort(); */
#define TMRM_ASSERT_DIE
#define TMRM_ASSERT_REPORT(msg) fprintf(stderr, "%s:%d: (%s) assertion failed: " msg "\n", __FILE__, __LINE__, __func__);
#define TMRM_ASSERT_OBJECT_POINTER_RETURN(pointer, type) do { \
  if(!pointer) { \
    TMRM_ASSERT_REPORT("object pointer of type " #type " is NULL.") \
    TMRM_ASSERT_DIE \
    return; \
  } \
} while(0)

#define TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(pointer, type, ret) do { \
  if(!pointer) { \
    TMRM_ASSERT_REPORT("object pointer of type " #type " is NULL.") \
    TMRM_ASSERT_DIE \
    return ret; \
  } \
} while(0)


#define TMRM_MALLOC(type, size) malloc(size)
#define TMRM_CALLOC(type, size, count) calloc(size, count)
#define TMRM_FREE(type, ptr) free(ptr)

/* Constants and sizes */
#define TMRM_MAXLENGTH_MAP_NAME 255   // Max size of name of map

#define TMRM_XMLSCHEMA_STRING "http://www.w3.org/2001/XMLSchema#string"

#define TMRM_BOTTOM_PROXY_LABEL "libtmrm_bottom"
    
#include <tmrm_list.h>
#include <tmrm_hash_internal.h>
    
/* Internal representation of proxies: we storage proxy_id as a number. */
typedef int tmrm_label;

/**
 * @}
 * Internal data structures.
 * @defgroup tmrm_struct Internal data structures
 * @ingroup libtmrm_private 
 * @{
 */

struct tmrm_subject_map_sphere_s {
    /** List of storage factories */
    tmrm_list* factories;

    /** The hash factory (currently one a memory implementation).
     TODO: Should be a list of hash factories. */
    tmrm_hash_factory* hashes;

    /* list of free librdf_hash_datums is kept */
    tmrm_hash_datum* hash_datums_list;
    /* hash load_factor out of 1000 */
    int hash_load_factor;

    tmrm_error_type_t errno;
    char *err;
};


/* => move to tmrm_subject_map_internal.h */
struct tmrm_subject_map_s {
    tmrm_subject_map_sphere* sms;
    tmrm_storage* storage;
    char *label;
    /* Keep the most important proxies from the bootstrap ontology in memory.
     */
    tmrm_proxy *bottom;
    tmrm_proxy *superclass;
    tmrm_proxy *subclass;
    tmrm_proxy *type;
    tmrm_proxy *instance;
};


/* => move to tmrm_proxy_internal.h */
struct tmrm_proxy_s {
    tmrm_object type;
    tmrm_subject_map* subject_map;
    tmrm_label label; /* It's up to the storage module to represent a label.
    Possible TODO: Allow different label representations (e.g. char*) for
    other backends. Use a union? 
    union {
        int int_val;
        long long_val;
        tmrm_char_t *string_val;
        void *void_val;
    } label;
    */
};

/* => move to tmrm_literal_internal.h */
struct tmrm_literal_s {
    tmrm_object type;
    tmrm_char_t* value;
    tmrm_char_t* datatype; /* URI: TODO should we use a separate class?
                       => TMDM uses URIs a lot */
};

/** 
 * A set that allows duplicate members.
 * @todo The implementation uses a single-linked list, but a different
 *       data structure might be needed in the future. The C++ STL uses
 *       red-black trees.
 */
struct tmrm_multiset_s {
    tmrm_object type;
    tmrm_subject_map* subject_map;
    tmrm_list* list; 
};

/* => move to tmrm_iterator_internal.h */
struct tmrm_iterator_s {
    tmrm_object type;
    tmrm_subject_map_sphere* sms;
    void* context;
    int (*next_method)(void*);
    int (*end_method)(void*);
    tmrm_object* (*get_element_method)(void*, tmrm_iterator_flag);
    void (*free_method)(void*);
};

/* => move to tmrm_tuple_internal.h */
struct tmrm_tuple_s {
    tmrm_object type;
    size_t size;
    tmrm_object **values;
    void (*free_method)(tmrm_object*);
};



/**
 * @}
 * Internal functions
 * @defgroup tmrm_functions Internal functions
 * @ingroup libtmrm_private 
 * @{
 */

static int
tmrm_subject_map_sphere_set_storage_error(tmrm_subject_map_sphere *sms,
tmrm_storage *storage, const char *problem);


void tmrm_init_storage(tmrm_subject_map_sphere *sms);

/**
 * Generates an anonymous (and unique) label for a proxy.
 */
char* tmrm_proxy_generate_label();

/** @} */

#ifdef __cplusplus
}
#endif

#endif
