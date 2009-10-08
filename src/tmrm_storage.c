/*
 * tmrm_storage.c - Implementation of the storage layer
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
#include <string.h>
#include <stdlib.h>
#include <db.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <libtmrm.h>
#include <tmrm_internal.h>
#include <tmrm_storage_internal.h>
#include <tmrm_storage.h>
#include <tmrm_hash.h>

/* ------------------------------------------------------------------------ */
/* TODO: Should the return type be int? */
void
tmrm_storage_register_factory(tmrm_subject_map_sphere* sms,
        const char* name,
        const char* label,
        void (*factory)(tmrm_storage_factory*))
{
    tmrm_storage_factory* storage;

    TMRM_DEBUG3("Registering storage factory %s (%s)\n", name, label);
    storage = (tmrm_storage_factory*)TMRM_CALLOC(tmrm_storage_factory, 1,
        sizeof(tmrm_storage_factory));
    if (!storage) {
        TMRM_DEBUG1("Out of memory\n");
        TMRM_ASSERT_DIE; /* FIXME FATAL ERROR */
    }
    storage->name = (char*)TMRM_MALLOC(cstring, strlen(name)+1);
    if (!storage->name) {
        TMRM_FREE(cstring, storage->name);
        return /* -1 */;
    }
    strcpy(storage->name, name);
    storage->label = (char*)TMRM_MALLOC(cstring, strlen(label)+1);
    if (!storage->label) {
        TMRM_FREE(cstring, storage->name);
        TMRM_FREE(tmrm_storage_factory, storage);
    }
    strcpy(storage->label, label);
    /* Call the storage registration function on the new object */
    (*factory)(storage);
    tmrm_list_ins_next(sms->factories, NULL, (void*)storage);
}

tmrm_storage*
tmrm_storage_new(tmrm_subject_map_sphere* sms, const char* name, const char* params)
{
    tmrm_storage_factory* factory;
    tmrm_list_elmt* elem;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(name, tmrm_subject_map_sphere,
            (tmrm_storage*)NULL);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(name, cstring,
            (tmrm_storage*)NULL);

    if (sms->factories == NULL) return NULL;

    elem = tmrm_list_head(sms->factories);
    while (elem != NULL) {
        factory = (tmrm_storage_factory*)tmrm_list_data(elem);
        /*TMRM_DEBUG2("Found storage_factory '%s'\n", factory->name);*/
        if (strcmp(factory->name, name) == 0) {
           return tmrm_storage_new_from_factory(sms, factory, params);
        }
        elem = tmrm_list_next(elem);
    }
    TMRM_DEBUG2("Storage not found: '%s'\n", name);
    
    return (tmrm_storage*)NULL;
}

tmrm_storage* 
tmrm_storage_new_from_factory(tmrm_subject_map_sphere* sms, tmrm_storage_factory* factory, const char* params)
{
    tmrm_storage* storage;
    tmrm_hash* options = NULL;
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(factory, tmrm_storage_factory, NULL);
    storage = (tmrm_storage*)TMRM_CALLOC(tmrm_storage, 1, sizeof(tmrm_storage));
    if (!storage) return (tmrm_storage*)NULL;
    storage->subject_map_sphere = sms;
    storage->factory = factory;
    storage->context = NULL;
    if (params != NULL) {
        options = tmrm_hash_new_from_string(sms, "memory", params);
    }
    if (factory->init(storage, options)) {
        tmrm_storage_free(storage);
        return (tmrm_storage*)NULL;
    }
    
    return storage;
}

void
tmrm_storage_free(tmrm_storage* s)
{
    s->factory->free(s);
    TMRM_FREE(tmrm_storage, s);
}

int
tmrm_storage_init(tmrm_storage* s, tmrm_hash* options)
{
    TMRM_DEBUG1("tmrm_storage_init()\n");
    return s->factory->init(s, options);
}

/**
 * Removes a given subject map and its storage (e.g. by removing associated
 * files or databases). 
 * If the storage supports several subject maps, only the parts affecting the
 * subject map are removed. If map is null, all subject maps are removed from
 * the storage.
 * Note that this function does not free the memory occupied by the subject
 * map and the storage (and the subject map system) objects. You still have
 * to call tmrm_subject_map_free() and tmrm_storage_free(). Other calls than
 * there are not guaranteed to work.
 *
 * @param s The storage object
 * @param map The subject map object.
 * @returns 0 on success or a non-zero value on failure.
 */
int
tmrm_storage_remove(tmrm_storage* s, tmrm_subject_map* map) {
    /* Ignore if not applicable or not implemented */
    if (!s->factory->remove) return 0;

    return s->factory->remove(s, map);
}

int tmrm_storage_bootstrap(tmrm_storage* s, tmrm_subject_map* map)
{
    return s->factory->bootstrap(s, map);
}

tmrm_proxy* tmrm_storage_bottom(tmrm_storage* s, tmrm_subject_map* map) {
    return s->factory->bottom(s, map);
}

int tmrm_storage_merge(tmrm_storage* s, tmrm_subject_map* map) {
    return s->factory->merge(s, map);
}

tmrm_proxy*
tmrm_storage_proxy_create(tmrm_storage* s, tmrm_subject_map* map)
{
    return s->factory->proxy_create(s, map);
}

int
tmrm_storage_proxy_update(tmrm_storage* s, tmrm_proxy* p)
{
    /* Ignore if not applicable or not implemented */
    if (s->factory->proxy_update == NULL) return 0;

    return s->factory->proxy_update(s, p);
}

int
tmrm_storage_add_property(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key, tmrm_proxy* value)
{
    return s->factory->add_property(s, p, key, value);
}

int
tmrm_storage_add_property_literal(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key, tmrm_literal* value)
{
    return s->factory->add_property_literal(s, p, key, value);
}

int
tmrm_storage_remove_properties_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key)
{
    /* TODO Could also be implemented independent of the storage (get all
       properties with key 'key' and remove all of them) */
    return s->factory->proxy_remove_properties_by_key(s, p, key);
}


tmrm_iterator*
tmrm_storage_proxy_get_properties(tmrm_storage* s, tmrm_proxy* p) {
    return s->factory->proxy_properties(s, p);
}


tmrm_proxy*
tmrm_storage_proxy_by_label(tmrm_storage* s, tmrm_subject_map* map,
        const char *label)
{
    return s->factory->proxy_by_label(s, map, label);
}

tmrm_iterator*
tmrm_storage_proxies(tmrm_storage* s, tmrm_subject_map* map)
{
    return s->factory->proxies(s, map);
}


/** 
 * Returns the label of a proxy from the backend.
 * Returns NULL on failure.
 */
const char*
tmrm_storage_proxy_label(tmrm_storage* s, tmrm_proxy* p)
{
    return s->factory->proxy_label(s, p);
}

tmrm_iterator*
tmrm_storage_proxy_keys(tmrm_storage* s, tmrm_proxy* p)
{
    return s->factory->proxy_keys(s, p);
}

tmrm_iterator*
tmrm_storage_proxy_values_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key)
{
    return s->factory->proxy_values_by_key(s, p, key);
}

tmrm_iterator*
tmrm_storage_proxy_is_value_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key) {
    return s->factory->proxy_is_value_by_key(s, p, key);
}

tmrm_iterator*
tmrm_storage_literal_is_value_by_key(tmrm_storage* s, tmrm_literal* lit, tmrm_proxy* key) {
    tmrm_iterator* it;
    it = s->factory->literal_is_value_by_key(s, lit, key);
    return it;
}

tmrm_iterator*
tmrm_storage_literal_keys_by_value(tmrm_storage* s, tmrm_literal* lit, tmrm_subject_map* map) {
    tmrm_iterator* it;
    it = s->factory->literal_keys_by_value(s, lit, map);
    return it;
}

int
tmrm_storage_proxy_remove(tmrm_storage* s, const tmrm_proxy* p)
{
    return s->factory->proxy_remove(s, p);
}

int
tmrm_storage_proxy_add_type(tmrm_storage* s, tmrm_proxy *p, tmrm_proxy *type)
{
    return s->factory->proxy_add_type(s, p, type);
}

int
tmrm_storage_proxy_add_superclass(tmrm_storage* s, tmrm_proxy *p, tmrm_proxy *superclass)
{
    return s->factory->proxy_add_superclass(s, p, superclass);
}

tmrm_iterator*
tmrm_storage_proxy_direct_subclasses(tmrm_storage* s, tmrm_proxy *p)
{
    return s->factory->proxy_direct_subclasses(s, p);
}

tmrm_iterator*
tmrm_storage_proxy_direct_superclasses(tmrm_storage* s, tmrm_proxy *p)
{
    return s->factory->proxy_direct_superclasses(s, p);
}

tmrm_iterator*
tmrm_storage_proxy_direct_types(tmrm_storage* s, tmrm_proxy *p)
{
    return s->factory->proxy_direct_types(s, p);
}

tmrm_iterator*
tmrm_storage_proxy_direct_instances(tmrm_storage* s, tmrm_proxy *p)
{
    return s->factory->proxy_direct_instances(s, p);
}

