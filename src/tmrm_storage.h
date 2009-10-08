/*
 * tmrm_storage.h - implementation of the storage layer.
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
#ifndef LIBTMRM_STORAGE_H
#define LIBTMRM_STORAGE_H

#include <libtmrm.h>
#include <tmrm_storage_internal.h>

#ifdef __cplusplus
extern "C" {
#endif


tmrm_storage* tmrm_storage_new_from_factory(tmrm_subject_map_sphere* sms, tmrm_storage_factory* factory, const char* params);

int tmrm_storage_init(tmrm_storage* s, tmrm_hash* options);
/*
Could be changed to
int tmrm_storage_open(tmrm_storage* s, tmrm_hash* options);
int tmrm_storage_create(tmrm_storage* s, tmrm_hash* options);
*/

/* Removes a given subject map and its storage */
int tmrm_storage_remove(tmrm_storage* s, tmrm_subject_map* map);

int tmrm_storage_bootstrap(tmrm_storage* s, tmrm_subject_map* map);
tmrm_proxy* tmrm_storage_bottom(tmrm_storage* s, tmrm_subject_map* map);

/* Merges all equal proxies until the subject map is fully merged */
int tmrm_storage_merge(tmrm_storage* s, tmrm_subject_map* map);

tmrm_proxy* tmrm_storage_proxy_create(tmrm_storage* storage, tmrm_subject_map* map);
int tmrm_storage_proxy_update(tmrm_storage* storage, tmrm_proxy* p);
int tmrm_storage_add_property(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key, tmrm_proxy* value);
int tmrm_storage_add_property_literal(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key, tmrm_literal* value);

int tmrm_storage_remove_properties_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key);

tmrm_iterator* tmrm_storage_proxy_get_properties(tmrm_storage* s, tmrm_proxy* p);

tmrm_proxy* tmrm_storage_proxy_by_label(tmrm_storage* s, tmrm_subject_map* map, const char *label); 

/* Fetches the label of a proxy from the backend, and stores its value in p. */
const char* tmrm_storage_proxy_label(tmrm_storage* s, tmrm_proxy* p);

tmrm_iterator* tmrm_storage_proxies(tmrm_storage* s, tmrm_subject_map* map);

tmrm_iterator* tmrm_storage_proxy_keys(tmrm_storage* s, tmrm_proxy* p);
tmrm_iterator* tmrm_storage_proxy_values_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key);

tmrm_iterator* tmrm_storage_proxy_keys_by_value(tmrm_storage* s, tmrm_proxy* p);

/* Removes a proxy from the storage */
int tmrm_storage_proxy_remove(tmrm_storage* s, const tmrm_proxy* p);

int tmrm_storage_proxy_add_type(tmrm_storage* s, tmrm_proxy *p, tmrm_proxy *type);
int tmrm_storage_proxy_add_superclass(tmrm_storage* s,
        tmrm_proxy *p, tmrm_proxy *superclass);

tmrm_iterator* tmrm_storage_proxy_direct_subclasses(tmrm_storage* s, tmrm_proxy* p);
tmrm_iterator* tmrm_storage_proxy_direct_superclasses(tmrm_storage* s,
        tmrm_proxy* p);
tmrm_iterator* tmrm_storage_proxy_direct_types(tmrm_storage* s, tmrm_proxy *p);
tmrm_iterator* tmrm_storage_proxy_direct_instances(tmrm_storage* s, tmrm_proxy *p);

tmrm_iterator* tmrm_storage_proxy_is_value_by_key(tmrm_storage* s,
        tmrm_proxy* p, tmrm_proxy* key);

tmrm_iterator* tmrm_storage_literal_is_value_by_key(tmrm_storage* s,
        tmrm_literal* lit, tmrm_proxy* key);

tmrm_iterator* tmrm_storage_literal_keys_by_value(tmrm_storage* s, tmrm_literal* lit, tmrm_subject_map* map);

/* TODO:

tmrm_list* tmrm_proxies_by_properties(tmrm_subject_map* map, tmrm_list* properties); => TMRM_SubjectMap::getProxiesByProperties()
tmrm_iterator* tmrm_subject_map_proxies(tmrm_subject_map* map); => TMRM_SubjectMap::getProxies()
PATH LANGUAGE:
tmrm_iterator* tmrm_subject_map_values_by_key(tmrm_iterator* proxies, tmrm_proxy* key);
tmrm_iterator* tmrm_subject_map_values_by_key_star(tmrm_iterator* proxies, tmrm_proxy* key);
tmrm_iterator* tmrm_subject_map_is_value_by_key_star(tmrm_iterator* proxies, tmrm_proxy* key);
tmrm_iterator* tmrm_subject_map_keys_by_value(tmrm_iterator* value)

int tmrm_proxy_remove_property(tmrm_proxy* p, tmrm_proxy* key, tmrm_proxy* value);
int tmrm_proxy_remove_property_literal(tmrm_proxy* p, tmrm_proxy* key, const unsigned char* value);

Can be implemented with other functions
? int tmrm_proxy_remove_properties_by_key(tmrm_proxy* p, tmrm_proxy* key);
*/

void tmrm_storage_free(tmrm_storage* s);


extern void
tmrm_init_storage_pgsql(tmrm_subject_map_sphere *sms);

extern void
tmrm_init_storage_db(tmrm_subject_map_sphere *sms);

#ifdef __cplusplus
}
#endif

#endif
