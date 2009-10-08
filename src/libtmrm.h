/*
 * libtmrm.h - libtmrm pubic API
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

#ifndef LIBTMRM_H
#define LIBTMRM_H


/**
 * @mainpage libtmrm - A C implementation of the Topic Maps Reference Model (ISO 13250-5)
 *
 * @section intro_sec Introduction
 *
 * This is the introduction. Take a look at the Modules page.
 *
 * @section install_sec Installation
 *
 * @subsection step1 Step 1: Opening the box
 *  
 * etc...
 */

/**
 * @file libtmrm.h
 * @brief libtmrm public API.
 * TODO: Detailed description of the library.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libtmrm_public libtmrm public routines
 * @{
 * @}
 */
#include <stdio.h>

#define TMRM_API


/**
 * A list over public types.
 * @defgroup tmrm_types Public types
 * @ingroup libtmrm_public 
 * @{
 */

typedef unsigned char tmrm_char_t;
typedef struct tmrm_subject_map_sphere_s tmrm_subject_map_sphere;
typedef struct tmrm_subject_map_s tmrm_subject_map;
typedef struct tmrm_hash_s tmrm_hash;
typedef struct tmrm_hash_cursor_s tmrm_hash_cursor;

/** 
 * Represents a tmrm object: tmrm_proxy, tmrm_literal, or tmrm_multiset. 
 */
enum tmrm_object_e {
  TMRM_TYPE_UNKNOWN  = 0,
  TMRM_TYPE_PROXY    = 1,
  TMRM_TYPE_LITERAL  = 2,
  TMRM_TYPE_MULTISET = 3,
  TMRM_TYPE_ITERATOR = 4,
  TMRM_TYPE_TUPLE    = 5
};

typedef enum tmrm_object_e tmrm_object;
typedef enum tmrm_object_e tmrm_object_type;

typedef struct tmrm_proxy_s tmrm_proxy;
typedef struct tmrm_literal_s tmrm_literal;
typedef struct tmrm_multiset_s tmrm_multiset;
typedef struct tmrm_iterator_s tmrm_iterator;

typedef enum {
    TMRM_ITERATOR_GET_METHOD_GET_OBJECT = 0,
    TMRM_ITERATOR_GET_METHOD_GET_CONTEXT = 1,
    TMRM_ITERATOR_GET_METHOD_GET_KEY = 2,
    TMRM_ITERATOR_GET_METHOD_GET_VALUE = 3
} tmrm_iterator_flag;

typedef struct tmrm_tuple_s tmrm_tuple;

typedef struct tmrm_storage_s tmrm_storage;
typedef struct tmrm_storage_factory_s tmrm_storage_factory;

/** Many bad things could happen in the subject map sphere. */
typedef enum tmrm_error_type_e {
    /** No error is produced. */
    TMRM_NO_ERROR,

    /** Cannot allocate or reallocate a block of memory. */
    TMRM_MEMORY_ERROR,

    /** Error in the storage backend */
    TMRM_STORAGE_ERROR,
    /** Something's wrong with the ontology */
    TMRM_MODEL_ERROR
} tmrm_error_type_t;


/* tmrm_list.h depends on tmrm_iterator */
#include <tmrm_list.h>
#include <tmrm_tuple.h>


/**
 * @}
 * tmrm_object is the common abstract base class for tmrm_proxy, tmrm_multiset
 * and tmrm_literal. It is used as a placeholder in path functions which can
 * take any of these objects as a parameter.
 * 
 * @defgroup tmrm_object tmrm_object
 * @ingroup libtmrm_public
 * @{
 */

/* Casts the proxy p into a tmrm_object. */
tmrm_object* tmrm_proxy_to_object(tmrm_proxy* p);

/* Casts the literal lit into a tmrm_object. */
tmrm_object* tmrm_literal_to_object(tmrm_literal* lit);

/* Casts the multiset ms into a tmrm_object. */
tmrm_object* tmrm_multiset_to_object(tmrm_multiset* ms);

/* Casts the iterator it into a tmrm_object. */
tmrm_object* tmrm_iterator_to_object(tmrm_iterator* ms);

/* Casts the tuple t into a tmrm_object. */
tmrm_object* tmrm_tuple_to_object(tmrm_tuple* t);


/* Returns the type of a tmrm_object. */
tmrm_object_type tmrm_object_get_type(const tmrm_object* object);


/* Returns 1 if an object is a proxy object. */
int tmrm_object_is_proxy(const tmrm_object* object);

/* Returns 1 if an object is a literal object. */
int tmrm_object_is_literal(const tmrm_object* object);

/* Returns 1 if an object is a multiset object. */
int tmrm_object_is_multiset(const tmrm_object* object);

/* Returns 1 if an object is an iterator object. */
int tmrm_object_is_iterator(const tmrm_object* object);


/* Casts object into a proxy object. */
tmrm_proxy* tmrm_object_to_proxy(tmrm_object* object);

/* Cases object into a literal object. */
tmrm_literal* tmrm_object_to_literal(tmrm_object* object);

/* Cases object into a multiset object. */
tmrm_multiset* tmrm_object_to_multiset(tmrm_object* object);

/* Cases object into an iterator object. */
tmrm_iterator* tmrm_object_to_iterator(tmrm_object* object);

/* Cases object into a tuple object. */
tmrm_tuple* tmrm_object_to_tuple(tmrm_object* object);

/* Frees a tmrm_object by calling the corresponding free function
   of the underlying object. */
void tmrm_object_free(tmrm_object* object);


/**
 * @}
 * The subject map sphere
 * 
 * @defgroup tmrm_subject_map_sphere tmrm_subject_map_sphere
 * @ingroup libtmrm_public
 * @{
 */


/* Contructor: */
/*@null@*/ tmrm_subject_map_sphere* tmrm_subject_map_sphere_new(void);


tmrm_error_type_t tmrm_subject_map_sphere_last_error(tmrm_subject_map_sphere *sms);

const char* tmrm_subject_map_sphere_last_error_string(tmrm_subject_map_sphere *sms);


/* Destructor: */
void tmrm_subject_map_sphere_free(/*@only@*/ tmrm_subject_map_sphere* sms);


/**
 * @}
 * tmrm_subject_map respresents an instance of a subject map.
 * @defgroup tmrm_subject_map tmrm_subject_map
 * @ingroup libtmrm_public
 * @{
 */


/* Constructor */
/*@null@*/ tmrm_subject_map* tmrm_subject_map_new(tmrm_subject_map_sphere* sms,
                                       tmrm_storage* storage, const char* label);


/* Returns an iterator over all proxies in the subject map. */
tmrm_iterator* tmrm_subject_map_iterator(tmrm_subject_map *map);


/* Returns a multiset with all proxies in the subject map. */
tmrm_multiset* tmrm_subject_map_proxies(tmrm_subject_map *map);


/* Merges all equal proxies in the subject map until the subject map is
   fully merged. */
int tmrm_subject_map_merge(tmrm_subject_map *map);


/* Serializes the subject map into a YAML file */
int tmrm_subject_map_export_to_yaml(tmrm_subject_map *map, FILE *fh);


/* All proxies in which the proxies are the value for a particular key. */
/* tmrm_status tmrm_subject_map_is_value_by_key(tmrm_subject_map* map, tmrm_multiset* proxies,
    tmrm_proxy* key, tmtm_iterator** it); */
/*@null@*/ tmrm_multiset* tmrm_subject_map_is_value_by_key(tmrm_multiset* proxies,
        tmrm_proxy* key);


/* Returns the symbolic name of a subject map. */
/*@null@*/ const char* tmrm_subject_map_name(tmrm_subject_map* map);


/* Returns the bottom proxy of the subject map. */
/*@null@*/ tmrm_proxy* tmrm_subject_map_bottom(tmrm_subject_map* map);


/* Parses a subject map from a YAML-file.*/
int tmrm_subject_map_import_from_yaml(tmrm_subject_map *map, FILE *fh);


/* Parses a subject map from a YAML string.*/
int tmrm_subject_map_import_from_yaml_string(tmrm_subject_map *map,
        const unsigned char* str, size_t len);

/* Destructor: */
void tmrm_subject_map_free(tmrm_subject_map* m);


/* 
tmrm_iterator* tmrm_proxies_by_properties(tmrm_subject_map* map,
    tmrm_list* properties); => TMRM_SubjectMap::getProxiesByProperties()

The above function should maybe return an iterator (as there may be many
matching proxies)

tmrm_subject_map_sphere* tmrm_subject_map_sphere(tmrm_subject_map* map);
    => TMRM_SubjectMap::getWorld()

Path language:
--------------

 * Notation: "PROXY <- KEY *"
 * PHP-prototype: TMRM_SubjectMap::isValueByKeyWithSubtyping()
tmrm_multiset* tmrm_subject_map_is_value_by_key_star(tmrm_multiset* proxies,
    tmrm_proxy* key);

 * Finds all keys (in all proxies in the map) where the proxy is the value
 * for it.
 *
 * Notation: "PROXY /"
 *
 * TODO: Value can be tmrm_proxy, tmrm_literal, or tmrm_list/tmrm_multiset
 * => tmrm_multiset* tmrm_subject_map_keys_by_value(tmrm_object* object)
 * PHP-prototype: TMRM_SubjectMap::keysByValue()
Internal function:
TODO: Maybe tmrm_proxy_new_from_id(...) => Yes!
tmrm_proxy* tmrm_proxy_init_from_id(tmrm_subject_map* map, int id);

TODO: Do we need this function?
Internal function:
 * Creates a hash value that represents the proxy including
 * its properties in a way that proxies with equal properties
 * always hav the same hash value. The label is not taken
 * into account.
int tmrm_proxy_recalculate_hash(tmrm_proxy* p);

 * PHP-prototype: TMRM_Proxy::setLabel()
int tmrm_proxy_set_label(tmrm_proxy* p, char* label);

 * PHP-prototype: TMRM_Proxy::getSubjectMap()
tmrm_subject_map* tmrm_proxy_subject_map(tmrm_proxy* p);
 Low priority since we have proxy_remove_properties_by_key: Removes a
   property for a proxy.
   int tmrm_proxy_remove_property(tmrm_proxy* p, tmrm_proxy* key,
                                  tmrm_proxy* value);
   int tmrm_proxy_remove_property_literal(tmrm_proxy* p, tmrm_proxy* key,
                                          const unsigned char* value);
 */


/**
 * @}
 * An object that represents a proxy in a subject map. The proxy belongs to
 * the same subject map in its whole livespan. A single proxy in the subject
 * map can be represented by several tmrm_proxy objects.
 *
 * @defgroup tmrm_proxy tmrm_proxy
 * @ingroup libtmrm_public
 * @{
 */


/* Constructor */
/*@null@*/ tmrm_proxy* tmrm_proxy_new(tmrm_subject_map* m);


/* Retrieves the proxy with the label 'label' and returns a new tmrm_proxy 
 * object. */
/*@null@*/ tmrm_proxy* tmrm_proxy_by_label(tmrm_subject_map* map,
                                           const char *label);


/* Copy constructor: */
/*@null@*/ tmrm_proxy* tmrm_proxy_clone(tmrm_proxy* p);


/* Finishes a series of update operations on a proxy. */
int tmrm_proxy_update(tmrm_proxy* p);


/* Returns the label of the proxy p. */
/*@null@*/ const char* tmrm_proxy_label(tmrm_proxy* p);


/* Returns a multiset with all local keys of p. */
tmrm_multiset* tmrm_proxy_keys(tmrm_proxy *p);


/* Finds all keys (in all proxies in the map) where the proxy is the value
   for it. */
/*@null@*/ tmrm_multiset* tmrm_proxy_keys_by_value(tmrm_proxy* p);


/* Returns a list of values behind a particular key for a given proxy. */
/*@null@*/ tmrm_multiset* tmrm_proxy_values_by_key(tmrm_proxy* proxy,
    tmrm_proxy* key);


/* Returns a list of values behind a particular key for a given proxy by
   honoring the subclasses of the key. (_t stands for 'transitive') */
tmrm_multiset* tmrm_proxy_values_by_key_t(tmrm_proxy* p,
    tmrm_proxy* key);


/* All proxies in which the proxy is the value for a particular key. */
/*@null@*/ tmrm_multiset* tmrm_proxy_is_value_by_key(tmrm_proxy* p,
        tmrm_proxy* key);


/* Checks if a proxy is being used as a key or value. */
int tmrm_proxy_is_referenced(tmrm_proxy* p);


/* Removes a proxy from the subject map AND frees the proxy object p. */
int tmrm_proxy_remove(tmrm_proxy* p);


/* Checks if the two proxy object p and other reference the
   same proxy in the subject map. */
int tmrm_proxy_equals(tmrm_proxy* p, tmrm_proxy* other);


/* Adds a new property to a proxy. */
int tmrm_proxy_add_property(tmrm_proxy* p, tmrm_proxy* key, tmrm_proxy* value);


/* Adds a new literal property to the proxy. */
int tmrm_proxy_add_property_literal(tmrm_proxy* p, tmrm_proxy* key,
    tmrm_literal* value);


/* Removes all properties with a given key from a proxy. */
int tmrm_proxy_remove_properties_by_key(tmrm_proxy* p, tmrm_proxy* key);


/* Returns an interator over all properties of a proxy */
tmrm_iterator* tmrm_proxy_get_properties(tmrm_proxy* p);


/* Adds the proxy type as a type to p. */
int tmrm_proxy_add_type(tmrm_proxy* p, tmrm_proxy* type);

 
/* Adds superclass to the proxy p. */
int tmrm_proxy_add_superclass(tmrm_proxy* p, tmrm_proxy* superclass);


/* Returns a list of all proxy objects that are a subclass of the
   proxy p. */
/*@null@*/ tmrm_list* tmrm_proxy_subclasses(tmrm_proxy* p);


/* Returns a list of all proxy objects that are a superclass
   of the proxy p. */
/*@null@*/ tmrm_list* tmrm_proxy_superclasses(tmrm_proxy* p);


/* Returns a list of proxy objects that are a type of the proxy p. */
/*@null@*/ tmrm_iterator* tmrm_proxy_direct_types(tmrm_proxy* p);


/* Returns a list of proxy objects that are a direct instance of */
/*@null@*/ tmrm_iterator* tmrm_proxy_direct_instances(tmrm_proxy* p);


/* Returns the proxy-objects that are direct subclasses of p */
/*@null@*/ tmrm_iterator* tmrm_proxy_direct_subclasses(tmrm_proxy* p);


/* Returns the proxy-objects that are direct superclasses of p */
/*@null@*/ tmrm_iterator* tmrm_proxy_direct_superclasses(tmrm_proxy* p);


/* Checks if p is a subclass of class_p. */
int tmrm_proxy_sub(tmrm_proxy* p, tmrm_proxy* class_p);


/* Checks if p is an instance of type. */
int tmrm_proxy_isa(tmrm_proxy* p, tmrm_proxy* type);


/* Destructor: */
void tmrm_proxy_free(/*@only@*/ tmrm_proxy* p);


/**
 * @}
 * tmrm_literal represents literal values of properties. A literal consists
 * of a datatype and the value itself.
 *
 * @defgroup tmrm_literal tmrm_literal
 * @ingroup libtmrm_public
 * @{
 */

/* Constructor: */
/*@null@*/ tmrm_literal* tmrm_literal_new(const tmrm_char_t* value, const tmrm_char_t* datatype);


/* Returns the datatype of a literal. */
const tmrm_char_t* tmrm_literal_datatype(const tmrm_literal* lit);


/* Returns the value of a literal. */
const tmrm_char_t* tmrm_literal_value(const tmrm_literal* lit);


/* All proxies in which the literal is the value for a particular key. */
/*@null@*/ tmrm_multiset* tmrm_literal_is_value_by_key(tmrm_literal* lit,
        tmrm_proxy* key);


/* Finds all keys (in all proxies in the map) where the literal is the value
   for it. */
/*@null@*/ tmrm_multiset* tmrm_literal_keys_by_value(tmrm_literal* lit,
    tmrm_subject_map* map);


/**
 * TODO: We should probably add more functions for conversion from and to other
 * types:
 * int tmrm_literal_value_integer(const tmrm_literal* lit); 
 * float tmrm_literal_value_integer(const tmrm_literal* lit); 
 * [...]
 */

/* Destructor: */
void tmrm_literal_free(tmrm_literal* lit);


/**
 * @}
 * tmrm_multiset is a set that allows duplicate elements.
 *
 * @defgroup tmrm_multiset tmrm_multiset
 * @ingroup libtmrm_public
 * @{
 */

/* Constructor. */
/*@null@*/ tmrm_multiset* tmrm_multiset_new(tmrm_subject_map *map);

/* 

tmrm_multiset* tmrm_multiset* tmrm_multiset_new(tmrm_subject_map *map, int (*match)(const void *key1, const void *key2), 
   void (*destroy)(void *data));
*/

/* Constructor. */
/*@null@*/ tmrm_multiset* tmrm_multiset_new_from_iterator(tmrm_subject_map *map,
    tmrm_iterator *it);

/* Copy constructor. */
tmrm_multiset* tmrm_multiset_clone(tmrm_multiset *ms);

/* Inserts a new element into the set. */
int tmrm_multiset_insert(tmrm_multiset *ms, const tmrm_object *data);

/*
Removes all data elements that matches from the set. Returns the count of elements removed.
int tmrm_multiset_remove(tmrm_multiset *set, void **data);
*/


/* Adds all elements of ms2 to the multi set ms. */
int tmrm_multiset_add(tmrm_multiset *ms, const tmrm_multiset *ms2);

/*
int tmrm_multiset_union(tmrm_multiset *setu, const tmrm_multiset *set1, const tmrm_multiset *set2);
int tmrm_multiset_intersection(tmrm_multiset *seti, const tmrm_multiset *set1, const tmrm_multiset *set2);
int tmrm_multiset_difference(tmrm_multiset *setd, const tmrm_multiset *set1, const tmrm_multiset *set2);
int tmrm_multiset_is_member(const tmrm_multiset *set, const void *data);
int tmrm_multiset_is_subset(const tmrm_multiset *set1, const tmrm_multiset *set2);
int tmrm_multiset_is_equal(const tmrm_multiset *set1, const tmrm_multiset *set2);
*/

/* Returns the number of elements in the set. */
int tmrm_multiset_size(tmrm_multiset *ms);

/* Returns a list with all elements of the multi set. */
tmrm_list* tmrm_multiset_as_list(const tmrm_multiset *ms);

/* Destructor. */
void tmrm_multiset_free(/*@only@*/ tmrm_multiset *ms);


/**
 * @}
 * tmrm_iterator an iterable list. TODO: Move to tmrm_iterator.h
 *
 * FIXME: Adjust the following text when all iterators have been replaced
 *        by multisets.
 * In libtmrm, iterators are used to represent multisets in function
 * calls, so many of the path language functions return iterators or
 * can take iterators as a parameter. The elements returned by an
 * iterator are usually tmrm_proxy and tmrm_literal objects.
 *
 * @defgroup tmrm_iterator tmrm_iterator
 * @ingroup libtmrm_public
 * @{
 */


/* Constructor: */
/*@null@*/ tmrm_iterator* tmrm_iterator_new(tmrm_subject_map_sphere* sms,
                                 void* context, int (*next_method)(void*),
                                 int (*end_method)(void*),
                                 tmrm_object* (*get_element_method)(void*, tmrm_iterator_flag),
                                 void (*free_method)(void*));

int tmrm_iterator_next(tmrm_iterator* it);

int tmrm_iterator_end(tmrm_iterator* it);

/*@null@*/ tmrm_object* tmrm_iterator_get_object(tmrm_iterator* it);

/*@null@*/ tmrm_object* tmrm_iterator_get_key(tmrm_iterator* it);

/*@null@*/ tmrm_object* tmrm_iterator_get_value(tmrm_iterator* it);

void tmrm_iterator_free(tmrm_iterator* it);

/* TODO: Implement: */
/* tmrm_iterator* tmrm_iterator_from_list(tmrm_list* list); */

/** @} */

tmrm_storage* tmrm_storage_new(tmrm_subject_map_sphere* sms, const char* name, const char* params);
void tmrm_storage_free(tmrm_storage* s);


#ifdef __cplusplus
}
#endif

#endif
