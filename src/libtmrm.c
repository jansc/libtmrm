/*
 * libtmrm.c - libtmrm
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for abort() as used in errors */
#endif

#include <libtmrm_config.h>


#include <libtmrm.h>
#include <tmrm_list.h>
#include <tmrm_internal.h>
#include <tmrm_storage.h>


/**
 * Contructor: Creates a new subject map system object. Returns NULL
 * on failure.
 * The memory occupied by the object has to be freed with the 
 * tmrm_subject_map_sphere_free() function.
 *
 * @returns NULL on failure
 */
/*@null@*/ tmrm_subject_map_sphere* 
tmrm_subject_map_sphere_new(void)
{
    tmrm_subject_map_sphere* new_sms;
    new_sms = (tmrm_subject_map_sphere*)TMRM_CALLOC(tmrm_subject_map_sphere, 1,
            sizeof(tmrm_subject_map_sphere));
    if (!new_sms)
        return (tmrm_subject_map_sphere*)NULL;
    /* Use free to free tmrm_storage_factory structures */
    new_sms->factories = tmrm_list_new((tmrm_list_free_handler*)free);
    tmrm_init_storage(new_sms); 
    tmrm_init_hash(new_sms);
    return new_sms;
}


/**
 * Destructor.
 * @param sms A pointer to a valid subject_map_sphere object.
 */
void
tmrm_subject_map_sphere_free(/*@only@*/ tmrm_subject_map_sphere* sms)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN(sms, tmrm_subject_map_sphere);
    tmrm_list_free(sms->factories);
    /*@=compdestroy@*/
    TMRM_FREE(tmrm_subject_map_sphere, sms);
}

/**
 * Internal function that sets the error flags of the corresponding
 * subject map sphere object.
 */
static int
tmrm_subject_map_sphere_set_storage_error(tmrm_subject_map_sphere *sms,
        tmrm_storage *storage, const char *problem) {
    size_t len;

    TMRM_ASSERT_OBJECT_POINTER_RETURN(sms, tmrm_subject_map_sphere);

    if (sms->errno != TMRM_NO_ERROR) return 1;

    sms->errno = TMRM_STORAGE_ERROR;
    len = strlen(problem);
    if (!(sms->err = (char*)TMRM_MALLOC(cstring, len+1))) {
        return 1;
    }
    (void)strcpy(sms->err, problem);
    return 1;
}


/**
 * Constructor: Creates a new subject map with a label.
 * The memory occupied by the object has to be freed with
 * tmrm_subject_map_free().
 *
 * The function will always assure that the tmrm-bootstrapping-ontology
 * is part of the subject map.
 *
 * @param sms The subject map sphere that the subject map belongs to.
 * @param storage The backend that is used to store the subject map.
 * @param label A unique id that identfies the subject map in the
 *        subject map sphere that it belongs to. This may be a path name,
 *        a UUID, a URI.
 * @returns NULL on failure.
 */
/*@null@*/ tmrm_subject_map*
tmrm_subject_map_new(tmrm_subject_map_sphere* sms,
                     tmrm_storage* storage, const char* label)
{
    tmrm_subject_map* new_subject_map;
    char *tmp_label;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(sms, tmrm_subject_map_sphere,
            (tmrm_subject_map*)NULL);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(storage, tmrm_storage,
            (tmrm_subject_map*)NULL);
    new_subject_map = (tmrm_subject_map*)TMRM_CALLOC(tmrm_subject_map, 1,
            sizeof(tmrm_subject_map));
    if (!new_subject_map) return NULL;

    tmp_label = (char*)TMRM_MALLOC(cstring, strlen(label) + 1);
    if (!tmp_label) {
        /*@-compdestroy@*/
        TMRM_FREE(tmrm_subject_map, new_subject_map);
        /*@=compdestroy@*/
        return NULL;
    }
    (void)strcpy(tmp_label, label);
    new_subject_map->sms = sms;
    new_subject_map->storage = storage;
    new_subject_map->label = tmp_label;
    new_subject_map->bottom = new_subject_map->superclass =
        new_subject_map->subclass = new_subject_map->type =
        new_subject_map->instance = NULL;

    /* Make sure that the bootstrap ontology proxies exist and store
       pointers to them in the subject map object. */
    tmrm_storage_bootstrap(storage, new_subject_map);
    return new_subject_map;
}


/**
 * Returns the symbolic name of a subject map.
 *
 * PHP-prototype: TMRM_SubjectMap::getName()
 */
/*@null@*/ const char*
tmrm_subject_map_name(tmrm_subject_map* map) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(map, tmrm_subject_map, (const char*)NULL);
    return map->label;
}


/**
 * Returns the bottom proxy of the subject map. This proxy can be used for
 * bootstrapping purposes. If bottom does not exist, it will be created.
 */
/*@null@*/ tmrm_proxy*
tmrm_subject_map_bottom(tmrm_subject_map* map) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(map, tmrm_subject_map, (tmrm_proxy*)NULL);
    return tmrm_storage_bottom(map->storage, map);
}


/* Merges all equal proxies in the subject map until the subject map is
   fully merged. */
int tmrm_subject_map_merge(tmrm_subject_map *map) {
    return tmrm_storage_merge(map->storage, map);
}


/**
 * Frees the memory occupied by a subject map object. m must not be NULL.
 */
void
tmrm_subject_map_free(tmrm_subject_map* m)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN(m, tmrm_subject_map);
    if (m->label)
        TMRM_FREE(cstring, m->label);

    /* Free the bootstrap ontology proxies */
    if (m->bottom != NULL) tmrm_proxy_free(m->bottom);
    if (m->superclass != NULL) tmrm_proxy_free(m->superclass);
    if (m->subclass != NULL) tmrm_proxy_free(m->subclass);
    if (m->type != NULL) tmrm_proxy_free(m->type);
    if (m->instance != NULL) tmrm_proxy_free(m->instance);
    
    TMRM_FREE(tmrm_subject_map, m);
}

void 
tmrm_init_storage(tmrm_subject_map_sphere *sms)
{
#ifdef STORAGE_HASHES
    tmrm_init_storage_db(sms);
#endif

#ifdef STORAGE_POSTGRESQL
    tmrm_init_storage_pgsql(sms);
#endif
}

/* temporary bootstrap-ontology:
%YAML 1.1
---
subject_map: 'libtmrm-bootstrap'
proxies:
  _bottom_: {}
  subtype:
    _bottom_: {value: "subtype", datatype: 'http://www.w3.org/2001/XMLSchema#string'}
  supertype:
    _bottom_: {value: "supertype", datatype: 'http://www.w3.org/2001/XMLSchema#string'}
  type:
    _bottom_: {value: "type", datatype: 'http://www.w3.org/2001/XMLSchema#string'}
  instance:
    _bottom_: {value: "instance", datatype: 'http://www.w3.org/2001/XMLSchema#string'}
*/
