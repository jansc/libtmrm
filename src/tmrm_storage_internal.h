/*
 * tmrm_storage_internal.h - Internal structures of the storage layer
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

#ifndef LIBTMRM_STORAGE_INTERNAL_H
#define LIBTMRM_STORAGE_INTERNAL_H

#include "tmrm_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

void tmrm_storage_register_factory(tmrm_subject_map_sphere *sms,
        const char* name, const char* label,
        void (*factory)(tmrm_storage_factory*));

/** A storage object */
struct tmrm_storage_s
{
    tmrm_subject_map_sphere* subject_map_sphere;
    struct tmrm_storage_factory_s* factory;
    void *context;
};

struct tmrm_storage_factory_s {
    char* name;
    char* label;

    /* the rest of this structure is populated by the
     *      storage-specific register function */
    size_t context_length;

    /* Contructor */
    int (*init)(tmrm_storage* storage, tmrm_hash* options);
    /* Destructor */
    void (*free)(tmrm_storage* storage);

    int (*bootstrap)(tmrm_storage* s, tmrm_subject_map* map);
    int (*remove)(tmrm_storage* storage, tmrm_subject_map* map);

    tmrm_proxy* (*bottom)(tmrm_storage* storage, tmrm_subject_map* map);
    int (*merge)(tmrm_storage* storage, tmrm_subject_map* map);

    tmrm_proxy* (*proxy_create)(tmrm_storage* storage, tmrm_subject_map* map);
    int (*proxy_update)(tmrm_storage* storage, tmrm_proxy* p);
    int (*add_property)(tmrm_storage* storage, tmrm_proxy* p,
            tmrm_proxy* key, tmrm_proxy* value);
    int (*add_property_literal)(tmrm_storage* storage, tmrm_proxy* p, 
            tmrm_proxy* key, tmrm_literal* value);
    tmrm_proxy* (*proxy_by_label)(tmrm_storage* storage, 
        tmrm_subject_map* map, const char* label);
    tmrm_iterator* (*proxies)(tmrm_storage* storage, tmrm_subject_map* map);
    char* (*proxy_label)(tmrm_storage* storage, tmrm_proxy* p);
    tmrm_iterator* (*proxy_keys)(tmrm_storage* storage, tmrm_proxy* p);
    tmrm_iterator* (*proxy_values_by_key)(tmrm_storage* storage, tmrm_proxy* p,
            tmrm_proxy* key);
    tmrm_iterator* (*proxy_is_value_by_key)(tmrm_storage* storage, tmrm_proxy* p,
            tmrm_proxy* key);
    tmrm_iterator* (*literal_is_value_by_key)(tmrm_storage* storage,
            tmrm_literal* lit, tmrm_proxy* key);
    tmrm_iterator* (*proxy_keys_by_value)(tmrm_storage* storage, tmrm_proxy* p);
    tmrm_iterator* (*literal_keys_by_value)(tmrm_storage* storage, tmrm_literal* lit, tmrm_subject_map* map);
    int (*proxy_add_type)(tmrm_storage* storage, tmrm_proxy* p,
            tmrm_proxy* type);
    int (*proxy_add_superclass)(tmrm_storage* storage, tmrm_proxy* p,
            tmrm_proxy* superclass);
    tmrm_iterator* (*proxy_direct_subclasses)(tmrm_storage* storage,
            tmrm_proxy* p);
    tmrm_iterator* (*proxy_direct_superclasses)(tmrm_storage* storage,
            tmrm_proxy* p);
    tmrm_iterator* (*proxy_direct_types)(tmrm_storage* storage, tmrm_proxy* p);
    tmrm_iterator* (*proxy_direct_instances)(tmrm_storage* storage, tmrm_proxy* p);
    int (*proxy_remove_properties_by_key)(tmrm_storage* storage, tmrm_proxy* p,
            tmrm_proxy* key);
    tmrm_iterator* (*proxy_properties)(tmrm_storage* storage, tmrm_proxy* p);
    int (*proxy_remove)(tmrm_storage* storage, const tmrm_proxy* p);

};

#endif
