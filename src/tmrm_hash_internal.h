/*
 * tmrm_hash_internal.h - (Temporary) implementation of private hash functions.
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

#ifndef TMRM_HASH_INTERNAL_H
#define TMRM_HASH_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/** data type used to describe hash key and data */
struct tmrm_hash_datum_s
{
    tmrm_subject_map_sphere *sms;
    void *data;
    size_t size;
    /* used internally to build lists of these  */
    struct tmrm_hash_datum_s *next;
};
typedef struct tmrm_hash_datum_s tmrm_hash_datum;

/* constructor / destructor for above */
tmrm_hash_datum* tmrm_hash_datum_new(tmrm_subject_map_sphere *sms, void *data, size_t size);
void tmrm_hash_datum_free(tmrm_hash_datum *ptr);


/** A hash object */
struct tmrm_hash_s
{
    tmrm_subject_map_sphere* sms;
    char* identifier; /* as passed in during open(), used by clone() */
    void* context;
    int   is_open;
    struct tmrm_hash_factory_s* factory;
};


/** A Hash Factory */
struct tmrm_hash_factory_s {
    struct tmrm_hash_factory_s* next;
    char* name;

    /* the rest of this structure is populated by the
       hash-specific register function */
    size_t context_length;

    /* size of the cursor context */
    size_t cursor_context_length;

    /* clone an existing storage */
    int (*clone)(tmrm_hash* new_hash, void* new_context, char* new_name, void* old_context);

    /* create / destroy a hash implementation */
    int (*create)(tmrm_hash* hash, void* context);
    int (*destroy)(void* context);

    /* open/create hash with identifier and options  */
    int (*open)(void* context, const char *identifier, int mode, int is_writable, int is_new, tmrm_hash* options);
    /* end hash association */
    int (*close)(void* context);

    /* how many values? */
    int (*values_count)(void* context);

    /* insert key/value pairs according to flags */
    int (*put)(void* context, tmrm_hash_datum *key, tmrm_hash_datum *data);

    /* returns true if key exists in hash, without returning value */
    int (*exists)(void* context, tmrm_hash_datum *key, tmrm_hash_datum *value);

    int (*delete_key)(void* context, tmrm_hash_datum *key);
    int (*delete_key_value)(void* context, tmrm_hash_datum *key, tmrm_hash_datum *value);

    /* flush any cached information to disk */
    int (*sync)(void* context);

    /* get the file descriptor for the hash, if it is file based (for locking) */
    int (*get_fd)(void* context);

    /* create a cursor and operate on it */
    int (*cursor_init)(void *cursor_context, void* hash_context);
    int (*cursor_get)(void *cursor, tmrm_hash_datum *key, tmrm_hash_datum *value, unsigned int flags);
    void (*cursor_finish)(void *context);
};
typedef struct tmrm_hash_factory_s tmrm_hash_factory;

/* factory class methods */
void tmrm_hash_register_factory(tmrm_subject_map_sphere *sms, const char *name, void (*factory) (tmrm_hash_factory*));
tmrm_hash_factory* tmrm_get_hash_factory(tmrm_subject_map_sphere *sms, const char *name);


/* module init */
void tmrm_init_hash(tmrm_subject_map_sphere *sms);

/* module terminate */
void tmrm_finish_hash(tmrm_subject_map_sphere *sms);

/* hash cursor_get method flags */
#define TMRM_HASH_CURSOR_SET 0
#define TMRM_HASH_CURSOR_NEXT_VALUE 1
#define TMRM_HASH_CURSOR_FIRST 2
#define TMRM_HASH_CURSOR_NEXT 3


/* constructors */
tmrm_hash* tmrm_hash_new_from_factory(tmrm_subject_map_sphere *sms, tmrm_hash_factory* factory);

/* methods */

/* open/create hash with identifier and options  */
int tmrm_hash_open(tmrm_hash* hash, const char *identifier, int mode, int is_writable, int is_new, tmrm_hash* options);
/* end hash association */
int tmrm_hash_close(tmrm_hash* hash);

/* retrieve one value for a given hash key as a hash datum */
tmrm_hash_datum* tmrm_hash_get_one(tmrm_hash* hash, tmrm_hash_datum *key);

/* retrieve all values for a given hash key according to flags */
tmrm_iterator* tmrm_hash_get_all(tmrm_hash* hash, tmrm_hash_datum *key, tmrm_hash_datum *value);

/* insert a key/value pair */
int tmrm_hash_put(tmrm_hash* hash, tmrm_hash_datum *key, tmrm_hash_datum *value);

  /* returns true if key exists in hash, without returning value */
int tmrm_hash_exists(tmrm_hash* hash, tmrm_hash_datum *key, tmrm_hash_datum *value);

int tmrm_hash_delete(tmrm_hash* hash, tmrm_hash_datum *key, tmrm_hash_datum *value);
int tmrm_hash_delete_all(tmrm_hash* hash, tmrm_hash_datum *key);
tmrm_iterator* tmrm_hash_keys(tmrm_hash* hash, tmrm_hash_datum *key);

/* flush any cached information to disk */
int tmrm_hash_sync(tmrm_hash* hash);
/* get the file descriptor for the hash, if it is file based (for locking) */
int tmrm_hash_get_fd(tmrm_hash* hash);

/* init a hash from a string representation */
int tmrm_hash_from_string(tmrm_hash* hash, const char *string);

/* init a hash from an array of strings */
int tmrm_hash_from_array_of_strings(tmrm_hash* hash, const char *array[]);


/* cursor methods from tmrm_hash_cursor.c */

tmrm_hash_cursor* tmrm_hash_cursor_new(tmrm_hash* hash);
void tmrm_hash_cursor_free(tmrm_hash_cursor* cursor);
int tmrm_hash_cursor_set(tmrm_hash_cursor *cursor, tmrm_hash_datum *key,tmrm_hash_datum *value);
int tmrm_hash_cursor_get_next_value(tmrm_hash_cursor *cursor, tmrm_hash_datum *key,tmrm_hash_datum *value);
int tmrm_hash_cursor_get_first(tmrm_hash_cursor *cursor, tmrm_hash_datum *key, tmrm_hash_datum *value);
int tmrm_hash_cursor_get_next(tmrm_hash_cursor *cursor, tmrm_hash_datum *key, tmrm_hash_datum *value);

#ifdef HAVE_BDB_HASH
void tmrm_init_hash_bdb(tmrm_subject_map_sphere *sms);
#endif
void tmrm_init_hash_memory(tmrm_subject_map_sphere *sms);



#ifdef __cplusplus
}
#endif

#endif


