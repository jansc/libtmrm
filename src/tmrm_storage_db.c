/*
 * tmrm_storage_db.c - implementation of the Berkeley DB backend.
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
#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for abort() as used in errors */
#endif
#include <sys/types.h>

#include <libtmrm.h>
#include <tmrm_storage_internal.h>
#include <tmrm_storage.h>

/* BDB-support */
#include <db.h>
/* /BDB-support */

struct tmrm_storage_db_context_s {
    DB* dbp;
};

typedef struct tmrm_storage_db_context_s tmrm_storage_db_context;

/* ---------------------------------------------------------------------------
Prototypes for the DB storage factory
*/
static void
tmrm_storage_db_register_factory(tmrm_storage_factory *factory);

void
tmrm_init_storage_db(tmrm_subject_map_sphere *sms);

static void
tmrm_storage_db_free(tmrm_storage* storage);

static int
tmrm_storage_db_add_property(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key, tmrm_proxy* value);

static int
tmrm_storage_db_add_property_literal(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key, tmrm_literal* value);

/*
static int
tmrm_storage_db_put_property(tmrm_storage* storage, tmrm_proxy* p, tmrm_proxy* key, tmrm_proxy* value);
*/

/* ------------------------------------------------------------------------ */
static int
tmrm_storage_db_init(tmrm_storage* s, tmrm_hash* options);

/* ======================================================================= */
static int 
tmrm_storage_db_init(tmrm_storage* s, tmrm_hash* options) {
    tmrm_storage_db_context* c;
    int ret;
    u_int32_t flags;

    TMRM_DEBUG1("tmrm_storage_db_init()\n");
    c = (tmrm_storage_db_context*)TMRM_CALLOC(tmrm_storage_db_context, 1, sizeof(tmrm_storage_db_context));
    if (!c) return 1;
    ret = db_create(&c->dbp, NULL, 0);
    if (ret) {
        TMRM_FREE(tmrm_storage_db_context, c);
        return ret;
    }
    flags = DB_CREATE;

    ret = c->dbp->open(c->dbp, NULL, "tmrm.db", NULL, DB_BTREE, flags, 0);
    if (ret) {
        TMRM_FREE(tmrm_storage_db_context, c);
        return ret;
    }

    s->context = c;
    return 0;
}

static void
tmrm_storage_db_free(tmrm_storage* s) {
    int ret;
    TMRM_DEBUG1("tmrm_storage_db_free\n");
    tmrm_storage_db_context* c = (tmrm_storage_db_context*)s->context;
    if (c->dbp != NULL) {
        ret = c->dbp->close(c->dbp, 0);
        if (ret) {
            TMRM_DEBUG2("Closing tmrm-database failed: %s\n", db_strerror(ret));
        }
    }
    TMRM_FREE(tmrm_storage_db_context, s->context);
}

int
tmrm_storage_db_add_property(tmrm_storage* storage, tmrm_proxy* p, tmrm_proxy* key, tmrm_proxy* value) {
    TMRM_DEBUG1("PUT_PROPERTY\n");
    return 0;
}

int
tmrm_storage_db_add_property_literal(tmrm_storage* storage, tmrm_proxy* p, tmrm_proxy* key, tmrm_literal* value) {
    TMRM_DEBUG1("PUT_PROPERTY_LITERAL\n");
    return 0;
}

/*
int
tmrm_storage_db_put_property(tmrm_storage* storage, tmrm_property* pr) {
    TMRM_DEBUG1("FOO\n");
    TMRM_DEBUG2("property-id = %d\n", pr->statement_id); 
    DBT key, data;
    size_t recordsize, recordlen;
    char* record;
    recordsize = sizeof(int) * 4;
    record = TMRM_MALLOC(void, recordsize);

    memset(record, 0, recordsize);
    memcpy(record, &(pr->proxy->proxy_id), sizeof(int));
    recordlen = sizeof(int);
    memcpy(record + recordlen, &(pr->statement_id), sizeof(int));
    recordlen += sizeof(int);
    memcpy(record + recordlen, &(pr->label_id), sizeof(int));
    recordlen += sizeof(int);

    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    key.data = &(pr->statement_id);
    key.size = sizeof(int);

    data.data = record;
    data.size = recordlen;

    tmrm_storage_db_context* c = (tmrm_storage_db_context*)storage->context;
    c->dbp->put(c->dbp, NULL, &key, &data, DB_NOOVERWRITE);

    TMRM_FREE(void, record);
    return 0;
}
*/
/*
int 
tmrm_storage_db_put_proxy(tmrm_storage* storage, tmrm_proxy* p) {
    TMRM_DEBUG1("BAR\n");
}
*/
/*
tmrm_property*
tmrm_storage_db_get_property(tmrm_storage* storage, int label) {
    TMRM_DEBUG2("get_property(%d)\n", label);
    DBT key, data;
    char* record;
    tmrm_property* pr;

    pr = (tmrm_property*)TMRM_CALLOC(tmrm_property, 1, sizeof(tmrm_property));

    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    key.data = &label;
    key.size = sizeof(int);

    tmrm_storage_db_context* c = (tmrm_storage_db_context*)storage->context;
    c->dbp->get(c->dbp, NULL, &key, &data, 0);

    record = data.data;

    pr->proxy = (tmrm_proxy*)NULL;
    pr->statement_id = *((int *)record);
    TMRM_DEBUG2("pr->proxy_id\t= %d\n", pr->statement_id);
    record += sizeof(int);
    pr->statement_id = *((int *)record);
    TMRM_DEBUG2("pr->statement_id\t= %d\n", pr->statement_id);
    record += sizeof(int);
    pr->label_id = *((int *)record);
    TMRM_DEBUG2("pr->label_id\t= %d\n", pr->label_id);
    
    return pr;
}
*/
static void
tmrm_storage_db_register_factory(tmrm_storage_factory *factory) {
    factory->init = tmrm_storage_db_init; 
    factory->free = tmrm_storage_db_free; 
    factory->bottom = NULL;
    factory->remove = NULL;
    factory->proxy_create = NULL;
    factory->proxy_update = NULL;
    factory->add_property = tmrm_storage_db_add_property;
    factory->add_property_literal = tmrm_storage_db_add_property_literal;
    factory->proxy_remove_properties_by_key = NULL;
    factory->proxy_properties = NULL;
    factory->proxy_remove = NULL;
    factory->proxy_by_label = NULL;
    factory->proxies = NULL;
    factory->proxy_label = NULL;
    factory->proxy_keys = NULL;
    factory->proxy_values_by_key = NULL;
    factory->proxy_is_value_by_key = NULL;
    factory->proxy_keys_by_value = NULL;
    factory->literal_keys_by_value = NULL;
    factory->literal_is_value_by_key = NULL;
    factory->proxy_add_type = NULL;
    factory->proxy_add_superclass = NULL;
    factory->proxy_direct_subclasses = NULL;
    factory->proxy_direct_superclasses = NULL;
    factory->proxy_direct_types = NULL;
    factory->proxy_direct_instances = NULL;
}

void
tmrm_init_storage_db(tmrm_subject_map_sphere *sms)
{
    tmrm_storage_register_factory(sms, "dbd", "Sleepycat DBD storage module",
            &tmrm_storage_db_register_factory);
}
