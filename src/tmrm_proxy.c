/*
 * tmrm_proxy.c - Implementation of the proxy-related functions.
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

#ifdef HAVE_CONFIG_H
#include <libtmrm_config.h>
#endif

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_YAML_H
#include <yaml.h>
#endif


#include <libtmrm.h>
#include <tmrm_internal.h>
#include <tmrm_storage.h>
#include <tmrm_hash.h>


/* TODO Move this to one of the header files */
struct tmrm_streaming_handler_s {
    int (*subject_map_start)(void *context, tmrm_char_t *name);
    int (*subject_map_end)(void *context);
    int (*proxy_start)(void *context, tmrm_char_t *label);
    int (*property)(void *context, tmrm_char_t *key, tmrm_char_t *value);
    int (*property_literal)(void *context, tmrm_char_t *key, tmrm_char_t *value, tmrm_char_t *datatype);
    int (*proxy_end)(void *context);
};

typedef struct tmrm_streaming_handler_s tmrm_streaming_handler;

struct tmrm_yaml_export_context_s {
    FILE *fh;
    yaml_emitter_t emitter;
};
typedef struct tmrm_yaml_export_context_s tmrm_yaml_export_context;

struct tmrm_yaml_import_context_s {
    FILE *fh;
    yaml_parser_t parser;
};
typedef struct tmrm_yaml_import_context_s tmrm_yaml_import_context;

static int tmrm_subject_map_export_streamer(tmrm_subject_map *map,
    tmrm_streaming_handler *h, void *handler_context);
static int tmrm_subject_map_import_streamer(tmrm_subject_map *map,
    tmrm_streaming_handler *h, void *handler_context);

static int tmrm_yaml_export_subject_map_start(void *context, tmrm_char_t *name);
static int tmrm_yaml_export_subject_map_end(void *context);
static int tmrm_yaml_export_proxy_start(void *context, tmrm_char_t *label);
static int tmrm_yaml_export_proxy_end(void *context);
static int tmrm_yaml_export_property(void *context, tmrm_char_t *key, tmrm_char_t *value);
static int tmrm_yaml_export_property_literal(void *context, tmrm_char_t *key, tmrm_char_t *value, tmrm_char_t *datatype);

/* /testing */

/**
 * Creates a new proxy object. The proxy is written to the backend immediately.
 *
 * @param m The subject map that shall contain the proxy.
 * @returns NULL on failure.
 */
/*@null@*/ tmrm_proxy* 
tmrm_proxy_new(tmrm_subject_map* m)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(m, tmrm_subject_map, NULL);

    return tmrm_storage_proxy_create(m->storage, m);
}


/**
 * Returns a copy of the object that represents the proxy p.
 * Note that this function does not create a copy of the actual proxy, it only
 * copies the object that represents it.
 * p must not be NULL.
 *
 * @returns NULL on error, or a copy of @p otherwise.
 */
/*@null@*/ tmrm_proxy*
tmrm_proxy_clone(tmrm_proxy* p) {
    tmrm_proxy* copy;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);
    
    /* FIXME: call the storage backend to clone the internal structure */
    copy = (tmrm_proxy*)TMRM_CALLOC(tmrm_proxy, 1, sizeof(tmrm_proxy));
    if (!copy) {
        return NULL;
    }
    copy->subject_map = p->subject_map;
    copy->label = p->label;
    copy->type = TMRM_TYPE_PROXY;
    return copy;
}


/**
 * Finishes a series of update operations on a proxy. This function
 * can be used to update internal indexes of the storage module.
 * The user should call this function after all modifications of a
 * proxy (e.g. after adding some properties to the proxy).
 */
int tmrm_proxy_update(tmrm_proxy *p)
{
    return tmrm_storage_proxy_update(p->subject_map->storage, p);
}


/**
 * Returns the label of the proxy p. The caller is responsible to free the
 * returned string with free()
 *
 * PHP-prototype: TMRM_Proxy::getLabel()
 *
 * @returns NULL if an error occurs (e.g. not enough memory).
 */
/*@null@*/ const char*
tmrm_proxy_label(tmrm_proxy* p)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);

    return tmrm_storage_proxy_label(p->subject_map->storage, p);
}


/** 
 * Returns a multiset with all local keys of p.
 * Referred to as Keys() in TMRM.
 *
 * Notation: "PROXY \"
 *
 * @todo TODO p can be proxy or iterator.
*/
tmrm_multiset*
tmrm_proxy_keys(tmrm_proxy *p)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy,
            NULL);
    return tmrm_multiset_new_from_iterator(p->subject_map, 
            tmrm_storage_proxy_keys(p->subject_map->storage, p));
}


/**
 * Returns a list of values behind a particular key for a given proxy. 
 *
 * Notation: PROXY -> KEY
 *
 * PHP-prototype: TMRM_SubjectMap::valuesByKey()
 *
 * tmrm_iterator* tmrm_subject_map_values_by_key(tmrm_iterator* 
 * proxies/tmrm_proxy* proxy, tmrm_proxy* key);
 * => tmrm_iterator* tmrm_proxy_values_by_key(tmrm_object* object,
 *                                            tmrm_proxy* key);
 * object can be tmrm_iterator, tmrm_proxy and not a tmrm_literal
*/
/*@null@*/ tmrm_multiset*
tmrm_proxy_values_by_key(tmrm_proxy* p, tmrm_proxy* key)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(key, tmrm_proxy, NULL);
    return tmrm_multiset_new_from_iterator(p->subject_map, 
            tmrm_storage_proxy_values_by_key(p->subject_map->storage, p, key));
}


/**
 * Returns a list of values behind a particular key for a given proxy by
 * honoring the subclasses of the key. (_t stands for 'transitive')
 *
 * Notation: PROXY -> KEY *
 *
 * PHP-prototype: TMRM_SubjectMap::valuesByKeyWithSubtyping()
 */
/*@null@*/ tmrm_multiset* tmrm_proxy_values_by_key_t(tmrm_proxy* p,
    tmrm_proxy* key) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(key, tmrm_proxy, NULL);

    /* FIXME: Implement! */
    /* Pseudo-code:
       Result set = empty set.
       For all subclasses of key:
           Add the result of tmrm_proxy_values_by_key to the result set
       Return the result set. 
    */
    return NULL;
}


/**
 * Finds all keys (in all proxies in the map) where the proxy is the value
 * for it.
 *
 * Notation: "PROXY /"
 *
 * PHP-prototype: TMRM_SubjectMap::keysByValue()
 *
 * @todo create a similar function for subject map objects: 
 * Value can be tmrm_proxy, tmrm_literal, or tmrm_list/tmrm_iterator
 * => tmrm_iterator* tmrm_subject_map_keys_by_value(tmrm_object* object)
 * @fixme return iterator instead of multiset. Create generic
 * tmrm_subject_map_keys_by_value that takes a tmrm_object as a parameter.
 */
/*@null@*/ tmrm_multiset*
tmrm_proxy_keys_by_value(tmrm_proxy* p) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);
    return tmrm_multiset_new_from_iterator(p->subject_map, 
            tmrm_storage_proxy_keys_by_value(p->subject_map->storage, p));
}


/**
 * All proxies in which the proxies are the value for a particular key.
 * Is is the generalized version of the function. See
 * tmrm_proxy_is_value_by_key() for details.
 *
 * Notation: "PROXY <- KEY"
 *
 * PHP-prototype: TMRM_SubjectMap::isValueByKey()
 */
/*@null@*/ tmrm_multiset*
tmrm_subject_map_is_value_by_key(tmrm_multiset* proxies, tmrm_proxy* key) {
    void *object;
    tmrm_list *list;
    tmrm_list_elmt *node;
    tmrm_multiset *set;
    tmrm_multiset *subset;
    tmrm_object_type type;
    
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(proxies, tmrm_iterator, NULL);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(key, tmrm_proxy, NULL);

    set = tmrm_multiset_new(key->subject_map);
    if (set == NULL) return NULL;

    list = tmrm_multiset_as_list(proxies);
    node = tmrm_list_head(list);
    while (node != NULL) {
        object = (tmrm_object*)tmrm_list_data(node);
        subset = NULL;
        type = tmrm_object_get_type((tmrm_object*)object);
        switch (type) {
            case TMRM_TYPE_PROXY:
                subset = tmrm_proxy_is_value_by_key((tmrm_proxy*)object, key);
                break;
            case TMRM_TYPE_LITERAL:
                subset = tmrm_literal_is_value_by_key((tmrm_literal*)object, key);
                break;
            default: /* */ 
                TMRM_DEBUG2("Unknown object type %d\n", (int)type);
        }
        if (subset != NULL) {
            tmrm_multiset_add(set, subset);
            tmrm_multiset_free(subset);
        }
        node = tmrm_list_next(node);
    }
    tmrm_list_free(list);
    return set;
}


/**
 * All proxies in which the proxy is the value for a particular key.
 *
 * Notation: "PROXY <- KEY"
 *
 * PHP-prototype: TMRM_Proxy::isValueByKey()
 * 
 * @returns NULL on failure
 */
/*@null@*/ tmrm_multiset*
tmrm_proxy_is_value_by_key(tmrm_proxy* p, tmrm_proxy* key) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(key, tmrm_proxy, NULL);

    return tmrm_multiset_new_from_iterator(p->subject_map, 
            tmrm_storage_proxy_is_value_by_key(p->subject_map->storage, p, key));
}


/**
 * All proxies in which the literal is the value for a particular key.
 *
 * Notation: "LITERAL <- KEY"
 *
 * PHP-prototype: TMRM_SubjectMap::isValueByKey()
 */
/*@null@*/ tmrm_multiset*
tmrm_literal_is_value_by_key(tmrm_literal* lit, tmrm_proxy* key) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(lit, tmrm_literal, NULL);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(key, tmrm_proxy, NULL);

    return tmrm_multiset_new_from_iterator(key->subject_map,
        tmrm_storage_literal_is_value_by_key(key->subject_map->storage, lit, key));
}


/**
 * Finds all keys (in all proxies in the map) where the literal is the value
 * for it.
 *
 * Notation: "LITERAL /"
 *
 * PHP-prototype: TMRM_SubjectMap::keysByValue()
 *
 * @internal This function is a bit odd, since it needs the subject map
 * as a parameter, while most other functions don't. The reason is
 * that proxies always are connected to the subject map, while literals are not.
 * This could be fixed by adding the subject map as a parameter for most functions
 * (probably not a good idea). The other idea would be to a reference to the subject
 * map to literals. (which would make sence if the literals also are bound a digest
 * function)
 */
/*@null@*/ tmrm_multiset* tmrm_literal_keys_by_value(tmrm_literal* lit,
    tmrm_subject_map* map) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(lit, tmrm_literal, NULL);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(map, tmrm_subject_map, NULL);

    return tmrm_multiset_new_from_iterator(map,
            tmrm_storage_literal_keys_by_value(map->storage, lit, map));
}


/* Casts the proxy p into a tmrm_object. */
tmrm_object*
tmrm_proxy_to_object(tmrm_proxy *p) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);
    return (tmrm_object*)p;
}

/* Casts the literal lit into a tmrm_object. */
tmrm_object*
tmrm_object_new_from_literal(tmrm_literal *lit, int clone) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(lit, tmrm_literal, NULL);
    return (tmrm_object*)lit;
}

/* Casts the multiset ms into a tmrm_object. */
tmrm_object*
tmrm_object_new_from_multiset(tmrm_multiset *ms, int clone) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(ms, tmrm_multiset,
        (tmrm_object*)NULL);
    return (tmrm_object*)ms;
}

/* Casts the iterator it into a tmrm_object. */
tmrm_object*
tmrm_object_new_from_iterator(tmrm_iterator *it) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(it, tmrm_iterator,
        (tmrm_object*)NULL);
    return (tmrm_object*)it;
}

/* Casts the tuple t into a tmrm_object. */
tmrm_object*
tmrm_object_new_from_tuple(tmrm_tuple *t, int clone) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(t, tmrm_tuple, NULL);
    return (tmrm_object*)t;
}


/**
 * Returns the type of a tmrm_object. object must not be NULL.
 *
 * @param[in] object The object in question
 * @returns TMRM_TYPE_UNKNOWN on failure.
 */
tmrm_object_type
tmrm_object_get_type(const tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, tmrm_object, TMRM_TYPE_UNKNOWN);
    return *object;
}


/**
 * Checks if an tmrm_object represents a proxy object.
 *
 * @param[in] object The object in question
 * @returns 1 if an object is a proxy object or 0 otherwise.
 */
int tmrm_object_is_proxy(const tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, int, 0);
    if (*object == TMRM_TYPE_PROXY) return 1;
    return 0;
}


/**
 * Checks if an tmrm_object represents a literal object.
 *
 * @param[in] object The object in question
 * @returns 1 if an object is a literal object or 0 otherwise.
 */
int tmrm_object_is_literal(const tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, int, 0);
    if (*object == TMRM_TYPE_LITERAL) return 1;
    return 0;
}

/**
 * Checks if an tmrm_object represents a multiset object.
 *
 * @param[in] object The object in question
 * @returns 1 if an object is a multiset object or 0 otherwise.
 */
int tmrm_object_is_multiset(const tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, int, 0);
    if (*object == TMRM_TYPE_MULTISET) return 1;
    return 0;
}

/**
 * Checks if an tmrm_object represents an iterator object.
 *
 * @param[in] object The object in question
 * @returns 1 if an object is an iterator object or 0 otherwise.
 */
int tmrm_object_is_iterator(const tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, int, 0);
    if (*object == TMRM_TYPE_ITERATOR) return 1;
    return 0;
}

/**
 * Checks if an tmrm_object represents a tuple object.
 *
 * @param[in] object The object in question
 * @returns 1 if an object is a tuple object or 0 otherwise.
 */
int tmrm_object_is_tuple(const tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, int, 0);
    if (*object == TMRM_TYPE_TUPLE) return 1;
    return 0;
}

/**
 * Casts object into a proxy object. Returns the underlying proxy object
 * or NULL if the the tmrm_object is not of type tmrm_proxy.
 *
 * @param[in] object The object in question
 * @returns a new tmrm_proxy object or NULL if an error occurs or
 *          object has a different type than TMRM_TYPE_PROXY.
 */
tmrm_proxy* tmrm_object_to_proxy(tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, tmrm_object, (tmrm_proxy*)NULL);

    if (*object != TMRM_TYPE_PROXY) return NULL;
    return (tmrm_proxy*)object;
}


/**
 * Casts object into a literal object. Returns the underlying literal object
 * or NULL if the the tmrm_object is not of type tmrm_literal.
 *
 * @param[in] object The object in question
 * @returns a new tmrm_literal object or NULL if an error occurs or
 *          object has a different type than TMRM_TYPE_LITERAL.
 */
tmrm_literal* tmrm_object_to_literal(tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, tmrm_object, (tmrm_literal*)NULL);
    if (*object != TMRM_TYPE_LITERAL) return NULL;
    return (tmrm_literal*)object;
}


/**
 * Casts object into a multiset object. Returns the underlying multiset object
 * or NULL if the the tmrm_object is not of type tmrm_multiset.
 *
 * @param[in] object The object in question
 * @returns a new tmrm_multiset object or NULL if an error occurs or
 *          object has a different type than TMRM_TYPE_MULTISET.
 */
tmrm_multiset* tmrm_object_to_multiset(tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, tmrm_object, (tmrm_multiset*)NULL);
    if (*object != TMRM_TYPE_MULTISET) return NULL;
    return (tmrm_multiset*)object;
}


/**
 * Casts object into a iterator object. Returns the underlying iterator object
 * or NULL if the the tmrm_object is not of type tmrm_iterator.
 *
 * @param[in] object The object in question
 * @returns a new tmrm_iterator object or NULL if an error occurs or
 *          object has a different type than TMRM_TYPE_ITERATOR.
 */
tmrm_iterator* tmrm_object_to_iterator(tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, tmrm_object, (tmrm_iterator*)NULL);
    if (*object != TMRM_TYPE_ITERATOR) return NULL;
    return (tmrm_iterator*)object;
}


/**
 * Casts object into a tuple object. Returns the underlying tuple object
 * or NULL if the the tmrm_object is not of type tmrm_tuple.
 *
 * @param[in] object The object in question
 * @returns a new tmrm_tuple object or NULL if an error occurs or
 *          object has a different type than TMRM_TYPE_TUPLE.
 */
tmrm_tuple* tmrm_object_to_tuple(tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(object, tmrm_object, (tmrm_tuple*)NULL);
    if (*object != TMRM_TYPE_TUPLE) return NULL;
    return (tmrm_tuple*)object;
}


/**
 * Frees a tmrm_object by calling the corresponding free function
 * of the underlying object.
 */
void tmrm_object_free(tmrm_object* object)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN(object, tmrm_object);
    switch (*object) {
        case TMRM_TYPE_PROXY: {
            tmrm_proxy_free(tmrm_object_to_proxy(object));
            break;
        }
        case TMRM_TYPE_LITERAL: {
            tmrm_literal_free(tmrm_object_to_literal(object));
            break;
        }
        case TMRM_TYPE_MULTISET: {
            tmrm_multiset_free(tmrm_object_to_multiset(object));
            break;
        }
        case TMRM_TYPE_ITERATOR: {
            tmrm_iterator_free(tmrm_object_to_iterator(object));
            break;
        }
        case TMRM_TYPE_TUPLE: {
            tmrm_tuple_free(tmrm_object_to_tuple(object));
            break;
        }
        default: break;/* Silence. */
    }
}


/**
 * Removes a proxy from the subject map AND frees the proxy object p.
 * p must not be NULL.
 *
 * @returns 0 on success or a non-zero value on failure.
 */
int
tmrm_proxy_remove(tmrm_proxy* p)
{
    int ret;
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, 1);
    ret = tmrm_storage_proxy_remove(p->subject_map->storage, p);
    if (ret != 0) return 1;

    tmrm_proxy_free(p);
    return 0;
}


/**
 * Checks if the two proxy object p and other reference the
 * same proxy in the subject map. The function assumes that the proxies
 * belong to the same subject map, and comparing proxies from different
 * subject maps results in an undefined result.
 *
 * @returns 1 if the proxies refer to the same object.
 *          0 if the proxies refer to different proxies.
 *         -1 if an error occurs.
 */
int
tmrm_proxy_equals(tmrm_proxy* p, tmrm_proxy* other) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, -1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(other, tmrm_proxy, -1);

    /* FIXME: Create a backend function */
    if (p->label == other->label) {
        return 1;
    }
    return 0;    
}


/**
 * Frees the memory occupied by a proxy. May not be called with a
 * NULL pointer as p.
 */
void
tmrm_proxy_free(/*@only@*/ tmrm_proxy* p)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN(p, tmrm_proxy);

    /* TODO: Reference counting when the copy contructor has been implemented */
    /* FIXME: Free the proxy structure of the storage module */
    TMRM_FREE(tmrm_proxy, p);
}


/**
 * Adds a new property to a proxy. If the proxy p has not been saved to
 * the model before, it is created. p, key and value must not be NULL.
 *
 * PHP-prototype: TMRM_Proxy::addProperty()
 *
 * @returns 0 on success or a non-zero result on failure.
 */
int 
tmrm_proxy_add_property(tmrm_proxy* p, tmrm_proxy* key, tmrm_proxy* value)
{
    int res;
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, 1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(key, tmrm_proxy, 1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(value, tmrm_proxy, 1);

    res = tmrm_storage_add_property(p->subject_map->storage, p, key, value);
    /* TODO: Add something like "if map.settings.auto_update" */
    tmrm_proxy_update(p);
    return res;
}


/**
 * Adds a new literal property to the proxy. If the proxy p has not been
 * saved in the model before, it is created.
 *
 * @param[in,out] p The proxy to be modified
 * @param[in] key Key must be an existing proxy.
 * @param[in] value The literal to be added to p.
 * @returns 0 on success or a non-zero result on failure.
 */
int
tmrm_proxy_add_property_literal(tmrm_proxy* p, tmrm_proxy* key, tmrm_literal* value)
{
    int res;
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, 1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(key, tmrm_proxy, 1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(value, tmrm_literal, 1);

    res = tmrm_storage_add_property_literal(p->subject_map->storage, p, key, value);
    /* TODO: Add something like "if map.settings.auto_update" */
    tmrm_proxy_update(p);
    return res;
}


/**
 * Short cut function: Removes all properties with a given key from a proxy.
 * p and key must not be NULL.
 *
 * PHP-prototype: TMRM_Proxy::removePropertiesByKey()
 *
 * @returns 0 on success or a non-zero value on failure.
 */
int
tmrm_proxy_remove_properties_by_key(tmrm_proxy* p, tmrm_proxy* key)
{
    int res;
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, 1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(key, tmrm_proxy, 1);

    res = tmrm_storage_remove_properties_by_key(p->subject_map->storage, p, key);
    /* TODO: Add something like "if map.settings.auto_update" */
    tmrm_proxy_update(p);
    return res;
}


/**
 * Returns an interator over all properties of a proxy.
 *
 * @returns NULL if an error occurs, or an interator otherwise.
 */
tmrm_iterator*
tmrm_proxy_get_properties(tmrm_proxy* p) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);

    return tmrm_storage_proxy_get_properties(p->subject_map->storage, p);
}


/**
 * Checks if a proxy is being used as a key or value.
 *
 * PHP-prototype: TMRM_Proxy::proxy_is_referenced()
 *
 * @returns 1 if p is used as a key or value, 0 if the proxy is not used, or
 * -1 if an error occurs.
 *
 * @todo implement!
 */
int
tmrm_proxy_is_referenced(tmrm_proxy* p)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, -1);
    // We should be able to provide a default implementation
    // using path functions.

    // TODO: implement
    return -1;
}


/**
 * Retrieves the proxy with the label 'label' and returns a new tmrm_proxy 
 * object.
 * The caller is responsible for freeing the tmrm_proxy object with the
 * tmrm_proxy_free()-method.
 *
 * @returns If no proxy is found or an error occurrs, NULL is returned.
 */
/*@null@*/ tmrm_proxy*
tmrm_proxy_by_label(tmrm_subject_map* map, const char *label)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(map, tmrm_subject_map, (tmrm_proxy*)NULL);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(label, cstring, (tmrm_proxy*)NULL);

    return tmrm_storage_proxy_by_label(map->storage, map, label);
}


/**
 * Returns a multiset with all proxies in the subject map.
 *
 * PHP-prototype: TMRM_SubjectMap::getProxies()
 * 
 * @returns NULL on error.
 * @todo Move to tmrm_subjectmap.c
 */
tmrm_multiset*
tmrm_subject_map_proxies(tmrm_subject_map *map)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(map, tmrm_subject_map,
            NULL);

    return tmrm_multiset_new_from_iterator(map, 
            tmrm_storage_proxies(map->storage, map));
}

/**
 * Returns an iterator over all proxies in the subject map.
 *
 * @returns NULL on error.
 * @todo Move to tmrm_subjectmap.c
 */
tmrm_iterator*
tmrm_subject_map_iterator(tmrm_subject_map *map) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(map, tmrm_subject_map, NULL);

    return tmrm_storage_proxies(map->storage, map);
}


static int
tmrm_yaml_export_subject_map_start(void *context, tmrm_char_t *name) {
    tmrm_yaml_export_context *c;
    yaml_event_t event;
    yaml_version_directive_t version;

    c = (tmrm_yaml_export_context*)context;
    yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    version.major = 1;
    version.minor = 1;
    yaml_document_start_event_initialize(&event, &version, NULL, NULL, 0);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_mapping_start_event_initialize(&event, NULL, NULL, 0, YAML_BLOCK_MAPPING_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_scalar_event_initialize(&event,
            NULL, NULL,
            (yaml_char_t*)"subject_map", -1,
            1, 1, YAML_PLAIN_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_scalar_event_initialize(&event,
            NULL, NULL,
            (yaml_char_t*)name, -1,
            1, 1, YAML_SINGLE_QUOTED_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_scalar_event_initialize(&event,
            NULL, NULL,
            (yaml_char_t*)"proxies", -1,
            1, 1, YAML_PLAIN_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_mapping_start_event_initialize(&event, NULL, NULL, 0, YAML_BLOCK_MAPPING_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
}

static int
tmrm_yaml_export_subject_map_end(void *context) {
    tmrm_yaml_export_context *c;
    yaml_event_t event;

    c = (tmrm_yaml_export_context*)context;
    yaml_mapping_end_event_initialize(&event);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_mapping_end_event_initialize(&event);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_document_end_event_initialize(&event, 0);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_stream_end_event_initialize(&event);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
}

static int
tmrm_yaml_export_proxy_start(void *context, tmrm_char_t *label) {
    tmrm_yaml_export_context *c;
    yaml_event_t event;

    c = (tmrm_yaml_export_context*)context;
    yaml_scalar_event_initialize(&event,
            NULL, NULL,
            (yaml_char_t*)label, -1,
            1, 1, YAML_PLAIN_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_mapping_start_event_initialize(&event, NULL, NULL, 0, YAML_BLOCK_MAPPING_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
}

static int
tmrm_yaml_export_proxy_end(void *context) {
    tmrm_yaml_export_context *c;
    yaml_event_t event;

    c = (tmrm_yaml_export_context*)context;

    yaml_mapping_end_event_initialize(&event);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
}

static int
tmrm_yaml_export_property(void *context, tmrm_char_t *key, tmrm_char_t *value) {
    tmrm_yaml_export_context *c;
    yaml_event_t event;

    c = (tmrm_yaml_export_context*)context;

    yaml_scalar_event_initialize(&event,
            NULL, NULL,
            (yaml_char_t*)key, -1,
            1, 1, YAML_ANY_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_scalar_event_initialize(&event,
            NULL, (yaml_char_t*)"!proxy",
            (yaml_char_t*)value, -1,
            0, 0, YAML_ANY_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
}

static int
tmrm_yaml_export_property_literal(void *context, tmrm_char_t *key,
    tmrm_char_t *value, tmrm_char_t *datatype) {
    tmrm_yaml_export_context *c;
    yaml_event_t event;

    c = (tmrm_yaml_export_context*)context;

    yaml_scalar_event_initialize(&event,
            NULL, NULL,
            (yaml_char_t*)key, -1,
            1, 1, YAML_ANY_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
/*
    FIXME: Encode datatype into export
    yaml_mapping_start_event_initialize(&event, NULL, NULL, 0, YAML_FLOW_MAPPING_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_scalar_event_initialize(&event,
            NULL, NULL,
            (yaml_char_t*)"value", -1,
            1, 1, YAML_PLAIN_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
*/
    yaml_scalar_event_initialize(&event,
            NULL, NULL,
            (yaml_char_t*)value, -1,
            1, 1, YAML_DOUBLE_QUOTED_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
/*
    yaml_scalar_event_initialize(&event,
            NULL, NULL,
            (yaml_char_t*)"datatype", -1,
            1, 1, YAML_PLAIN_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_scalar_event_initialize(&event,
            NULL, NULL,
            (yaml_char_t*)datatype, -1,
            1, 1, YAML_SINGLE_QUOTED_SCALAR_STYLE);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
    yaml_mapping_end_event_initialize(&event);
    if (!yaml_emitter_emit(&c->emitter, &event)) return -1;
*/
}

/**
 * Serializes the subject map m to a file.
 *
 * @returns -1 on failure
 */
int
tmrm_subject_map_export_to_yaml(tmrm_subject_map *map, FILE *fh) {
    tmrm_streaming_handler handler;
    tmrm_yaml_export_context context;
    int res;

    context.fh = fh;
    yaml_emitter_initialize(&context.emitter);
    yaml_emitter_set_output_file(&context.emitter, fh);
    handler.subject_map_start = tmrm_yaml_export_subject_map_start;
    handler.subject_map_end = tmrm_yaml_export_subject_map_end;
    handler.proxy_start = tmrm_yaml_export_proxy_start;
    handler.property = tmrm_yaml_export_property;
    handler.property_literal = tmrm_yaml_export_property_literal;
    handler.proxy_end = tmrm_yaml_export_proxy_end;
    res = tmrm_subject_map_export_streamer(map, &handler, &context);

    yaml_emitter_delete(&context.emitter);
    return res;
}

static int tmrm_subject_map_export_streamer(tmrm_subject_map *map,
    tmrm_streaming_handler *h, void *handler_context) {
    tmrm_proxy *p, *proxy_res, *key;
    tmrm_literal *lit;
    tmrm_iterator *it, *proxy_it;
    tmrm_object *obj = NULL;
    tmrm_object_type object_type;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(map, tmrm_subject_map, -1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(h, tmrm_streaming_handler, -1);

    if (h->subject_map_start) h->subject_map_start(handler_context, (unsigned char*)tmrm_subject_map_name(map));

    it = tmrm_subject_map_iterator(map);
    if (!it) return -1;

    while (!tmrm_iterator_end(it)) {
        obj = tmrm_iterator_get_value(it);
        if (tmrm_object_get_type(obj) != TMRM_TYPE_PROXY) goto error;
        /* FIXME potential memory leak! define how tmrm_object_to_proxy handles
           memory allocation */
        p = tmrm_object_to_proxy(obj);
        if (!p) goto error;
        if (h->proxy_start) h->proxy_start(handler_context, (unsigned char*)tmrm_proxy_label(p));

        /* => Iterator over properties */
        /* Try to list all properties of a proxy */
        proxy_it = tmrm_proxy_get_properties(p);
        if (!proxy_it) goto error;

        while(!tmrm_iterator_end(proxy_it)) {
            /* Get the key of the property: */
            if (!(obj = tmrm_iterator_get_key(proxy_it))) goto error_proxy_it;
            if ((object_type = tmrm_object_get_type(obj)) != TMRM_TYPE_PROXY) {
                tmrm_iterator_free(proxy_it);
                tmrm_iterator_free(it);
                /* FIXME: How can we free value? */
                return -1;
            }
            key = tmrm_object_to_proxy(obj);

            if (!(obj = tmrm_iterator_get_value(proxy_it))) {
                tmrm_proxy_free(key);
                goto error_proxy_it;
            }
            object_type = tmrm_object_get_type(obj);
            if (object_type != TMRM_TYPE_PROXY && object_type != TMRM_TYPE_LITERAL) {
                tmrm_proxy_free(key);
                goto error_proxy_it;
            }
            if (object_type == TMRM_TYPE_PROXY) {
                proxy_res = tmrm_object_to_proxy(obj);
                if (h->property)
                    h->property(handler_context, (unsigned char*)tmrm_proxy_label(key), (unsigned char*)tmrm_proxy_label(proxy_res)); 
                tmrm_proxy_free(proxy_res);
            } else {
                lit = tmrm_object_to_literal(obj);
                if (h->property_literal)
                    h->property_literal(handler_context, (unsigned char*)tmrm_proxy_label(key),
                            (unsigned char*)tmrm_literal_value(lit), (unsigned char*)tmrm_literal_datatype(lit));
                tmrm_literal_free(lit);
            }
            tmrm_proxy_free(key);

            if (tmrm_iterator_next(proxy_it)) goto error_proxy_it;
        }
        if (h->proxy_end) h->proxy_end(handler_context);

        tmrm_iterator_free(proxy_it);
        if (tmrm_iterator_next(it)) goto error;
    }
    tmrm_iterator_free(it);
    if (h->subject_map_end) h->subject_map_end(handler_context);
    return 0;

error_proxy_it:
    tmrm_iterator_free(proxy_it);
error:
    if (obj) tmrm_object_free(obj);
    tmrm_iterator_free(it);
    return -1;
}


/**
 * Parses a subject map from a YAML-file.
 *
 * @returns -1 on failure
 */
int
tmrm_subject_map_import_from_yaml(tmrm_subject_map *map, FILE *fh)
{
    tmrm_streaming_handler handler;
    tmrm_yaml_import_context context;
    int res;

    context.fh = fh;
    yaml_parser_initialize(&context.parser);
    yaml_parser_set_input_file(&context.parser, fh);
    res = tmrm_subject_map_import_streamer(map, &handler, &context);

    yaml_parser_delete(&context.parser);
    return res;
}


/**
 * Parses a subject map from a YAML string.
 *
 * @param[in,out] map The subject map object
 * @param[in] str YAML data
 * @param[in] len length of the source date in bytes
 * @returns -1 on failure
 */
int
tmrm_subject_map_import_from_yaml_string(tmrm_subject_map *map, const unsigned char* str, size_t len)
{
    tmrm_streaming_handler handler;
    tmrm_yaml_import_context context;
    int res;

    yaml_parser_initialize(&context.parser);
    yaml_parser_set_input_string(&context.parser, str, len);
    res = tmrm_subject_map_import_streamer(map, &handler, &context);

    yaml_parser_delete(&context.parser);
    return res;
}



/**
 * Imports a subject map from a YAML file.
 */
static int tmrm_subject_map_import_streamer(tmrm_subject_map *map,
    tmrm_streaming_handler *h, void *handler_context)
{
    yaml_event_t event;
    tmrm_yaml_import_context *c = (tmrm_yaml_import_context*)handler_context;
    int done = 0, state = 0, i;
    tmrm_proxy *proxy = NULL, *key_proxy = NULL, *value_proxy = NULL;
    tmrm_hash* labels;
    char *label;
    tmrm_literal *lit;

    /* FIXME: Add meaningful constants and refactor the transition table */
    typedef enum state_e {
        STATE_INVALID = -1,
        STATE_INIT,
        STATE_1,
        STATE_2,
        STATE_3,
        STATE_4,
        STATE_5,
        STATE_6,
        STATE_7,
        STATE_8,
        STATE_9,
        STATE_END,
        STATE_11 
    } state_t;

    yaml_event_type_t event_types[11] = {
        YAML_NO_EVENT,
        YAML_STREAM_START_EVENT,
        YAML_STREAM_END_EVENT,
        YAML_DOCUMENT_START_EVENT,
        YAML_DOCUMENT_END_EVENT,
        YAML_ALIAS_EVENT,
        YAML_SCALAR_EVENT,
        YAML_SEQUENCE_START_EVENT,
        YAML_SEQUENCE_END_EVENT,
        YAML_MAPPING_START_EVENT,
        YAML_MAPPING_END_EVENT
    };
    char* event_names[11] = {
        "NO_EVENT",
        "STREAM_START",
        "STREAM_END",
        "DOCUMENT_START",
        "DOCUMENT_END",
        "ALIAS",
        "SCALAR",
        "SEQUENCE_START",
        "SEQUENCE_END",
        "MAPPING_START",
        "MAPPING_END"
    };
    /* 
       State transition table for YAML parser events.
       0 is the start state.
       10 is the final state. (TODO: Add meaningful constants)
       -1 means that there is no transition for an input event.

       The columns are the events that represent transitions between states.
       Each row represents a state.
     */
    int transitions[12][10] = {
/* STREAM_ST STR_END DOC_ST DOC_END ALIAS SCALAR SEQ_ST SEQ_END MAP_ST MAP_END*/
/* 0 */ {  0,    10,   0,     10,    -1,    -1,     -1,     -1,     1,     -1},
/* 1 */ {  -1,   -1,  -1,     -1,    -1,     2,     -1,     -1,    -1,     -1},
/* 2 */ {  -1,   -1,  -1,     -1,    -1,     3,     -1,     -1,    -1,     -1},
/* 3 */ {  -1,   -1,  -1,     -1,    -1,     4,     -1,     -1,    -1,     10},
/* 4 */ {  -1,   -1,  -1,     -1,    -1,    -1,     -1,     -1,     5,     -1},
/* 5 */ {  -1,   -1,  -1,     -1,    -1,    11,     -1,     -1,    -1,      7},
/* 6 */ {  -1,   -1,  -1,     -1,    -1,     8,     -1,     -1,    -1,      5},
/* 7 */ {  -1,   -1,  -1,     -1,    -1,    -1,     -1,     -1,    -1,     10},
/* 8 */ {  -1,   -1,  -1,     -1,    -1,     6,      9,     -1,    -1,     -1},
/* 9 */ {  -1,   -1,  -1,     -1,    -1,     9,     -1,      6,    -1,     -1},
/*10 */ {  -1,   10,  -1,     10,    -1,    -1,     -1,     -1,    -1,     -1},
/*11 */ {  -1,   -1,  -1,     -1,    -1,    -1,     -1,     -1,     6,     -1}
    };

    /* We could use an external hash table here since we only store labels and
       not pointers to proxies */
    labels = tmrm_hash_new(map->sms, "memory");
    if (!labels) return -1;
    while (!done)
    {
        if (!yaml_parser_parse(&c->parser, &event))
            goto error_cleanup;

        for(i = 0; i<11 && event_types[i] != event.type; i++) ;
        if (state != -1) {
            /*TMRM_DEBUG5("Current state = %d, event = %d (%s), new state = %d\n",
                    state, i, event_names[i], transitions[state][i-1]);*/
            state = transitions[state][i-1];
        }
        if (state == -1) {
            TMRM_DEBUG4("Unexpected %s in line %d, column %d\n",
                event_names[i], (int)event.start_mark.line,
                (int)event.start_mark.column);
            goto error_cleanup;
        }
        switch (state) {
            case STATE_3: {
                TMRM_DEBUG2(" - found subject map: '%s'\n",
                    (char*)event.data.scalar.value);
                break;
            }
            case STATE_5: {
                if (event.type == YAML_MAPPING_END_EVENT) {
                    if (proxy) {
                        tmrm_proxy_free(proxy);
                        proxy = NULL;
                    }
                    if (key_proxy) {
                        tmrm_proxy_free(key_proxy);
                        key_proxy = value_proxy = NULL;
                    }
                }
                break;
            }
            case STATE_9: {
                if (event.type == YAML_SCALAR_EVENT) {
                    TMRM_DEBUG2(" - found property: '%s'\n",
                            (char*)event.data.scalar.value);
                } else {
                    break;
                }
            }
            case STATE_6: {
                if (event.type == YAML_SCALAR_EVENT) {
                    /* assert(key_proxy); */
                    /* Check if value is proxy or literal */
                    if (event.data.scalar.tag &&
                        !strcmp("!proxy", (char*)event.data.scalar.tag)) {
                        /* assert(value_proxy == NULL); */
                        /* assert(proxy != NULL); */
                        if (!(label = tmrm_hash_get(labels,
                                        (char*)event.data.scalar.value))) {
                            if (strcmp(TMRM_BOTTOM_PROXY_LABEL, (char*)event.data.scalar.value)) {
                                value_proxy = tmrm_proxy_new(map);
                            } else {
                                value_proxy = tmrm_subject_map_bottom(map);
                            }
                            tmrm_hash_put_strings(labels,
                                    (char*)event.data.scalar.value,
                                    (char*)tmrm_proxy_label(value_proxy));
                        }
                        if (!value_proxy) {
                            value_proxy = tmrm_proxy_by_label(map, label);
                            if (!value_proxy) goto error_cleanup;
                        }
                        if (label) {
                            TMRM_FREE(cstring, label);
                            label = NULL;
                        }
                        tmrm_proxy_add_property(proxy, key_proxy, value_proxy);
                        tmrm_proxy_free(value_proxy);
                    } else {
                        /* property is a literal */
                        lit = tmrm_literal_new((tmrm_char_t*)event.data.scalar.value,
                            (tmrm_char_t*)TMRM_XMLSCHEMA_STRING);
                        tmrm_proxy_add_property_literal(proxy, key_proxy, lit);
                        
                        tmrm_literal_free(lit);
                        lit = NULL;
                    }
                }
                break;
            }
            case STATE_8: {
                if (key_proxy) {
                    tmrm_proxy_free(key_proxy);
                    key_proxy = value_proxy = NULL;
                }
                if (!(label = tmrm_hash_get(labels,
                            (char*)event.data.scalar.value))) {
                    if (strcmp(TMRM_BOTTOM_PROXY_LABEL, (char*)event.data.scalar.value)) {
                        key_proxy = tmrm_proxy_new(map);
                    } else {
                        key_proxy = tmrm_subject_map_bottom(map);
                    }
                    tmrm_hash_put_strings(labels, (char*)event.data.scalar.value,
                            (char*)tmrm_proxy_label(key_proxy));
                }
                if (!key_proxy) {
                    key_proxy = tmrm_proxy_by_label(map, label);
                    if (!key_proxy) goto error_cleanup;
                }
                /* assert(key_proxy != NULL); */
                if (label) {
                    TMRM_FREE(cstring, label);
                    label = NULL;
                }
                break;
            }
            case STATE_11: {
                /*TMRM_DEBUG2(" - found proxy label: '%s'\n",
                    (unsigned char*)event.data.scalar.value);*/
                /* assert(proxy == NULL); */
                /* Check if the proxy exists */
                if (!(label = tmrm_hash_get(labels,
                            (char*)event.data.scalar.value))) {
                    /* Recognise the special label 'libtmrm:bottom' for
                       bootstrapping purposes. */
                    if (strcmp(TMRM_BOTTOM_PROXY_LABEL, (char*)event.data.scalar.value)) {
                        proxy = tmrm_proxy_new(map);
                    } else {
                        proxy = tmrm_subject_map_bottom(map);
                    }
                    tmrm_hash_put_strings(labels, (char*)event.data.scalar.value,
                        (char*)tmrm_proxy_label(proxy));
                }
                if (!proxy) {
                    proxy = tmrm_proxy_by_label(map, label);
                }
                if (label) {
                    TMRM_FREE(cstring, label);
                    label = NULL;
                }
                /* assert(proxy); */
                /* labels[label] = proxy; */
                break;
            }
            case STATE_END: done = 1; break;
            default: break;
        }
    }
    tmrm_hash_free(labels);

    return 0;
error_cleanup:
    if (labels) tmrm_hash_free(labels);
    if (proxy) tmrm_proxy_free(proxy);
    if (key_proxy) tmrm_proxy_free(key_proxy);
    if (value_proxy) tmrm_proxy_free(value_proxy);
    if (lit) tmrm_literal_free(lit);
    if (label) TMRM_FREE(cstring, label);
    return -1;
}


/**
 * Adds the proxy type as a type to p. p and type must not be NULL.
 *
 * PHP-prototype: TMRM_Proxy::addType().
 *
 * @returns 0 on success or a non-zero value on failure.
 */
int
tmrm_proxy_add_type(tmrm_proxy* p, tmrm_proxy* type)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, 1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(type, tmrm_proxy, 1);

    return tmrm_storage_proxy_add_type(p->subject_map->storage, p, type);
}


/**
 * Adds superclass to the proxy p. p and superclass must not be NULL.
 *
 * PHP-prototype: TMRM_Proxy::addSuperclass()
 *
 * @returns 0 on success or a non-zero value on failure.
 */
int
tmrm_proxy_add_superclass(tmrm_proxy* p, tmrm_proxy* superclass)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, 1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(superclass, tmrm_proxy, 1);

    return tmrm_storage_proxy_add_superclass(p->subject_map->storage, p, superclass);
}


/**
 * Returns a list of proxy-objects that are part of an superclass-
 * subclass relation where the proxy 'p' is the superclass.
 *
 * @returns NULL on failure.
 * @todo Decide if this function should be for internal use only.
 */
/*@null@*/ tmrm_iterator*
tmrm_proxy_direct_subclasses(tmrm_proxy* p) 
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);
    return tmrm_storage_proxy_direct_subclasses(p->subject_map->storage, p);
}


/**
 * Returns a list of all proxy objects that are a subclass of the
 * proxy p.
 *
 * @todo Could be optimized by keeping a sorted list of proxies
 * @returns NULL on failure.
 */
/*@null@*/ tmrm_list*
tmrm_proxy_subclasses(tmrm_proxy* p)
{
    tmrm_list *subclasses;
    tmrm_list *items_to_check;
    tmrm_list_elmt *elem;
    tmrm_proxy *self;
    tmrm_object *subclass, *obj, *current_obj, *obj_from_list;
    tmrm_iterator *subclass_it;
    int found_element = 0;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);

    subclasses = tmrm_list_new((tmrm_list_free_handler*)tmrm_proxy_free);
    if (!subclasses) {
        TMRM_DEBUG1("Could not create subclasses\n");
        return (tmrm_list*)NULL;
    }

    /* Note that we don't specify a function to free list items here, 
       since all items in the items_to_check list also are referenced
       from the subclasses list */
    items_to_check = tmrm_list_new(NULL);
    if (!items_to_check) {
        TMRM_DEBUG1("Could not create items_to_check\n");
        tmrm_list_free(subclasses);
        return (tmrm_list*)NULL;
    }

    /* p is always a subclass of itself, so we add a copy of it to the list */
    self = tmrm_proxy_clone(p);
    if (self == NULL) {
        tmrm_list_free(subclasses);
        tmrm_list_free(items_to_check);
        TMRM_DEBUG1("Could not clone proxy p\n");
        return (tmrm_list*)NULL;
    }
    if (!(obj = tmrm_proxy_to_object(self)) ||
        tmrm_list_ins_next(subclasses, NULL, obj) ||
        tmrm_list_ins_next(items_to_check, NULL, obj)) {
        tmrm_proxy_free(self);
        tmrm_list_free(subclasses);
        tmrm_list_free(items_to_check);
        TMRM_DEBUG1("Could not add proxy p to lists\n");
        return (tmrm_list*)NULL;
    }

    while(tmrm_list_size(items_to_check) > 0) {
        /*TMRM_DEBUG2("list_size(items_to_check) = %d\n", tmrm_list_size(items_to_check));*/
        if (tmrm_list_rem_next(items_to_check, NULL, &current_obj) > 0)
            break;
        TMRM_ASSERT(tmrm_object_get_type(current_obj) == TMRM_TYPE_PROXY);
        
        subclass_it = tmrm_proxy_direct_subclasses(tmrm_object_to_proxy(current_obj));
        if (subclass_it == NULL) continue;
        while (!tmrm_iterator_end(subclass_it)) {
            subclass = tmrm_iterator_get_object(subclass_it);
            TMRM_ASSERT(tmrm_object_get_type(subclass) == TMRM_TYPE_PROXY);
            found_element = 0;
            elem = tmrm_list_head(subclasses);
            /*TMRM_DEBUG2("Checking if proxy '%s' exists in list\n",
                 tmrm_proxy_label(tmrm_object_to_proxy(subclass)));*/
            do {
                obj_from_list = tmrm_list_data(elem);
                TMRM_ASSERT(tmrm_object_get_type(obj_from_list) == TMRM_TYPE_PROXY);
                /*TMRM_DEBUG2("   => comparing with proxy '%s'\n",
                     tmrm_proxy_label(tmrm_object_to_proxy(obj_from_list)));*/
                if (tmrm_proxy_equals(tmrm_object_to_proxy(subclass),
                    tmrm_object_to_proxy(obj_from_list)) == 1) {
                    /*TMRM_DEBUG1("----> proxy did exists. NOT ADDING\n");*/
                    found_element = 1;
                    break;
                }
            } while ((elem = tmrm_list_next(elem)) != NULL);
            if (found_element == 0) {
                /*TMRM_DEBUG1("----> proxy did not exist in list of subclasses. Adding\n");*/
                tmrm_list_ins_next(subclasses, NULL, subclass);
                tmrm_list_ins_next(items_to_check, NULL, subclass);
            } else {
                tmrm_object_free(subclass);
            }
            if (tmrm_iterator_next(subclass_it) > 0) break;
        }
        tmrm_iterator_free(subclass_it);
    }
    tmrm_list_free(items_to_check);
    return subclasses;
    /*
    PSEUDO-code:

    subclasses = tmrm_list_new();
    tmrm_append_list(subclasses, p);

    items_to_check = tmrm_list_new();
    tmrm_list_append(items_to_check, p);
    
    while (tmrm_list_size(items_to_check) > 0) {
        item = tmrm_list_pop(items_to_check);
        subclasses = tmrm_proxy_direct_subclasses(item);
        foreach(subclasses as subclass) {
            if (!tmrm_list_has_item(subclasses, subclass)) {
                tmrm_list_append(subclasses, subclass);
                tmrm_list_append(items_to_check, subclass);
            }
            // we have already seen this subclass
        }
    }
    return subclasses;
    */
}


/**
 * Returns a list of all proxy objects that are a superclass
 * of the proxy p.
 *
 * @todo Could be optimized by keeping a sorted list of proxies
 * @returns NULL on failure.
 */
/*@null@*/ tmrm_list*
tmrm_proxy_superclasses(tmrm_proxy* p)
{
    tmrm_list *superclasses;
    tmrm_list *items_to_check;
    tmrm_list_elmt *elem;
    tmrm_proxy *self;
    tmrm_object *current_obj, *superclass, *obj_from_list, *obj;
    tmrm_iterator *superclass_it;
    int found_element = 0;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);

    superclasses = tmrm_list_new((tmrm_list_free_handler*)tmrm_object_free);
    if (!superclasses) { return NULL; }

    items_to_check = tmrm_list_new(NULL);
    if (!items_to_check) {
        tmrm_list_free(superclasses);
        return NULL;
    }

    /* p is always a subclass of itself, so we add a copy of it to the list */
    self = tmrm_proxy_clone(p);
    if (self == NULL) {
        tmrm_list_free(superclasses);
        tmrm_list_free(items_to_check);
        return NULL;
    }
    if (!(obj = tmrm_proxy_to_object(self)) || 
        tmrm_list_ins_next(superclasses, NULL, obj) ||
        tmrm_list_ins_next(items_to_check, NULL, obj)) {
        tmrm_object_free(obj);
        tmrm_list_free(superclasses);
        tmrm_list_free(items_to_check);
        return NULL;
    }

    while(tmrm_list_size(items_to_check) > 0) {
        if (tmrm_list_rem_next(items_to_check, NULL, &current_obj))
            break;
        superclass_it = tmrm_proxy_direct_superclasses(
            tmrm_object_to_proxy(current_obj));
        if (superclass_it == NULL) continue;
        while (!tmrm_iterator_end(superclass_it)) {
            superclass = tmrm_iterator_get_object(superclass_it);
            TMRM_ASSERT(tmrm_object_get_type(superclass) == TMRM_TYPE_PROXY);
            found_element = 0;
            elem = tmrm_list_head(superclasses);
            do {
                obj_from_list = tmrm_list_data(elem);
                TMRM_ASSERT(tmrm_object_get_type(obj_from_list) == TMRM_TYPE_PROXY);
                if (tmrm_proxy_equals(
                    tmrm_object_to_proxy(superclass),
                    tmrm_object_to_proxy(obj_from_list)) == 1) {
                    found_element = 1;
                    break;
                }
            } while ((elem = tmrm_list_next(elem)) != NULL);
            if (found_element == 0) {
                tmrm_list_ins_next(superclasses, NULL, superclass);
                tmrm_list_ins_next(items_to_check, NULL, superclass);
            } else {
                tmrm_object_free(superclass);
            }
            if (tmrm_iterator_next(superclass_it)) break;
        }
        tmrm_iterator_free(superclass_it);
    }
    tmrm_list_free(items_to_check);
    return superclasses;
}


/**
 * Returns a list of proxy object that are a type of the
 * proxy p.
 *
 * PHP-prototype: TMRM_Proxy::getDirectTypes()
 *
 * @returns NULL on failure.
 */
/*@null@*/ tmrm_iterator*
tmrm_proxy_direct_types(tmrm_proxy* p)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);
    return tmrm_storage_proxy_direct_types(p->subject_map->storage, p);
}


/**
 * Returns a list of proxy object that are a direct instance of the
 * proxy p.
 *
 * @returns NULL on failure.
 */
/*@null@*/ tmrm_iterator*
tmrm_proxy_direct_instances(tmrm_proxy* p)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);
    return tmrm_storage_proxy_direct_instances(p->subject_map->storage, p);
}


/**
 * Returns a list of proxy-objects that are part of an 
 * superclass-subclass relation where the proxy p is the instance.
 * 
 * PHP-prototype: TMRM_Proxy::getDirectSuperclasses()
 *
 * @returns NULL on failure.
 * @todo Decide if this function should be for internal use only.
 */
/*@null@*/ tmrm_iterator*
tmrm_proxy_direct_superclasses(tmrm_proxy* p)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, NULL);
    return tmrm_storage_proxy_direct_superclasses(p->subject_map->storage, p);
}


/**
 * Returns 1 if p is a subclass of class_p, 0 if not, or
 * a a value < 0 if an error occurrs.
 */
int
tmrm_proxy_sub(tmrm_proxy* p, tmrm_proxy* class)
{
    tmrm_list *subclasses;
    tmrm_object *current;
    tmrm_proxy *current_p;
    int found = 0;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, -1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(class, tmrm_proxy, -1);

    // A proxy is always a subclass of itself
    if (tmrm_proxy_equals(p, class) == 1) {
        return 1;
    }
    subclasses = tmrm_proxy_subclasses(p);
    if (subclasses == NULL) {
        return -1;
    }
    /* Loop through all subclasses and return 1 if one of the elements 
       matches p: */
    while (tmrm_list_size(subclasses) > 0) {
        if (tmrm_list_rem_next(subclasses, NULL, &current) == 0) {
            current_p = tmrm_object_to_proxy(current);
            if (current_p == NULL) {
                tmrm_list_free(subclasses);
                return -1;
            }
            if (tmrm_proxy_equals(current_p, p) == 1) found = 1;
            tmrm_proxy_free(current_p);
        }
    }
    tmrm_list_free(subclasses);
    return found;
}


/**
 * Returns 1 if p is an instance of type, 0 if not, or
 * a a value < 0 if an error occurrs.
 */
int
tmrm_proxy_isa(tmrm_proxy* p, tmrm_proxy* type)
{
    tmrm_iterator *types_it;
    tmrm_list *subclasses;
    tmrm_list *types;
    tmrm_object *current_obj = NULL;
    tmrm_proxy *current_proxy;
    tmrm_object *current_type;
    tmrm_list_elmt *elem;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_proxy, -1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(type, tmrm_proxy, -1);

    /* An instance is never a type of itself */
    if (tmrm_proxy_equals(p, type) == 1) {
        return 0;
    }
    /*
       The isa relationship is supposed to be non-reﬂexive, i.e. x isa m x
       for no x ∈ m, so that no proxy can be an instance of itself.
       Additionally, whenever a proxy a is an instance of another c, then
       a is an instance of any superclass of c: if x isam c and c subm c′, 
       then x isa c′ is true.

       => find all direct types of p

       => find all subclasses of type
          foreach subclass s, check if s is part of the direct types of p
    */

    /* Create a list over direct types of p */
    types_it = tmrm_proxy_direct_types(p);
    if (types_it == NULL) return -1;

    types = tmrm_list_from_iterator(types_it, tmrm_object_free);
    if (types == NULL) return -1;
    tmrm_iterator_free(types_it);

    /* Loop through all subclasses of type */
    subclasses = tmrm_proxy_subclasses(type);
    if (subclasses == NULL) {
        tmrm_list_free(types);
        return -1;
    }
    /* Loop through all subclasses and return 1 if one of the elements 
       matches p: */
    while (tmrm_list_size(subclasses) > 0) {
        if (tmrm_list_rem_next(subclasses, NULL, &current_obj) == 0) {
            /* Check if p is an instance of current */
            elem = tmrm_list_head(types);
            while (elem != NULL) {
                current_type = tmrm_list_data(elem);
                TMRM_ASSERT(tmrm_object_get_type(current_obj) == TMRM_TYPE_PROXY);
                /* TMRM_DEBUG3("Checking if proxy '%s' is equal '%s'\n", 
                   tmrm_proxy_label(current_proxy), tmrm_proxy_label(current));*/
                if (tmrm_proxy_equals(
                    tmrm_object_to_proxy(current_type),
                    tmrm_object_to_proxy(current_obj))) {
                    tmrm_object_free(current_obj);
                    tmrm_list_free(subclasses);
                    return 1;
                }
                elem = tmrm_list_next(elem);
            }
            if (current_obj != NULL) tmrm_object_free(current_obj);
        }
    }
    tmrm_list_free(subclasses);
    tmrm_list_free(types);
    return 0;
}


