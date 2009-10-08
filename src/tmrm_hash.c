/*
 * tmrm_hash.c - (Temporary) implementation of hash functions.
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

/* This file is taken from David Beckett's librdf with only minor modifications
 */

#ifdef HAVE_CONFIG_H
#include <libtmrm_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for strtol */
#endif

#include <libtmrm.h>

#include <tmrm_internal.h>
#include <tmrm_hash.h>
#include <tmrm_hash_internal.h>

#ifndef STANDALONE

/* prototypes for helper functions */
static void tmrm_delete_hash_factories(tmrm_subject_map_sphere *sms);

static void tmrm_init_hash_datums(tmrm_subject_map_sphere *sms);
static void tmrm_hash_datums_free(tmrm_subject_map_sphere *sms);


/* prototypes for iterator for getting all keys and values */
static int tmrm_hash_get_all_iterator_is_end(void* iterator);
static int tmrm_hash_get_all_iterator_next_method(void* iterator);
static void* tmrm_hash_get_all_iterator_get_method(void* iterator, tmrm_iterator_flag flags);
static void tmrm_hash_get_all_iterator_finished(void* iterator);

/* prototypes for iterator for getting all keys */
static int tmrm_hash_keys_iterator_is_end(void* iterator);
static int tmrm_hash_keys_iterator_next_method(void* iterator);
static tmrm_object* tmrm_hash_keys_iterator_get_method(void* iterator, tmrm_iterator_flag flags);
static void tmrm_hash_keys_iterator_finished(void* iterator);




/**
 * Initialises and registers all compiled hash modules.  Must be called
 * before using any of the hash factory functions such as 
 * tmrm_get_hash_factory()
 *
 * INTERNAL - Initialise the hash module.
 *
 * @param sms subject map sphere object
 */
void
tmrm_init_hash(tmrm_subject_map_sphere *sms) 
{
    /* Init hash datum cache */
    tmrm_init_hash_datums(sms);
/*
#ifdef HAVE_BDB_HASH
     FIXME not implemented
    tmrm_init_hash_bdb(sms);
#endif
*/
    /* Always have hash in memory implementation available */
    tmrm_init_hash_memory(sms);
}


/**
 * Terminate the hash module.
 *
 * @param sms subject map system object
 */
void
tmrm_finish_hash(tmrm_subject_map_sphere *sms) 
{
    tmrm_delete_hash_factories(sms);
    tmrm_hash_datums_free(sms);
}



/* helper functions */
static void
tmrm_delete_hash_factories(tmrm_subject_map_sphere *sms)
{
    tmrm_hash_factory *factory, *next;

    for(factory = sms->hashes; factory; factory = next) {
        next = factory->next;
        TMRM_FREE(tmrm_hash_factory, factory->name);
        TMRM_FREE(tmrm_hash_factory, factory);
    }
    sms->hashes = NULL;
}



/* hash datums structures */
static void
tmrm_init_hash_datums(tmrm_subject_map_sphere *sms)
{
    sms->hash_datums_list = NULL;
}


static void
tmrm_hash_datums_free(tmrm_subject_map_sphere *sms)
{
    tmrm_hash_datum *datum, *next;

    for(datum = sms->hash_datums_list; datum; datum = next) {
        next = datum->next;
        TMRM_FREE(tmrm_hash_datum, datum);
    }
    sms->hash_datums_list=NULL;
}


/**
 * tmrm_hash_datum_new:
 * @param sms Subject map sphere object
 * @param data data to store
 * @param size size of data
 *
 * Constructor - Create a new tmrm_hash_datum object.
 * 
 * @returns New tmrm_hash_datum object or NULL on failure
 **/
tmrm_hash_datum*
tmrm_hash_datum_new(tmrm_subject_map_sphere *sms, void *data, size_t size)
{
    tmrm_hash_datum *datum;

    /*tmrm_subject_map_sphere_open(sms);*/

    /* get one from free list, or allocate new one */ 
    if ((datum = sms->hash_datums_list)) {
        sms->hash_datums_list = datum->next;
    } else {
        datum = (tmrm_hash_datum*)TMRM_CALLOC(tmrm_hash_datum, 1, sizeof(tmrm_hash_datum));
        if (datum)
            datum->sms = sms;
    }

    if (datum) {
        datum->data = data;
        datum->size = size;
    }

    return datum;
}


/**
 * tmrm_hash_datum_free:
 * @param datum hash datum object
 *
 * Destructor - destroy a tmrm_hash_datum object.
 *
 **/
void
tmrm_hash_datum_free(tmrm_hash_datum *datum) 
{
    if(datum->data)
        TMRM_FREE(cstring, datum->data);
    datum->next = datum->sms->hash_datums_list;
    datum->sms->hash_datums_list = datum;
}


/* class methods */

/**
 * tmrm_hash_register_factory:
 * @param sms Subject map sphere object
 * @param name the hash factory name
 * @param factory pointer to function to call to register the factory
 *
 * Register a hash factory.
 * 
 **/
void
tmrm_hash_register_factory(tmrm_subject_map_sphere *sms, const char *name,
                             void (*factory) (tmrm_hash_factory*)) 
{
    tmrm_hash_factory *hash;

    /*tmrm_subject_map_sphere_open(sms);*/
    TMRM_DEBUG2("Registering hash factory %s\n", name);

    for (hash = sms->hashes; hash; hash = hash->next ) {
        if (!strcmp(hash->name, name)) {
            /*tmrm_log(sms, 0, TMRM_LOG_ERROR, TMRM_FROM_HASH, NULL,
              "hash %s already registered", hash->name);*/
            return;
        }
    }

    hash = (tmrm_hash_factory*)TMRM_CALLOC(tmrm_hash_factory, 1,
            sizeof(tmrm_hash_factory));
    if (!hash)
        goto oom;

    hash->name = (char*)TMRM_MALLOC(cstring, strlen(name)+1);
    if (!hash->name)
        goto oom_tidy;
    strcpy(hash->name, name);

    hash->next = sms->hashes;
    sms->hashes = hash;

    /* Call the hash registration function on the new object */
    (*factory)(hash);

#if defined(TMRM_DEBUG) && TMRM_DEBUG > 1
    TMRM_DEBUG3("%s has context size %d\n", name, hash->context_length);
#endif

    return;

oom_tidy:
    TMRM_FREE(tmrm_hash, hash);
oom:
    /* FIXME TMRM_FATAL1(sms, TMRM_FROM_HASH, "Out of memory"); */
    return;
}


/**
 * Get a hash factory by name.
 * 
 * @param sms Subject map sphere object
 * @param name the factory name or NULL for the default factory
 * @returns the factory object or NULL if there is no such factory
 * 
 * FIXME: several bits of code assume the default hash factory is
 * in memory.
 */
tmrm_hash_factory*
tmrm_get_hash_factory(tmrm_subject_map_sphere *sms, const char *name) 
{
    tmrm_hash_factory *factory;

    /*tmrm_subject_map_sphere_open(sms);*/

    /* return 1st hash if no particular one wanted - why? */
    if (!name) {
        factory = sms->hashes;
        if(!factory) {
            TMRM_DEBUG1("No (default) hashes registered\n");
            return NULL;
        }
    } else {
        for(factory = sms->hashes; factory; factory = factory->next) {
            if(!strcmp(factory->name, name)) {
                break;
            }
        }
        /* else FACTORY name not found */
        if(!factory)
            return NULL;
    }

    return factory;
}



/**
 * Constructor - create a new tmrm_hash object.
 *
 * @param sms Subject map system object
 * @param name factory name
 * @returns a new tmrm_hash object or NULL on failure.
 */
tmrm_hash*
tmrm_hash_new(tmrm_subject_map_sphere *sms, const char* name)
{
    tmrm_hash_factory *factory;

    /*tmrm_subject_map_sphere_open(sms);*/

    factory = tmrm_get_hash_factory(sms, name);
    if (!factory)
        return NULL;

    return tmrm_hash_new_from_factory(sms, factory);
}


/**
 * Constructor - create a new tmrm_hash object from a factory.
 *
 * @param sms Subject map sphere object.
 * @param factory The factory to use to construct the hash
 * @returns A new tmrm_hash object or NULL on failure.
 */
tmrm_hash*
tmrm_hash_new_from_factory(tmrm_subject_map_sphere *sms,
                             tmrm_hash_factory* factory)
{
    tmrm_hash* h;

    /*tmrm_subject_map_sphere_open(sms);*/

    h = (tmrm_hash*)TMRM_CALLOC(tmrm_hash, sizeof(tmrm_hash), 1);
    if(!h)
        return NULL;

    h->context = (char*)TMRM_CALLOC(tmrm_hash_context, 1,
            factory->context_length);
    if(!h->context) {
        tmrm_hash_free(h);
        return NULL;
    }

    h->sms = sms;

    h->factory = factory;

    /* call factory constructor */
    if (h->factory->create(h, h->context)) {
        tmrm_hash_free(h);
        return NULL;
    }

    return h;
}


/**
 * Constructor - create a new tmrm_hash object from a string.
 * @param sms Subject map sphere object.
 * @param name Hash name
 * @param string Hash encoded as a string
 * @returns A new tmrm_hash object or NULL on failure
 *
 * See tmrm_hash_from_string for the string format.
 */
tmrm_hash*
tmrm_hash_new_from_string(tmrm_subject_map_sphere *sms, const char *name,
                            const char *string)
{
    tmrm_hash* hash;

    /*tmrm_subject_map_sphere_open(sms);*/

    hash = tmrm_hash_new(sms, name);
    if(!hash)
        return NULL;

    if(tmrm_hash_from_string(hash, string)) {
        tmrm_hash_free(hash);
        return NULL;
    }

    return hash;
}


/**
 * Constructor - create a new tmrm_hash object from an array of strings.
 * @param sms Subject map sphere object.
 * @param name Hash name
 * @param array Address of the start of the array of char* pointers
 * @returns A new tmrm_hash object or NULL on failure.
 */
tmrm_hash*
tmrm_hash_new_from_array_of_strings(tmrm_subject_map_sphere *sms, const char *name,
                                      const char **array)
{
    tmrm_hash* hash;

    /*tmrm_subject_map_sphere_open(sms);*/

    hash = tmrm_hash_new(sms, name);
    if(!hash)
        return NULL;

    if(tmrm_hash_from_array_of_strings(hash, array)) {
        tmrm_hash_free(hash);
        return NULL;
    }

    return hash;
}


/**
 * Copy Constructor - create a new tmrm_hash object from an existing one.
 * @param old_hash The hash to use to construct the hash
 * @returns A new tmrm_hash object or NULL on failure.
 */
tmrm_hash*
tmrm_hash_new_from_hash(tmrm_hash* old_hash)
{
    tmrm_hash* hash;

    hash=(tmrm_hash*)TMRM_CALLOC(tmrm_hash, sizeof(tmrm_hash), 1);
    if(!hash)
        return NULL;

    hash->sms = old_hash->sms;
    hash->factory = old_hash->factory;

    hash->context = (char*)TMRM_CALLOC(tmrm_hash_context, 1,
            hash->factory->context_length);
    if(!hash->context) {
        tmrm_hash_free(hash);
        return NULL;
    }

    if(old_hash->identifier) {
        /* FIXME: Implement function to create an identifier */
        hash->identifier = "FOOBAR"; /* tmrm_heuristic_gen_name(old_hash->identifier); */
        if(!hash->identifier) {
            tmrm_hash_free(hash);
            return NULL;
        }
    }

    if(hash->factory->clone(hash, hash->context, hash->identifier,
                old_hash->context)) {
        if(hash->identifier)
            TMRM_FREE(cstring, hash->identifier);
        tmrm_hash_free(hash);
        return NULL;
  }

  return hash;
}


/**
 * Destructor - destroy a tmrm_hash object.
 * @param hash Hash object
 */
void
tmrm_hash_free(tmrm_hash* hash) 
{
    if(hash->context) {
        if(hash->is_open)
            tmrm_hash_close(hash);
        hash->factory->destroy(hash->context);
        TMRM_FREE(tmrm_hash_context, hash->context);
    }
    TMRM_FREE(tmrm_hash, hash);
}


/* methods */

/**
 * Start a hash association.
 * This method opens and/or creates a new hash with any resources it
 * needs.
 *
 * @param hash Hash object
 * @param identifier Identifier for the hash factory - usually a URI or file name
 * @param mode hash access mode
 * @param is_writable is hash writable?
 * @param is_new is hash new?
 * @param options a hash of options for the hash factory or NULL if there are none.
 * @returns Non 0 on failure
 */
int
tmrm_hash_open(tmrm_hash* hash, const char *identifier,
                 int mode, int is_writable, int is_new,
                 tmrm_hash* options) 
{
    int status;

    if(identifier) {
        hash->identifier = (char*)TMRM_MALLOC(cstring, strlen(identifier)+1);
        if(!hash->identifier)
            return 1;
        strcpy(hash->identifier, identifier);
    }
    status = hash->factory->open(hash->context, identifier, 
            mode, is_writable, is_new, 
            options);
    if(!status)
        hash->is_open = 1;
    return status;
}


/**
 * End a hash association.
 *
 * @param hash hash object
 * @returns non 0 on failure
 */
int
tmrm_hash_close(tmrm_hash* hash)
{
    hash->is_open = 0;
    if(hash->identifier) {
        TMRM_FREE(cstring, hash->identifier);
        hash->identifier = NULL;
    }
    return hash->factory->close(hash->context);
}


/**
 * Get the number of values in the hash.
 *
 * @param hash Hash object
 * @returns Integer number of values in the hash or <0 if cannot be determined
 */
int
tmrm_hash_size(tmrm_hash* hash) 
{
    return hash->factory->values_count(hash->context);
}


/**
 * Retrieve one value from hash for a given key as string.
 * The value returned is from newly allocated memory which the
 * caller must free.
 * 
 * @param hash hash object
 * @param key pointer to key
 * @returns Returns the value or NULL on failure
 */
char*
tmrm_hash_get(tmrm_hash* hash, const char *key)
{
    tmrm_hash_datum *hd_key, *hd_value;
    char *value = NULL;

    hd_key = tmrm_hash_datum_new(hash->sms, (void*)key, strlen(key));
    if(!hd_key)
        return NULL;

    hd_value = tmrm_hash_get_one(hash, hd_key);

    if(hd_value) {
        if(hd_value->data) {
            value = (char*)TMRM_MALLOC(cstring, hd_value->size+1);
            if(value) {
                /* Copy into new null terminated string for userland */
                memcpy(value, hd_value->data, hd_value->size);
                value[hd_value->size]='\0';
            }
        }
        tmrm_hash_datum_free(hd_value);
    }

    /* don't free user key */
    hd_key->data = NULL;
    tmrm_hash_datum_free(hd_key);

    return value;
}


/**
 * Retrieve one value from hash for a given key.
 * The value returned is from newly allocated memory which the
 * caller must free.
 * 
 * @param hash Hash object
 * @param key Pointer to key
 * @returns The value or NULL on failure
 */
tmrm_hash_datum*
tmrm_hash_get_one(tmrm_hash* hash, tmrm_hash_datum *key)
{
    tmrm_hash_datum *value;
    tmrm_hash_cursor *cursor;
    int status;
    char *new_value;

    value = tmrm_hash_datum_new(hash->sms, NULL, 0);
    if(!value)
        return NULL;

    cursor = tmrm_hash_cursor_new(hash);
    if(!cursor) {
        tmrm_hash_datum_free(value);
        return NULL;
    }

    status = tmrm_hash_cursor_get_next(cursor, key, value);
    if(!status) {
        /* value->data will point to SHARED area, so copy it */
        new_value = (char*)TMRM_MALLOC(cstring, value->size);
        if(new_value) {
            memcpy(new_value, value->data, value->size);
            value->data = new_value;
        } else {
            status = 1;
            value->data = NULL;
        }
    }

    /* this deletes the data behind the datum */
    tmrm_hash_cursor_free(cursor);

    if(status) {
        tmrm_hash_datum_free(value);
        return NULL;
    }

    return value;
}


typedef struct {
    tmrm_hash* hash;
    tmrm_hash_cursor* cursor;
    tmrm_hash_datum *key;
    tmrm_hash_datum *value;

    tmrm_hash_datum next_key; /* not used if one_key set */
    tmrm_hash_datum next_value;
    int is_end;
    int one_key;
} tmrm_hash_get_all_iterator_context;



/**
 * Retrieve all values from hash for a given key.
 * The iterator returns tmrm_hash_datum objects containing the values.
 * These are newly allocated memory which the caller must free.
 *
 * @param hash Hash object
 * @param key Pointer to key
 * @param value Pointer to value
 * @returns a tmrm_iterator serialization of all values or NULL on failure
 */
tmrm_iterator*
tmrm_hash_get_all(tmrm_hash* hash, 
                    tmrm_hash_datum *key, tmrm_hash_datum *value)
{
    tmrm_hash_get_all_iterator_context* context;
    int status;
    tmrm_iterator* iterator;

    context = (tmrm_hash_get_all_iterator_context*)TMRM_CALLOC(
        tmrm_hash_get_all_iterator_context, 1, sizeof(tmrm_hash_get_all_iterator_context));
    if(!context)
        return NULL;

    if(!(context->cursor = tmrm_hash_cursor_new(hash))) {
        tmrm_hash_get_all_iterator_finished(context);
        return NULL;
    }

    if(key->data)
        context->one_key = 1;

    context->hash = hash;
    context->key = key;
    context->value = value;

    if(context->one_key)
        status = tmrm_hash_cursor_set(context->cursor, context->key, 
                &context->next_value);
    else
        status = tmrm_hash_cursor_get_first(context->cursor, &context->next_key, 
                &context->next_value);

    context->is_end = (status != 0);

    iterator = tmrm_iterator_new(hash->sms,
            (void*)context,
            tmrm_hash_get_all_iterator_next_method,
            tmrm_hash_get_all_iterator_is_end,
            tmrm_hash_get_all_iterator_get_method,
            tmrm_hash_get_all_iterator_finished);
    if(!iterator)
        tmrm_hash_get_all_iterator_finished(context);
    return iterator;
}


static int
tmrm_hash_get_all_iterator_is_end(void* iterator)
{
    tmrm_hash_get_all_iterator_context* context = 
        (tmrm_hash_get_all_iterator_context*)iterator;

    return context->is_end;
}


static int
tmrm_hash_get_all_iterator_next_method(void* iterator) 
{
  tmrm_hash_get_all_iterator_context* context =
      (tmrm_hash_get_all_iterator_context*)iterator;
  int status;

  if(context->is_end)
      return 1;

  /* move on */

  if(context->one_key)
      status = tmrm_hash_cursor_get_next_value(context->cursor, 
              &context->next_key,
              &context->next_value);
  else {
      /* want the next key/value pair, so mark last data as used */
      context->next_key.data = NULL;
      status = tmrm_hash_cursor_get_next(context->cursor, 
              &context->next_key, 
              &context->next_value);
  }

  if(status)
      context->is_end = 1;

  return context->is_end;
}


static void*
tmrm_hash_get_all_iterator_get_method(void* iterator, tmrm_iterator_flag flags) 
{
    tmrm_hash_get_all_iterator_context* context = 
        (tmrm_hash_get_all_iterator_context*)iterator;
    tmrm_hash_datum *result = NULL;

    if(context->is_end)
        return NULL;

    switch(flags) {
        case TMRM_ITERATOR_GET_METHOD_GET_OBJECT:
            /* This is so that tmrm_iterator_update_current_element works OK,
             * since the get_object method isn't used for hashes,
             * might as well return something useful to signify not-end-of-list.
             */

            result = iterator;
            break;

        case TMRM_ITERATOR_GET_METHOD_GET_KEY:
            result = &context->next_key;
            break;

        case TMRM_ITERATOR_GET_METHOD_GET_VALUE:
            result = &context->next_value;
            break;

        default:
            /*
            tmrm_log(context->hash->sms, 
                    0, tmrm_LOG_ERROR, tmrm_FROM_HASH, NULL,
                    "Unknown iterator method flag %d", flags);
            */
            result = (void*)NULL;
            break;
    }

    return result;
}


static void
tmrm_hash_get_all_iterator_finished(void* iterator) 
{
    tmrm_hash_get_all_iterator_context* context =
        (tmrm_hash_get_all_iterator_context*)iterator;

    if(context->cursor)
        tmrm_hash_cursor_free(context->cursor);

    if(context->key)
        context->key->data = NULL;

    if(context->value)
        context->value->data = NULL;

    TMRM_FREE(tmrm_hash_get_all_iterator_context, context);
}


/**
 * Retrieve one value from hash for a given key as string and remove all values with that key.
 * The value returned is from newly allocated memory which the
 * caller must free.
 *
 * @param hash Hash object
 * @param key Pointer to key
 * @returns the value or NULL on failure
 */
char*
tmrm_hash_get_del(tmrm_hash* hash, const char *key)
{
    tmrm_hash_datum hd_key;
    char *s;

    s = tmrm_hash_get(hash, key);
    if(!s)
        return NULL;

    hd_key.data = (char*)key;
    hd_key.size = strlen(key);

    tmrm_hash_delete_all(hash, &hd_key);

    return s;
}



/**
 * Insert key/value pairs into the hash according to flags.
 * The key and values are copied into the hash; the original pointers
 * can be deleted.
 * 
 * @param hash Hash object
 * @param key key 
 * @param value value
 * @returns non 0 on failure
 */
int
tmrm_hash_put(tmrm_hash* hash, tmrm_hash_datum *key, 
                tmrm_hash_datum *value)
{
    return hash->factory->put(hash->context, key, value);
}


/**
 * Check if a given key/value is in the hash.
 * 
 * @param hash hash object
 * @param key key
 * @param value value
 * @returns >0 if the key/value exists in the hash, 0 if not, <0 on failure
 */
int
tmrm_hash_exists(tmrm_hash* hash, tmrm_hash_datum *key,
                   tmrm_hash_datum *value)
{
    return hash->factory->exists(hash->context, key, value);
}


/**
 * Delete a key/value pair from the hash.
 * 
 * @param hash hash object
 * @param key key
 * @param value value
 * @returns non 0 on failure (including pair not present)
 */
int
tmrm_hash_delete(tmrm_hash* hash, tmrm_hash_datum *key,
                   tmrm_hash_datum *value)
{
    return hash->factory->delete_key_value(hash->context, key, value);
}


/**
 * Delete a key and all values from the hash.
 * 
 * @param hash hash object
 * @param key key
 * @returns non 0 on failure (including pair not present)
 */
int
tmrm_hash_delete_all(tmrm_hash* hash, tmrm_hash_datum *key)
{
    return hash->factory->delete_key(hash->context, key);
}


typedef struct {
    tmrm_hash* hash;
    tmrm_hash_cursor* cursor;
    tmrm_hash_datum *key;

    tmrm_hash_datum next_key;
    int is_end;
} tmrm_hash_keys_iterator_context;



/**
 * Get the hash keys.
 * The iterator returns tmrm_hash_datum objects containingvalue returned is from newly allocated memory which the
 * caller must free.
 * 
 * @param hash hash object
 * @param key pointer to key
 * @returns tmrm_iterator serialisation of keys or NULL on failure
 */
tmrm_iterator*
tmrm_hash_keys(tmrm_hash* hash, tmrm_hash_datum *key)
{
    tmrm_hash_keys_iterator_context* context;
    int status;
    tmrm_iterator* iterator;

    context = (tmrm_hash_keys_iterator_context*)TMRM_CALLOC(
            tmrm_hash_keys_iterator_context, 1, sizeof(tmrm_hash_keys_iterator_context));
    if(!context)
        return NULL;


    if(!(context->cursor = tmrm_hash_cursor_new(hash))) {
        tmrm_hash_keys_iterator_finished(context);
        return NULL;
    }

    context->hash = hash;
    context->key = key;

    status = tmrm_hash_cursor_get_first(context->cursor, &context->next_key, 
            NULL);
    context->is_end = (status != 0);

    iterator = tmrm_iterator_new(hash->sms, 
            (void*)context,
            tmrm_hash_keys_iterator_is_end,
            tmrm_hash_keys_iterator_next_method,
            tmrm_hash_keys_iterator_get_method,
            tmrm_hash_keys_iterator_finished);
    if(!iterator)
        tmrm_hash_keys_iterator_finished(context);
    return iterator;
}


static int
tmrm_hash_keys_iterator_is_end(void* iterator)
{
    tmrm_hash_keys_iterator_context* context =
        (tmrm_hash_keys_iterator_context*)iterator;

    if(context->is_end)
        return 1;

    /* have key */
    if(context->next_key.data)
        return 0;

    /* no stored data, so check for it */
    if(tmrm_hash_cursor_get_next(context->cursor, &context->next_key, NULL))
        context->is_end = 1;

    return context->is_end;
}


static int
tmrm_hash_keys_iterator_next_method(void* iterator) 
{
    tmrm_hash_keys_iterator_context* context = 
        (tmrm_hash_keys_iterator_context*)iterator;

    if(context->is_end)
        return 1;

    /* move on */

    /* want the next key, so mark last key data as used */
    context->next_key.data = NULL;
    if(tmrm_hash_cursor_get_next(context->cursor, &context->next_key, NULL))
        context->is_end = 1;

    return context->is_end;
}


static tmrm_object*
tmrm_hash_keys_iterator_get_method(void* iterator, tmrm_iterator_flag flags) 
{
    tmrm_object *result = NULL;
    tmrm_hash_keys_iterator_context* context =
        (tmrm_hash_keys_iterator_context*)iterator;

    if(context->is_end)
        return NULL;

    switch(flags) {
        case TMRM_ITERATOR_GET_METHOD_GET_OBJECT:
            /* This is so that tmrm_iterator_update_current_element works OK,
             * since the get_object method isn't used for hashes,
             * might as well return something useful to signify not-end-of-list.
             */
            result = (tmrm_object*)iterator;
            break;

        case TMRM_ITERATOR_GET_METHOD_GET_KEY:
            result = (tmrm_object*)&context->next_key;
            break;

        default:
            result = NULL;
    }

    return result;
}


static void
tmrm_hash_keys_iterator_finished(void* iterator) 
{
    tmrm_hash_keys_iterator_context* context =
        (tmrm_hash_keys_iterator_context*)iterator;

    if(context->cursor)
        tmrm_hash_cursor_free(context->cursor);

    context->key->data = NULL;

    TMRM_FREE(tmrm_hash_keys_iterator_context, context);
}


/**
 * Flush any cached information to disk if appropriate.
 * 
 * @param hash hash object
 * @returns non 0 on failure
 */
int
tmrm_hash_sync(tmrm_hash* hash)
{
    return hash->factory->sync(hash->context);
}


/**
 * Get the file descriptor for the hash.
 * This returns the file descriptor if it is file based for
 * use with file locking.
 * 
 * @param hash hash object
 * @returns the file descriptor
 */
int
tmrm_hash_get_fd(tmrm_hash* hash)
{
    return hash->factory->get_fd(hash->context);
}


/**
 * Pretty print the hash to a file descriptor.
 *
 * @param hash the hash
 * @param fh file handle
 */
void
tmrm_hash_print(tmrm_hash* hash, FILE *fh) 
{
    tmrm_iterator* iterator;
    tmrm_hash_datum *key, *value;

    fputs(hash->factory->name, fh);
    fputs(" hash: {\n", fh);

    key = tmrm_hash_datum_new(hash->sms, NULL, 0);
    value = tmrm_hash_datum_new(hash->sms, NULL, 0);

    iterator = tmrm_hash_get_all(hash, key, value);
    while(!tmrm_iterator_end(iterator)) {
        tmrm_hash_datum *k, *v;
        size_t l;

        k = (tmrm_hash_datum *)tmrm_iterator_get_key(iterator);
        v = (tmrm_hash_datum *)tmrm_iterator_get_value(iterator);

        fputs("  '", fh);
        l = fwrite(k->data, k->size, 1, fh);
        if(l != 1) {
            break;
        }

        fputs("'=>'", fh);
        l = fwrite(v->data, v->size, 1, fh);
        if(l != 1) {
            break;
        }

        fputs("'\n", fh);

        tmrm_iterator_next(iterator);
    }
    if(iterator)
        tmrm_iterator_free(iterator);

    tmrm_hash_datum_free(value);
    tmrm_hash_datum_free(key);

    fputs("}\n", fh);
}


/**
 * Pretty print the keys to a file descriptor.
 *
 * @param hash the hash
 * @param fh file handle
 */
void
tmrm_hash_print_keys(tmrm_hash* hash, FILE *fh) 
{
    tmrm_iterator* iterator;
    tmrm_hash_datum *key;

    fputs("{\n", fh);

    key = tmrm_hash_datum_new(hash->sms, NULL, 0);

    iterator = tmrm_hash_keys(hash, key);
    while(!tmrm_iterator_end(iterator)) {
        tmrm_hash_datum *k = (tmrm_hash_datum *)tmrm_iterator_get_key(iterator);
        size_t l;

        fputs("  '", fh);
        l=fwrite(k->data, k->size, 1, fh);
        if(l != 1)
            break;
        fputs("'\n", fh);

        tmrm_iterator_next(iterator);
    }
    if(iterator)
        tmrm_iterator_free(iterator);

    tmrm_hash_datum_free(key);

    fputc('}', fh);
}


/**
 * Pretty print the values of one key to a file descriptor.
 *
 * @param hash the hash
 * @param key_string the key as a string
 * @param fh param file handle
 */
void
tmrm_hash_print_values(tmrm_hash* hash, const char *key_string, FILE *fh)
{
    tmrm_hash_datum *key, *value;
    tmrm_iterator* iterator;
    int first=1;

    key = tmrm_hash_datum_new(hash->sms, (char*)key_string, strlen(key_string));
    if(!key)
        return;

    value = tmrm_hash_datum_new(hash->sms, NULL, 0);
    if(!value) {
        key->data = NULL;
        tmrm_hash_datum_free(key);
        return;
    }

    iterator = tmrm_hash_get_all(hash, key, value);
    fputc('(', fh);
    while(!tmrm_iterator_end(iterator)) {
        tmrm_hash_datum *v = (tmrm_hash_datum *)tmrm_iterator_get_value(iterator);
        size_t l;

        if(!first)
            fputs(", ", fh);

        fputc('\'', fh);
        l = fwrite(v->data, v->size, 1, fh);
        if(l != v->size)
            break;

        fputc('\'', fh);
        first = 0;
        tmrm_iterator_next(iterator);
    }
    fputc(')', fh);
    tmrm_iterator_free(iterator);

    key->data = NULL;
    tmrm_hash_datum_free(key);

    tmrm_hash_datum_free(value);
}



/* private enum */
typedef enum {
    HFS_PARSE_STATE_INIT = 0,
    HFS_PARSE_STATE_KEY = 1,
    HFS_PARSE_STATE_SEP = 2,
    HFS_PARSE_STATE_EQ = 3,
    HFS_PARSE_STATE_VALUE = 4
} tmrm_hfs_parse_state;

/**
 * Initialise a hash from a string.
 * 
 * The string format is something like:
 * key1='value1',key2='value2', key3='\'quoted value\''
 *
 * The 's are required and whitespace can appear around the = and ,s
 * 
 * @param hash hash object
 * @param string hash encoded as a string
 * @returns non 0 on failure.
 **/
int
tmrm_hash_from_string(tmrm_hash* hash, const char *string) 
{
    const char * p;
    tmrm_hash_datum hd_key, hd_value; /* on stack */
    const char *key;
    size_t key_len;
    const char *value;
    size_t value_len;
    int backslashes;
    int saw_backslash;
    tmrm_hfs_parse_state state;
    int real_value_len;
    char *new_value;
    int i;
    char *to;

    if(!string)
        return 0;

#if defined(TMRM_DEBUG) && TMRM_DEBUG > 1
    TMRM_DEBUG2("Parsing >>%s<<\n", string);
#endif

    p = string;
    key = NULL; key_len = 0;
    value = NULL; value_len = 0;
    backslashes = 0;
    state = HFS_PARSE_STATE_INIT;
    while(*p) {

#if defined(TMRM_DEBUG) && TMRM_DEBUG > 1
        TMRM_DEBUG3("state %d at %s\n", state, p);
#endif

        switch(state){
            /* start of config - before key */
            case HFS_PARSE_STATE_INIT:
                while(*p && (isspace((int)*p) || *p == ','))
                    p++;
                if(!*p)
                    break;

                /* fall through to next state */
                state = HFS_PARSE_STATE_KEY;

#if defined(TMRM_DEBUG) && TMRM_DEBUG > 1
                TMRM_DEBUG3("state %d at %s\n", state, p);
#endif

                /* start of key */
            case HFS_PARSE_STATE_KEY:
                key = p;
                while(*p && (isalnum((int)*p) || *p == '_' || *p == '-'))
                    p++;
                if(!*p)
                    break;
                key_len = p-key;

                /* if 1st char is not space or alpha, move on */
                if(!key_len) {
                    p++;
                    state = HFS_PARSE_STATE_INIT;
                    break;
                }
        
                state = HFS_PARSE_STATE_SEP;
                /* fall through to next state */

#if defined(TMRM_DEBUG) && TMRM_DEBUG > 1
                TMRM_DEBUG3("state %d at %s\n", state, p);
#endif

                /* got key, now skipping spaces */
            case HFS_PARSE_STATE_SEP:
                while(*p && isspace((int)*p))
                    p++;
                if(!*p)
                    break;
                /* expecting = now */
                if(*p != '=') {
                    p++;
                    state = HFS_PARSE_STATE_INIT;
                    break;
                }
                p++;
                state = HFS_PARSE_STATE_EQ;
                /* fall through to next state */

#if defined(TMRM_DEBUG) && TMRM_DEBUG > 1
                TMRM_DEBUG3("state %d at %s\n", state, p);
#endif

                /* got key\s+= now skipping spaces " */
            case HFS_PARSE_STATE_EQ:
                while(*p && isspace((int)*p))
                    p++;
                if(!*p)
                    break;
                /* expecting ' now */
                if(*p != '\'') {
                    p++;
                    state = HFS_PARSE_STATE_INIT;
                    break;
                }
                p++;
                state = HFS_PARSE_STATE_VALUE;
                /* fall through to next state */

#if defined(TMRM_DEBUG) && TMRM_DEBUG > 1
                TMRM_DEBUG3("state %d at %s\n", state, p);
#endif

                /* got key\s+=\s+" now reading value */
            case HFS_PARSE_STATE_VALUE:
                value = p;
                backslashes = 0;
                saw_backslash = 0;
                while(*p) {
                    if(!saw_backslash && *p == '\\') {
                        /* backslashes are removed during value copy later */
                        backslashes++; /* reduces real length */
                        saw_backslash = 1;
                    } else {
                        if (!saw_backslash && *p == '\'')
                            break;
                        saw_backslash = 0;
                    }

                    p++;
                }
                if(!*p)
                    return 1;

                /* ' at end of value found */
                value_len = p-value;
                real_value_len = value_len - backslashes;
                new_value = (char*)TMRM_MALLOC(cstring, real_value_len+1);
                if(!new_value)
                    return 1;
                for(i=0, to = new_value; i<(int)value_len; i++){
                    if(value[i] == '\\')
                        i++;
                    *to++ = value[i];
                }
                *to='\0';

#if defined(TMRM_DEBUG) && TMRM_DEBUG > 1
                TMRM_DEBUG3("decoded key >>%s<< (true) value >>%s<<\n", key, new_value);
#endif

                hd_key.data = (void*)key; hd_key.size = key_len;
                hd_value.data = (void*)new_value; hd_value.size = real_value_len;

                tmrm_hash_put(hash, &hd_key, &hd_value);

                TMRM_FREE(cstring, new_value);

#if defined(TMRM_DEBUG) && TMRM_DEBUG > 1
                TMRM_DEBUG1("after decoding ");
                tmrm_hash_print(hash, stderr) ;
                fputc('\n', stderr);
#endif
                state = HFS_PARSE_STATE_INIT;
                p++;

                break;

            default:
                /*
                   tmrm_log(hash->sms, 
                   0, tmrm_LOG_ERROR, tmrm_FROM_HASH, NULL,
                   "No such state %d", state);
                 */
                return 1;
        }
    }
    return 0;
}


/**
 * Initialise a hash from an array of strings.
 * 
 * @param hash hash object
 * @param array address of the start of the array of char* pointers
 * @returns non 0 on failure
 **/
int
tmrm_hash_from_array_of_strings(tmrm_hash* hash, const char **array)
{
    tmrm_hash_datum key, value; /* on stack */
    int i;

    for(i = 0; (key.data = (char*)array[i]); i+=2) {
        value.data = (char*)array[i+1];
        if(!value.data) {
            /*
               tmrm_log(hash->sms, 
               0, tmrm_LOG_ERROR, tmrm_FROM_HASH, NULL,
               "Array contains an odd number of strings - %d", i);
             */
            return 1;
        }
        key.size = strlen((char*)key.data);
        value.size = strlen((char*)value.data);
        tmrm_hash_put(hash, &key, &value);
    }
    return 0;
}


/**
 * Lookup a hash key and decode value as a boolean.
 * 
 * @param hash tmrm_hash object
 * @param key key string to look up
 * @returns >0 (for true), 0 (for false) or <0 (for key not found or not known boolean value)
 */
int
tmrm_hash_get_as_boolean (tmrm_hash* hash, const char *key) 
{
    int bvalue = (-1);
    char *value;

    value = tmrm_hash_get(hash, key);
    if(!value)
        /* does not exist - fail */
        return -1;

    switch(strlen(value)) {
        case 2: /* try 'no' */
            if(*value == 'n' && value[1] == 'o')
                bvalue = 0;
            break;
        case 3: /* try 'yes' */
            if(*value == 'y' && value[1] == 'e' && value[2] == 's')
                bvalue = 1;
            break;
        case 4: /* try 'true' */
            if(*value == 't' && value[1] == 'r' && value[2] == 'u' && value[3] == 'e')
                bvalue = 1;
            break;
        case 5: /* try 'false' */
            if(!strncmp(value, "false", 5))
                bvalue = 0;
            break;
            /* no need for default, bvalue is set above */
    }

    TMRM_FREE(cstring, value);

    return bvalue;
}


/**
 * Lookup a hash key and decode value as a long.
 *
 * @param hash tmrm_hash object
 * @param key key string to look up
 * @returns >0 (for success), <0 (for key not found or not known boolean value)
 **/
long
tmrm_hash_get_as_long (tmrm_hash* hash, const char *key) 
{
    int lvalue;
    char *value;
    char *end_ptr;

    value = tmrm_hash_get(hash, key);
    if(!value)
        /* does not exist - fail */
        return -1;

    /* Using special base 0 which allows decimal, hex and octal */
    lvalue = strtol(value, &end_ptr, 0);

    /* nothing found, return error */
    if(end_ptr == value)
        lvalue = (-1);

    TMRM_FREE(cstring, value);
    return lvalue;
}

/**
 * Insert key/value pairs into the hash as strings.
 * The key and values are copied into the hash, no sharing is done.
 * 
 * @param hash hash object
 * @param key key 
 * @param value value
 * @returns non 0 on failure
 */
int
tmrm_hash_put_strings(tmrm_hash* hash, const char *key, const char *value)
{
    tmrm_hash_datum key_hd; /* static */
    tmrm_hash_datum value_hd;

    /* Note: We do not have to init the sms field of
     * these tmrm_hash_datum since they are never put on the
     * hash datums free list
     */

    key_hd.data = (void*)key;
    key_hd.size = strlen(key);
    value_hd.data = (void*)value;
    value_hd.size = strlen(value);
    return tmrm_hash_put(hash, &key_hd, &value_hd);
}


#endif

