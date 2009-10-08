/*
 * tmrm_storage_pgsql.c - The postgresql storage module
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

#include <libtmrm.h>
#include <tmrm_internal.h>
#include <tmrm_storage_internal.h>
#include <tmrm_storage.h>
#include <tmrm_hash.h>

#include <libpq-fe.h>

struct tmrm_storage_pgsql_context_s {
    const char* host;
    const char* port;
    const char* dbname;
    const char* user;
    const char* password;
    PGconn* conn;
};

typedef struct tmrm_storage_pgsql_context_s tmrm_storage_pgsql_context;

struct tmrm_storage_pgsql_iterator_context_s {
    tmrm_subject_map *subject_map;
    PGresult *res;
    int num_rows;
    int current_row;
};

typedef struct tmrm_storage_pgsql_iterator_context_s tmrm_storage_pgsql_iterator_context;


/* ---------------------------------------------------------------------------
   Prototypes for the pgsql storage factory
 */
static void
tmrm_storage_pgsql_register_factory(tmrm_storage_factory *factory);

static int 
tmrm_storage_pgsql_init(tmrm_storage* s, tmrm_hash* options);

static void
tmrm_storage_pgsql_free(tmrm_storage* storage);

static int
tmrm_storage_pgsql_remove(tmrm_storage* storage, tmrm_subject_map* map);

static tmrm_proxy*
tmrm_storage_pgsql_bottom(tmrm_storage* storage, tmrm_subject_map* map);

static int
tmrm_storage_pgsql_merge(tmrm_storage* storage, tmrm_subject_map* map);

static int
tmrm_storage_pgsql_proxy_merge(tmrm_storage* storage, tmrm_proxy* p1, tmrm_proxy* p2);

void
tmrm_init_storage_pgsql(tmrm_subject_map_sphere *sms);

static tmrm_proxy*
tmrm_storage_pgsql_proxy_create(tmrm_storage* storage, tmrm_subject_map* map);

static int
tmrm_storage_pgsql_proxy_update(tmrm_storage* storage, tmrm_proxy* p);

static int
tmrm_storage_pgsql_add_property(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key, tmrm_proxy* value);

static int
tmrm_storage_pgsql_add_property_literal(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key, tmrm_literal* value);

static int
tmrm_storage_pgsql_proxy_remove_properties_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key);

static int
tmrm_storage_pgsql_proxy_remove(tmrm_storage* s, const tmrm_proxy* p);

static tmrm_iterator*
tmrm_storage_pgsql_proxy_properties(tmrm_storage* s, tmrm_proxy* p);

static tmrm_proxy*
tmrm_storage_pgsql_proxy_by_label(tmrm_storage* s, tmrm_subject_map* map,
        const char* label);

static tmrm_iterator*
tmrm_storage_pgsql_proxies(tmrm_storage* s, tmrm_subject_map* map);

static char*
tmrm_storage_pgsql_proxy_label(tmrm_storage* s, tmrm_proxy* p);

static tmrm_iterator*
tmrm_storage_pgsql_proxy_keys(tmrm_storage* s, tmrm_proxy* p);

static tmrm_iterator*
tmrm_storage_pgsql_proxy_values_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key);

static tmrm_iterator*
tmrm_storage_pgsql_proxy_is_value_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key);

static tmrm_iterator*
tmrm_storage_pgsql_proxy_keys_by_value(tmrm_storage* s, tmrm_proxy* p);

static tmrm_iterator*
tmrm_storage_pgsql_literal_keys_by_value(tmrm_storage* s, tmrm_literal* lit, tmrm_subject_map* map);

static tmrm_iterator*
tmrm_storage_pgsql_literal_is_value_by_key(tmrm_storage* s, tmrm_literal* lit, tmrm_proxy* key);

static tmrm_proxy*
tmrm_storage_pgsql_proxy_by_literal(tmrm_storage* s, tmrm_literal* lit, tmrm_proxy* key);

static int
tmrm_storage_pgsql_proxy_add_type(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* type);

static int
tmrm_storage_pgsql_proxy_add_superclass(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* superclass);

/* Helper function */
static tmrm_iterator*
tmrm_storage_pgsql_proxy_direct_class(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* a, tmrm_proxy* b);

static tmrm_iterator*
tmrm_storage_pgsql_proxy_direct_subclasses(tmrm_storage* s, tmrm_proxy* p);

static tmrm_iterator*
tmrm_storage_pgsql_proxy_direct_superclasses(tmrm_storage* s, tmrm_proxy* p);

static tmrm_iterator*
tmrm_storage_pgsql_proxy_direct_types(tmrm_storage* s, tmrm_proxy* p);

static tmrm_iterator*
tmrm_storage_pgsql_proxy_direct_instances(tmrm_storage* s, tmrm_proxy* p);

/* ---------------------------------------------------------------------------
   Internal functions. Returns a new unique id for the use as a proxy id.
 */

static int 
tmrm_storage_pgsql_bootstrap(tmrm_storage* s, tmrm_subject_map* map);

static int
tmrm_storage_pgsql_list_next(void* context);

static int
tmrm_storage_pgsql_list_end(void* context);

static tmrm_object*
tmrm_storage_pgsql_proxy_list_get_element(void* context, tmrm_iterator_flag flag);

static tmrm_object*
tmrm_storage_pgsql_value_list_get_element(void* context, tmrm_iterator_flag flag);

static tmrm_object*
tmrm_storage_pgsql_property_get_element(void* context, tmrm_iterator_flag flag);

static void
tmrm_storage_pgsql_list_free(void* context);

/* Returns a new (unique) proxy id, or 0 on failure */
static tmrm_label
_proxy_get_new_id(tmrm_storage* s);

/* Creates a new proxy with a given id */
static int
_create_proxy(tmrm_storage* s, tmrm_label id);

static tmrm_proxy*
_create_proxy_struct(tmrm_subject_map* m, tmrm_label label);

static int
_exec_sql(tmrm_storage* s, const char* query);

static tmrm_iterator*
_iterator_by_sql_query(tmrm_storage* s, tmrm_subject_map *subject_map, const char* query);

static tmrm_iterator*
_iterator_by_sql_query_params(tmrm_storage* s, tmrm_subject_map *subject_map, const char* query, const char* const *param_values, int param_count);

/* ======================================================================= */
/* 
 * PostgreSQL-specific functions are placed here.
 */

static int 
tmrm_storage_pgsql_create(tmrm_storage* s)
{
    PGresult* res;
    PGconn* conn;
    char conn_str[512];
    char querystr[200+TMRM_MAXLENGTH_MAP_NAME];  
    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return -1;

    /* create a subject map

       Returns true if map creation succeeded, false on error
Note: This function will not initialize the map, the caller must do this

TODO: host, port, user, password from parameters
TODO: name from options?
     */

    (void)snprintf(conn_str, 512, "host=%s port=%s dbname=template1 user=%s password=%s",
            (c->host == NULL ? "localhost" : c->host),
            (c->port == NULL ? "5432" : c->port), c->user,
            (c->password == NULL ? "" : c->password));
    conn = PQconnectdb(conn_str);
    if (PQstatus(conn) != CONNECTION_OK ) {
        fprintf(stdout, "Connection to postgresql database failed: %s\n",
                PQerrorMessage(conn));
        return -1;
    }

    /*   - connect to template1
         - try  CREATE DATABASE xxx WITH TEMPLATE template0 ENCODING 'UTF-8'
         if it fails, the database may exist already

         FIXME constant TMRM_MAXLENGTH_MAP_NAME for max length of names
     */
    sprintf(querystr,
            "CREATE DATABASE %200s WITH TEMPLATE template0 ENCODING 'UTF-8'", c->dbname);
    res = PQexec(conn, querystr);
    if (!res) {
        /* fatal error */
        fprintf(stdout,
                "postgresql create database failed with postgres fatal error: %s\n",
                PQerrorMessage(conn));
        return -1;
    } else if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stdout, "postgresql create database failed: %s\n",
                PQresultErrorMessage(res));
        PQclear(res);
        return -1;
    }

    /* TODO:
    // XXX connect to the created database
    // FIXME: host, ports etc from options */
    sprintf(querystr, "host=localhost dbname=%200s", c->dbname);
    conn = PQconnectdb(querystr);
    if (PQstatus(conn) != CONNECTION_OK ) {
        fprintf(stdout,
                "Connection to the newly created postgresql database failed: %s\n",
                PQerrorMessage(conn));
        return -1;
    }

    /*   - create schema and indexes */

    /* TODO: Indexes / Get schema from file/preprocessor/config/something? */
    res = PQexec(conn, 
            "SET client_encoding = 'UTF8';"

            "CREATE TABLE proxy ("
            "id serial,"
            "hash VARCHAR(200),"
            "PRIMARY KEY (id)"
            ");"

            "CREATE TABLE property ("
            "proxy INTEGER NOT NULL,"
            "key INTEGER NOT NULL,"
            "value INTEGER,    "
            "value_literal TEXT,"
            "datatype TEXT,"
            "FOREIGN KEY (proxy) REFERENCES proxy(id),"
            "FOREIGN KEY (key) REFERENCES proxy(id),"
            "FOREIGN KEY (value) REFERENCES proxy(id)"
            ");"

            "-- some necessary data (temporarily) to make it work... \n"
            "INSERT INTO proxy (id) VALUES (0);"
            "INSERT INTO property (proxy, key, value) VALUES (0, 0, 0);\n"
            );

    if (!res) {
        /* fatal error */
        fprintf(stdout, "create schema in postgres failed with fatal error: %s\n",
                PQerrorMessage(conn));
        return -1;
    } else if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stdout, "create schema in postgres failed with message: %s\n",
                PQresultErrorMessage(res));
        PQclear(res);
        return -1;
    }
    PQclear(res);
    PQfinish(conn);
    return 0;
}

static int 
tmrm_storage_pgsql_init(tmrm_storage* s, tmrm_hash* options)
{
    tmrm_storage_pgsql_context* c;
    char conn_str[512];

    /* Opens a database connection */
    c = (tmrm_storage_pgsql_context*)TMRM_CALLOC(tmrm_storage_pgsql_context, 1,
            sizeof(tmrm_storage_pgsql_context));
    if (!c) return -1;
    c->host = tmrm_hash_get(options, "host");
    c->port = tmrm_hash_get(options, "port");
    c->dbname = tmrm_hash_get(options, "dbname");
    c->user = tmrm_hash_get(options, "user");
    c->password = tmrm_hash_get(options, "password");
    s->context = c;
    if (c->user == NULL || c->dbname == NULL) {
        fprintf(stderr, "Missing user or dbname - check the options string\n");
        return -1;
    }
    if (tmrm_hash_get_as_boolean(options, "new") > 0) {
        TMRM_DEBUG1("Creating storage\n");
        if (tmrm_storage_pgsql_create(s)) {
            return -1;
        }
    }
    (void)snprintf(conn_str, 512, "host=%s port=%s dbname=%s user=%s password=%s",
            (c->host == NULL ? "localhost" : c->host),
            (c->port == NULL ? "5432" : c->port), c->dbname, c->user,
            (c->password == NULL ? "" : c->password));
    c->conn = PQconnectdb(conn_str);
    if (PQstatus(c->conn) != CONNECTION_OK ) {
        fprintf(stderr, "Connection to postgresql database failed: %s\n",
                PQerrorMessage(c->conn));
        return -1;
    }

    return 0;
}


/**
 * Fetches the proxies from the bootstrap ontology and stores them in
 * the context object.
 *
 * @returns 0 on success or a non-zero value on failure.
 */
static int 
tmrm_storage_pgsql_bootstrap(tmrm_storage* s, tmrm_subject_map* map)
{
    tmrm_storage_pgsql_context *c;
    tmrm_literal *lit;
    /* TODO: move this to tmrm_proxy.h */
    const unsigned char* bootstrap_ontology = (unsigned char*)"%YAML 1.1\n"
        "---\n"
        "subject_map: 'bootstrap'\n"
        "proxies:\n"
        "    libtmrm_bottom: {libtmrm_bottom: libtmrm_bottom}\n"
        "    ontology: {libtmrm_bottom: 'libtmrm:ontology'}\n"
        "    ontology_version_major: {libtmrm_bottom: 'libtmrm:ontology-version-major'}\n"
        "    ontology_version_minor: {libtmrm_bottom: 'libtmrm:ontology-version-minor'}\n"
        "    bootstrap:\n"
        "        ontology: 'bootstrap'\n"
        "        ontology_version_major: 0\n"
        "        ontology_version_minor: 1\n"
        "    superclass: {libtmrm_bottom: 'libtmrm:superclass'}\n"
        "    subclass: {libtmrm_bottom: 'libtmrm:subclass'}\n"
        "    type: {libtmrm_bottom: 'libtmrm:type'}\n"
        "    instance: {libtmrm_bottom: 'libtmrm:instance'}\n"
        "...\n";

    c = (tmrm_storage_pgsql_context*)TMRM_CALLOC(tmrm_storage_pgsql_context, 1,
            sizeof(tmrm_storage_pgsql_context));

    TMRM_DEBUG1("bootstrapping...\n");
    
    tmrm_subject_map_import_from_yaml_string(map, bootstrap_ontology,
        strlen((char*)bootstrap_ontology));

    map->bottom = tmrm_subject_map_bottom(map);
    lit = tmrm_literal_new((tmrm_char_t*)"libtmrm:superclass", (tmrm_char_t*)TMRM_XMLSCHEMA_STRING);
    if (!lit) return 1;
    map->superclass = tmrm_storage_pgsql_proxy_by_literal(s, lit, map->bottom);
    tmrm_literal_free(lit);

    lit = tmrm_literal_new((tmrm_char_t*)"libtmrm:subclass", (tmrm_char_t*)TMRM_XMLSCHEMA_STRING);
    if (!lit) return 1;
    map->subclass = tmrm_storage_pgsql_proxy_by_literal(s, lit, map->bottom);
    tmrm_literal_free(lit);
    
    lit = tmrm_literal_new((tmrm_char_t*)"libtmrm:type", (tmrm_char_t*)TMRM_XMLSCHEMA_STRING);
    if (!lit) return 1;
    map->type = tmrm_storage_pgsql_proxy_by_literal(s, lit, map->bottom);
    tmrm_literal_free(lit);
    
    lit = tmrm_literal_new((tmrm_char_t*)"libtmrm:instance", (tmrm_char_t*)TMRM_XMLSCHEMA_STRING);
    if (!lit) return 1;
    map->instance = tmrm_storage_pgsql_proxy_by_literal(s, lit, map->bottom);
    tmrm_literal_free(lit);

    return 0;
}

static void
tmrm_storage_pgsql_free(tmrm_storage* s)
{
    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    /* Close database connection */
    if (!c) return;

    if (c->conn != NULL)
        PQfinish(c->conn);
    c->conn = NULL;

    TMRM_FREE(tmrm_storage_pgsql_context, s->context);
}

static int
tmrm_storage_pgsql_remove(tmrm_storage* s, tmrm_subject_map* map)
{
    PGresult* res;
    PGconn* conn;
    char conn_str[512];
    char querystr[200+TMRM_MAXLENGTH_MAP_NAME];  
    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return -1;

    (void)snprintf(conn_str, 512, "host=%s port=%s dbname=template1 user=%s password=%s",
            (c->host == NULL ? "localhost" : c->host),
            (c->port == NULL ? "5432" : c->port), c->user,
            (c->password == NULL ? "" : c->password));
    conn = PQconnectdb(conn_str);
    if (PQstatus(conn) != CONNECTION_OK ) {
        fprintf(stdout, "Connection to postgresql database failed: %s\n",
                PQerrorMessage(conn));
        return -1;
    }

    (void)snprintf(querystr, 200+TMRM_MAXLENGTH_MAP_NAME,
            "DROP DATABASE %200s", c->dbname);
    res = PQexec(conn, querystr);
    if (!res) {
        /* fatal error */
        fprintf(stdout,
                "postgresql create database failed with postgres fatal error: %s\n",
                PQerrorMessage(conn));
        return -1;
    } else if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stdout, "postgresql create database failed: %s\n",
                PQresultErrorMessage(res));
        PQclear(res);
        return -1;
    }
    return 0;
}

static tmrm_proxy*
tmrm_storage_pgsql_bottom(tmrm_storage* storage, tmrm_subject_map* map)
{
    tmrm_proxy* p;
    /* This function assumes that the bottom proxy has the label 0 */
    p = tmrm_storage_pgsql_proxy_by_label(storage, map, "0");
    if (p == NULL) {
        /* Create a proxy without properties */
        p = tmrm_storage_pgsql_proxy_create(storage, map);
    }
    return p;
}


static int
tmrm_storage_pgsql_merge(tmrm_storage* storage, tmrm_subject_map* map)
{
    /*
       while (1) {
       SELECT p1.id, p2.id FROM proxy p1, proxy p2 WHERE p1.hash=p2.hash
       if (!num_rows) break;
       tmrm_proxy_pgsql_proxy_merge(storage, p1, p2);
       }
     */
    return 0;
}


/* Internal function: Merge p2 into p1. Asserts that p2 and p1 have the same set of properties */
static int
tmrm_storage_pgsql_proxy_merge(tmrm_storage* storage, tmrm_proxy* p1, tmrm_proxy* p2)
{
    /*
       if p1 == p2 return 0;
       UPDATE property SET key = p1 WHERE key = p2;
       UPDATE property SET value = p1 WHERE value = p2;
       DELETE FROM property WHERE proxy = p2;
       DELETE FROM proxy WHERE id = p2
     */
    return 0;
}


static tmrm_proxy*
tmrm_storage_pgsql_proxy_create(tmrm_storage* storage, tmrm_subject_map* map)
{
    tmrm_proxy* new_proxy;
    tmrm_label proxy;

    new_proxy = (tmrm_proxy*)TMRM_CALLOC(tmrm_proxy, 1, sizeof(tmrm_proxy));
    if (!new_proxy) {
        return NULL;
    }
    proxy = _proxy_get_new_id(storage);
    if (proxy == 0 || !(new_proxy = _create_proxy_struct(map, proxy))) {
        TMRM_FREE(tmrm_proxy, new_proxy);
        return NULL;
    }
    if (_create_proxy(storage, proxy)) {
        TMRM_FREE(tmrm_proxy, new_proxy);
        return NULL;
    }
    return new_proxy;
}


/* Updates the hash that is used to determine if two proxies
   are the same (proxy.hash). The function does some obscure
   but well-defined MD5-calucations. The better options would
   probably be a stored procedure.

   Basically, the result is the MD5 hash of all MD5 hashes of
   the proxie's properties ordered by key, value.

   The function is part of the backend to allow more efficient
   implementations, but maybe a fallback-implementation should
   be provided in the proxy object.

   Calculation of the hash:
   - Retrieve all properties ordered by key, value
   - Combine hashes into one MD5 hash
   - Update the hash.
 */
static int
tmrm_storage_pgsql_proxy_update(tmrm_storage* s, tmrm_proxy* p)
{
    char upd_statement[] = "UPDATE proxy SET hash=MD5('%s') WHERE id=%d";
    char sel_statement[] = "SELECT MD5(TEXT(key) || '-' || "
        "COALESCE(TEXT(value), '') || '-' || "
        "COALESCE(value_literal,'') || '-' || "
        "COALESCE(datatype,'')) "
        "FROM property WHERE proxy=%d "
        "ORDER BY key, value, value_literal";
    char *query, *md5s;
    size_t len, md5len;
    int num_rows, i;
    PGresult* res;
    ExecStatusType status;

    /* update proxy.hash */
    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return 1;

    /* Retrieve the MD5 hash of all properties */
    len = strlen(sel_statement) + 1 * INT_DIGITS;
    if (!(query = (char*)TMRM_MALLOC(cstring, len+1))) {
        return 1;
    }
    (void)snprintf(query, len, sel_statement, p->label);
    if(!(res = PQexec(c->conn, query))) {
        fprintf(stdout, "postgresql select proxy keys failed: %s\n",
                PQresultErrorMessage(res));
        TMRM_FREE(cstring, query);
        PQclear(res);
        return 1;
    }
    TMRM_FREE(cstring, query);

    status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK) {
        fprintf(stdout, "postgresql failed: '%s': %s / %s\n",
                query, PQresStatus(status), PQresultErrorMessage(res));
        PQclear(res);
        return 1;
    }
    num_rows = PQntuples(res);
    if (num_rows < 1) {
        PQclear(res);
        return 1;
    }
    md5len = 32 * (size_t)num_rows + 1;
    if (!(md5s = (char*)TMRM_MALLOC(cstring, md5len+1))) {
        PQclear(res);
        return 1;
    }
    md5s[0] = '\0';
    for (i = 0; i < num_rows; i++) {
        (void)strncat(md5s, PQgetvalue(res, i, 0), md5len - strlen(md5s) - 1);
    }
    PQclear(res);
    /*TMRM_DEBUG2("String with all MD5-hashes: '%s'\n", md5s);*/

    /* Update the proxy's hash column with the new value */
    len = strlen(upd_statement) + 1 * INT_DIGITS + md5len;
    if (!(query = (char*)TMRM_MALLOC(cstring, len+1))) {
        TMRM_FREE(cstring, md5s);
        return 1;
    }
    /* TODO: Should we quote md5s? We know that only MD5s are involved */
    (void)snprintf(query, len, upd_statement, md5s, p->label);
    TMRM_FREE(cstring, md5s);
    if(!(res = PQexec(c->conn, query))) {
        fprintf(stdout, "postgresql select proxy keys failed: %s\n",
                PQresultErrorMessage(res));
        TMRM_FREE(cstring, query);
        PQclear(res);
        return 1;
    }
    TMRM_FREE(cstring, query);

    return 0;
}


static int
tmrm_storage_pgsql_add_property(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key, tmrm_proxy* value)
{
    char statement[] = "INSERT INTO property (proxy, key, value) VALUES (%d, %d, %d)";
    char* query;
    size_t len;
    int ret;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return 1;

    len = strlen(statement) + 3 * INT_DIGITS;
    if (!(query =(char*)TMRM_MALLOC(cstring, len+1))) {
        return 1;
    }
    (void)snprintf(query, len, statement, p->label, key->label, value->label);
    ret = _exec_sql(s, query);
    TMRM_FREE(cstring, query);
    return ret;
}

static int
tmrm_storage_pgsql_add_property_literal(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key, tmrm_literal* value)
{
    char statement[] = "INSERT INTO property (proxy, key, value_literal, datatype) VALUES (%d, %d, $1, $2)";
    char* query;
    PGresult* res;
    ExecStatusType status;
    const char* paramValues[2];
    size_t len;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return 1;

    len = strlen(statement) + 2 * INT_DIGITS;
    if(!(query=(char*)TMRM_MALLOC(cstring, len+1))) {
        return 1;
    }
    (void)snprintf(query, len, statement, (int)p->label, (int)key->label);

    paramValues[0] = (char*)(value->value);
    paramValues[1] = (char*)(value->datatype);
    if(!(res = PQexecParams(c->conn, (const char*)query, 2,
                    NULL /* Let the backend deduce the param type */,
                    paramValues,
                    NULL, /* don’t need param lengths since text */ 
                    NULL, /* default to all text params */ 
                    0 /* ask for text results */))) {
        fprintf(stdout, "postgresql insert into proxy failed: %s\n",
                PQresultErrorMessage(res));
        PQclear(res);
        return 1;
    }

    TMRM_FREE(cstring, query);
    status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK) {
        fprintf(stdout, "postgresql insert into proxy failed: %s / %s\n",
                PQresStatus(status), PQresultErrorMessage(res));
        PQclear(res);
        return 1;
    }
    PQclear(res);

    return 0;
}

/* Not tested yet. TODO: Write test case */
static int
tmrm_storage_pgsql_proxy_remove_properties_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key)
{
    char statement[] = "DELETE FROM property WHERE proxy=%d AND key=%d";
    char* query;
    size_t len;
    int ret;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return 1;

    len = strlen(statement) + 2 * INT_DIGITS;
    if(!(query=(char*)TMRM_MALLOC(cstring, len+1))) {
        return 1;
    }
    (void)snprintf(query, len, statement, (int)p->label, (int)key->label);
    ret = _exec_sql(s, query);
    TMRM_FREE(cstring, query);
    return ret;
}

static int
tmrm_storage_pgsql_proxy_remove(tmrm_storage* s, const tmrm_proxy* p)
{
    char statement[] = "DELETE FROM property WHERE proxy=%d OR key=%d OR value=%d";
    char statement2[] = "DELETE FROM proxy WHERE id=%d";
    char* query;
    size_t len;

    len = strlen(statement) + 3 * INT_DIGITS;
    if(!(query=(char*)TMRM_MALLOC(cstring, len+1))) {
        return 1;
    }
    (void)snprintf(query, len, statement, (int)p->label, (int)p->label, (int)p->label);
    if (_exec_sql(s, query) != 0) {
        TMRM_FREE(cstring, query);
        return 1;
    }
    TMRM_FREE(cstring, query);

    len = strlen(statement2) + 1 * INT_DIGITS;
    if(!(query=(char*)TMRM_MALLOC(cstring, len+1))) {
        return 1;
    }
    (void)snprintf(query, len, statement2, (int)p->label);

    if (_exec_sql(s, query) != 0) {
        TMRM_FREE(cstring, query);
        return 1;
    }

    return 0;
}


static tmrm_iterator*
tmrm_storage_pgsql_proxy_properties(tmrm_storage* s, tmrm_proxy* p)
{
    char statement[] = "SELECT key, value, value_literal, datatype FROM property WHERE proxy=%d";
    char *query;
    size_t len;
    PGresult* res;
    ExecStatusType status;
    tmrm_iterator *iterator;
    tmrm_storage_pgsql_iterator_context *context;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) {
        return NULL;
    }
    len = strlen(statement) + 1 * INT_DIGITS;
    if(!(query=(char*)TMRM_MALLOC(cstring, len+1))) {
        return NULL;
    }
    (void)snprintf(query, len, statement, (int)p->label);

    if(!(res = PQexec(c->conn, query))) {
        fprintf(stdout, "postgresql select proxy keys failed: %s\n",
                PQresultErrorMessage(res));
        TMRM_FREE(cstring, query);
        PQclear(res);
        return NULL;
    }
    TMRM_FREE(cstring, query);

    status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK) {
        fprintf(stdout, "postgresql failed: '%s': %s / %s\n",
                query, PQresStatus(status), PQresultErrorMessage(res));
        PQclear(res);
        return NULL;
    }

    context = (tmrm_storage_pgsql_iterator_context*)
        TMRM_CALLOC(tmrm_storage_pgsql_iterator_context, 1,
                sizeof(tmrm_storage_pgsql_iterator_context));

    if (!context) {
        PQclear(res);
        return NULL;
    }

    context->subject_map = p->subject_map;
    context->res = res;
    context->num_rows = PQntuples(res);
    context->current_row = 0;

    /* TMRM_DEBUG2("Found %d rows\n", context->num_rows); */

    iterator = tmrm_iterator_new(s->subject_map_sphere, (void*)context, 
            tmrm_storage_pgsql_list_next,
            tmrm_storage_pgsql_list_end,
            tmrm_storage_pgsql_property_get_element,
            tmrm_storage_pgsql_list_free);

    /* Note that we don't have to call PQclear(res) here. */
    return iterator;
}


static tmrm_proxy*
tmrm_storage_pgsql_proxy_by_label(tmrm_storage* s, tmrm_subject_map* map,
        const char* label)
{
    char statement[] = "SELECT id FROM proxy WHERE id=$1";
    PGresult* res;
    ExecStatusType status;
    const char* paramValues[1];
    char *proxy_id_str;
    int proxy_id;
    tmrm_proxy* new_proxy;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return NULL;

    /* Check that label is valid */
    if (atoi(label) == 0 && (strlen(label) > 0 && label[0] != '0' || strlen(label) == 0)) {
        return NULL;
    }

    paramValues[0] = label;
    if(!(res = PQexecParams(c->conn, (const char*)&statement, 1 /* one param */,
                    NULL /* Let the backend deduce the param type */,
                    paramValues,
                    NULL, /* don’t need param lengths since text */ 
                    NULL, /* default to all text params */ 
                    0 /* ask for text results */))) {
        fprintf(stdout, "postgresql select proxy by label failed: %s\n",
                PQresultErrorMessage(res));
        PQclear(res);
        return NULL;
    }

    status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK) {
        fprintf(stdout, "postgresql select proxy by label failed: %s / %s\n",
                PQresStatus(status), PQresultErrorMessage(res));
        PQclear(res);
        return NULL;
    }
    if (PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }

    proxy_id_str = PQgetvalue(res, 0, 0);
    proxy_id = atoi(proxy_id_str);

    /*TMRM_DEBUG3("get_proxy_by_id found id '%s' (%d)\n",
      proxy_id_str, proxy_id);*/

    PQclear(res);

    /* Now that we have the proxy id, we can create the proxy object */
    new_proxy = _create_proxy_struct(map, proxy_id);

    return new_proxy;
}

static tmrm_iterator*
tmrm_storage_pgsql_proxies(tmrm_storage* s, tmrm_subject_map* map)
{
    char statement[] = "SELECT id FROM proxy ORDER BY id";

    return _iterator_by_sql_query(s, map, (char*)&statement);
}

static char*
tmrm_storage_pgsql_proxy_label(tmrm_storage* s, tmrm_proxy* p)
{
    size_t len;
    char *label;

    len = 1 * INT_DIGITS;
    if (!(label = (char*)TMRM_MALLOC(cstring, len + 1))) {
        return NULL;
    }

    (void)snprintf(label, len, "%d", (int)p->label);
    return label;
}

static tmrm_iterator*
tmrm_storage_pgsql_proxy_keys(tmrm_storage* s, tmrm_proxy* p)
{
    char statement[] = "SELECT key FROM property WHERE proxy=%d";
    char *query;
    PGresult* res;
    ExecStatusType status;
    size_t len;
    tmrm_iterator *iterator;
    tmrm_storage_pgsql_iterator_context *context;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return NULL;

    len = strlen(statement) + 1 * INT_DIGITS;
    if (!(query = (char*)TMRM_MALLOC(cstring, len + 1))) {
        return NULL;
    }

    (void)snprintf(query, len, statement, (int)p->label);

    if(!(res = PQexec(c->conn, query))) {
        fprintf(stdout, "postgresql select proxy keys failed: %s\n",
                PQresultErrorMessage(res));
        TMRM_FREE(cstring, query);
        PQclear(res);
        return NULL;
    }
    TMRM_FREE(cstring, query);

    status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK) {
        fprintf(stdout, "postgresql select proxy keys failed: %s / %s\n",
                PQresStatus(status), PQresultErrorMessage(res));
        PQclear(res);
        return NULL;
    }

    context = (tmrm_storage_pgsql_iterator_context*)
        TMRM_CALLOC(tmrm_storage_pgsql_iterator_context, 1,
                sizeof(tmrm_storage_pgsql_iterator_context));

    if (!context) {
        PQclear(res);
        return NULL;
    }

    context->subject_map = p->subject_map;
    context->res = res;
    context->num_rows = PQntuples(res);
    context->current_row = 0;

    TMRM_DEBUG2("Found %d row(s)\n", context->num_rows);

    iterator = tmrm_iterator_new(s->subject_map_sphere, (void*)context, 
            tmrm_storage_pgsql_list_next,
            tmrm_storage_pgsql_list_end,
            tmrm_storage_pgsql_proxy_list_get_element,
            tmrm_storage_pgsql_list_free);

    /* Note that we don't have to call PQclear(res) here. */
    return iterator;
}

static tmrm_iterator*
tmrm_storage_pgsql_proxy_values_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key)
{
    char statement[] = "SELECT value, value_literal, datatype FROM property WHERE proxy=%d";
    char *query;
    PGresult* res;
    ExecStatusType status;
    size_t len;
    tmrm_iterator *iterator;
    tmrm_storage_pgsql_iterator_context *context;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return NULL;

    len = strlen(statement) + 1 * INT_DIGITS;
    if (!(query = (char*)TMRM_MALLOC(cstring, len + 1))) {
        return NULL;
    }

    (void)snprintf(query, len, statement, (int)p->label);

    if(!(res = PQexec(c->conn, query))) {
        fprintf(stdout, "postgresql select proxy keys failed: %s\n",
                PQresultErrorMessage(res));
        TMRM_FREE(cstring, query);
        PQclear(res);
        return NULL;
    }
    TMRM_FREE(cstring, query);

    status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK) {
        fprintf(stdout, "postgresql select proxy keys failed: %s / %s\n",
                PQresStatus(status), PQresultErrorMessage(res));
        PQclear(res);
        return NULL;
    }

    context = (tmrm_storage_pgsql_iterator_context*)
        TMRM_CALLOC(tmrm_storage_pgsql_iterator_context, 1,
                sizeof(tmrm_storage_pgsql_iterator_context));

    if (!context) {
        PQclear(res);
        return NULL;
    }

    context->subject_map = p->subject_map;
    context->res = res;
    context->num_rows = PQntuples(res);
    context->current_row = 0;

    TMRM_DEBUG2("Found %d rows\n", context->num_rows);

    iterator = tmrm_iterator_new(s->subject_map_sphere, (void*)context, 
            tmrm_storage_pgsql_list_next,
            tmrm_storage_pgsql_list_end,
            tmrm_storage_pgsql_value_list_get_element,
            tmrm_storage_pgsql_list_free);

    /* Note that we don't have to call PQclear(res) here. */
    return iterator;
}

static tmrm_iterator*
tmrm_storage_pgsql_proxy_is_value_by_key(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* key)
{
    char statement[] = "SELECT proxy FROM property WHERE key=%d AND value=%d";
    char* query;
    size_t len;
    tmrm_iterator* iterator;

    len = strlen(statement) + 2 * INT_DIGITS;
    if(!(query = (char*)TMRM_MALLOC(cstring, len + 1))) {
        return NULL;
    }
    (void)snprintf(query, len, statement, (int)key->label, (int)p->label);
    iterator = _iterator_by_sql_query(s, p->subject_map, query);
    TMRM_FREE(cstring, query);
    return iterator;
}

static tmrm_iterator*
tmrm_storage_pgsql_proxy_keys_by_value(tmrm_storage* s, tmrm_proxy* p)
{
    char statement[] = "SELECT key FROM property WHERE value=%d";
    char* query;
    size_t len;
    tmrm_iterator* iterator;

    len = strlen(statement) + 1 * INT_DIGITS;
    if(!(query = (char*)TMRM_MALLOC(cstring, len + 1))) {
        return NULL;
    }
    (void)snprintf(query, len, statement, (int)p->label);
    iterator = _iterator_by_sql_query(s, p->subject_map, query);
    TMRM_FREE(cstring, query);
    return iterator;
}

static tmrm_iterator*
tmrm_storage_pgsql_literal_keys_by_value(tmrm_storage* s, tmrm_literal* lit, tmrm_subject_map* map)
{
    char statement[] = "SELECT key FROM property WHERE value_literal=$1 AND "
        "datatype=$2";
    tmrm_iterator* iterator;
    const char* param_values[2];

    /* FIXME: Decode UTF-8? */
    param_values[0] = (char*)tmrm_literal_value(lit);
    param_values[1] = (char*)tmrm_literal_datatype(lit);

    iterator = _iterator_by_sql_query_params(s, map,
            (const char*)&statement, param_values, 2);
    return iterator;
}

static tmrm_iterator*
tmrm_storage_pgsql_literal_is_value_by_key(tmrm_storage* s, tmrm_literal* lit, tmrm_proxy* key)
{
    char statement[] = "SELECT proxy FROM property WHERE key=%d AND value_literal=$1 AND datatype=$2";
    char *query;
    size_t len;
    tmrm_iterator* iterator;
    const char* param_values[2];

    len = strlen(statement) + 1 * INT_DIGITS;
    if(!(query = (char*)TMRM_MALLOC(cstring, len + 1))) {
        return NULL;
    }

    /* FIXME: Decode UTF-8? */
    param_values[0] = (char*)tmrm_literal_value(lit);
    param_values[1] = (char*)tmrm_literal_datatype(lit);

    (void)snprintf(query, len, statement, (int)key->label);

    iterator = _iterator_by_sql_query_params(s, key->subject_map,
            (const char*)query, param_values, 2);
    TMRM_FREE(cstring, query);
    return iterator;
}


/**
 * Internal short cut function for the previous function.
 * Returns a proxy that has a property where lit is the value and key is
 * the key. This function might be moved to the public module.
 *
 * @returns One of the matching proxies or NULL if no proxy is found
 * (or a error occurs).
 * @todo Implement better error handling
 * @see tmrm_storage_pgsql_literal_is_value_by_key
 */
static tmrm_proxy*
tmrm_storage_pgsql_proxy_by_literal(tmrm_storage* s, tmrm_literal* lit, tmrm_proxy* key)
{
    tmrm_iterator *it;
    tmrm_object *obj;

    it = tmrm_storage_pgsql_literal_is_value_by_key(s, lit, key);
    if (!it) {
        TMRM_DEBUG1("internal error\n");
        return NULL;
    } 
    if (tmrm_iterator_end(it)) {
        TMRM_DEBUG1("proxy not found\n");
        return NULL;
    }
    obj = tmrm_iterator_get_object(it);
    if (!obj) return NULL;
    return tmrm_object_to_proxy(obj);
}


/**
 * Helper function to iterate over a list of proxies.
 */
static int
tmrm_storage_pgsql_list_next(void* context)
{
    tmrm_storage_pgsql_iterator_context *c;
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(context, void, -1);
    c = (tmrm_storage_pgsql_iterator_context*)context;

    if (c->current_row < c->num_rows) {
        c->current_row++;
        return 0;
    }
    return 1;
}

/**
 * Helper function to iterate over a list of proxies.
 */
static int
tmrm_storage_pgsql_list_end(void* context)
{
    tmrm_storage_pgsql_iterator_context* c;
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(context, void, -1);
    c = (tmrm_storage_pgsql_iterator_context*)context;
    if (c->current_row < c->num_rows) {
        return 0;
    }
    return 1;
}

/**
 * Helper function to iterate over a list of proxies.
 */
static tmrm_object*
tmrm_storage_pgsql_proxy_list_get_element(void* context, tmrm_iterator_flag flag)
{
    char *proxy_id_str;
    int proxy_id;
    tmrm_storage_pgsql_iterator_context *c;
    tmrm_proxy *new_proxy;
    tmrm_object* obj;

    c = (tmrm_storage_pgsql_iterator_context*)context;

    /* TMRM_DEBUG2("Trying to fetch row %d\n", c->current_row); */
    proxy_id_str = PQgetvalue(c->res, c->current_row, 0);
    proxy_id = atoi(proxy_id_str);

    /* TMRM_DEBUG2("get_element(): Found proxy with id %d\n", proxy_id); */

    new_proxy = (tmrm_proxy*)TMRM_CALLOC(tmrm_proxy, 1, sizeof(tmrm_proxy));
    if (!new_proxy) {
        return NULL;
    }
    if (c->subject_map == NULL) TMRM_DEBUG1("c->subject_map == NULL\n");
    new_proxy->type = TMRM_TYPE_PROXY;
    new_proxy->label = proxy_id;
    new_proxy->subject_map = c->subject_map;
    if (new_proxy->subject_map == NULL) TMRM_DEBUG1("new_proxy->subject_map == NULL\n");
    
    obj = tmrm_proxy_to_object(new_proxy);
    return obj;
}

/**
 * Helper function to iterate over a list of proxies and/or values.
 */
static tmrm_object*
tmrm_storage_pgsql_value_list_get_element(void* context, tmrm_iterator_flag flag)
{
    const char *proxy_id_str;
    const tmrm_char_t *value_datatype_str;
    const tmrm_char_t *value_literal_str;
    int proxy_id;
    tmrm_storage_pgsql_iterator_context *c;
    tmrm_proxy *new_proxy;
    tmrm_literal *new_literal;
    tmrm_object *obj = NULL;

    c = (tmrm_storage_pgsql_iterator_context*)context;

    if (PQgetisnull(c->res, c->current_row, 0)) {
        /* it's a literal */
        /* 
         * TODO maybe we should add a function tmrm_literal_new that
         * takes the strings length as a size_t parameter (to allow
         * binary literals).
         * size = PQgetlength(c->res, c->current_row, 1);
         * size = PQgetlength(c->res, c->current_row, 2);
         */
        /* FIXME: Encode UTF-8? */
        value_literal_str = (tmrm_char_t*)PQgetvalue(c->res, c->current_row, 1);
        value_datatype_str = (tmrm_char_t*)PQgetvalue(c->res, c->current_row, 2);
        new_literal = tmrm_literal_new(value_literal_str, value_datatype_str);
        obj = tmrm_literal_to_object(new_literal);
        return obj;
    } else {
        /* it's a proxy */
        proxy_id_str = PQgetvalue(c->res, c->current_row, 0);
        proxy_id = atoi(proxy_id_str);
        /*TMRM_DEBUG2("get_element(): Found proxy with id %d\n", proxy_id);*/
        new_proxy = (tmrm_proxy*)TMRM_CALLOC(tmrm_proxy, 1, sizeof(tmrm_proxy));
        if (!new_proxy) {
            return NULL;
        }
        new_proxy->type = TMRM_TYPE_PROXY;
        new_proxy->label = proxy_id;
        new_proxy->subject_map = c->subject_map;
        obj = tmrm_proxy_to_object(new_proxy);
        return obj;
    }
}


/**
 * Helper function that retrieves a proxy or value/literal from the
 * properties-table.
 */
static tmrm_object*
tmrm_storage_pgsql_property_get_element(void* context, tmrm_iterator_flag flag)
{
    const char *proxy_id_str;
    const tmrm_char_t *value_datatype_str;
    const tmrm_char_t *value_literal_str;
    int proxy_id;
    tmrm_object *obj = NULL;
    tmrm_storage_pgsql_iterator_context *c;
    tmrm_proxy *new_proxy;
    void *result = NULL;
    tmrm_literal *lit;
    c = (tmrm_storage_pgsql_iterator_context*)context;

    switch (flag) {
        case TMRM_ITERATOR_GET_METHOD_GET_KEY:
            {
                /* it's a proxy */
                proxy_id_str = PQgetvalue(c->res, c->current_row, 0);
                proxy_id = atoi(proxy_id_str);
                new_proxy = _create_proxy_struct(c->subject_map, proxy_id);
                obj = tmrm_proxy_to_object(new_proxy);
                return obj;
            }
            break;
        case TMRM_ITERATOR_GET_METHOD_GET_VALUE:
            {
                if (PQgetisnull(c->res, c->current_row, 1)) {
                    /* it's a literal */
                    /* FIXME: Encode UTF-8? */
                    value_literal_str = (tmrm_char_t*)PQgetvalue(c->res, c->current_row, 2);
                    value_datatype_str = (tmrm_char_t*)PQgetvalue(c->res, c->current_row, 3);
                    lit = tmrm_literal_new(
                            value_literal_str,
                            value_datatype_str);
                    obj = tmrm_literal_to_object(lit);
                    return obj;
                } else {
                    /* it's a proxy */
                    proxy_id_str = PQgetvalue(c->res, c->current_row, 1);
                    proxy_id = atoi(proxy_id_str);
                    new_proxy = _create_proxy_struct(c->subject_map, proxy_id);
                    obj = tmrm_proxy_to_object(new_proxy);
                    return obj;
                }
            }
            break;
        default:
            result = NULL;
            break;
    }
    return result;
}


/**
 * Helper function to iterate over a list of proxies.
 */
static void
tmrm_storage_pgsql_list_free(void* context)
{
    tmrm_storage_pgsql_iterator_context *c;
    c = (tmrm_storage_pgsql_iterator_context*)context;

    PQclear(c->res);
    /* TMRM_FREE(xxx_context, c); */
}

static int
tmrm_storage_pgsql_proxy_add_type(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* type)
{
    tmrm_proxy *anon;

    anon = tmrm_storage_pgsql_proxy_create(s, p->subject_map);
    if (!anon) 
        return -1;

    tmrm_storage_pgsql_add_property(s, anon, p->subject_map->type, type);
    tmrm_storage_pgsql_add_property(s, anon, p->subject_map->instance, p);
    tmrm_proxy_free(anon);

    return 0;
}

static int
tmrm_storage_pgsql_proxy_add_superclass(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* superclass)
{
    tmrm_proxy *anon;

    anon = tmrm_storage_pgsql_proxy_create(s, p->subject_map);
    if (!anon) 
        return -1;

    tmrm_storage_pgsql_add_property(s, anon, p->subject_map->superclass, superclass);
    tmrm_storage_pgsql_add_property(s, anon, p->subject_map->subclass, p);
    tmrm_proxy_free(anon);
    return 0;
}


/* Helper function for superclass-subclass or type-instance relations */
static tmrm_iterator*
tmrm_storage_pgsql_proxy_direct_class(tmrm_storage* s, tmrm_proxy* p, tmrm_proxy* a, tmrm_proxy* b)
{
    char statement[] = "SELECT p2.value FROM property p2, property p1 WHERE "
        "p1.proxy=p2.proxy AND p2.key=%d AND p1.key=%d AND p1.value=%d";
    char* query;
    size_t len;
    tmrm_iterator* iterator;

    len = strlen(statement) + 3 * INT_DIGITS;
    if(!(query = (char*)TMRM_MALLOC(cstring, len + 1))) {
        return NULL;
    }
    (void)snprintf(query, len, statement,
        (int)a->label,
        (int)b->label,
        (int)p->label);

    iterator = _iterator_by_sql_query(s, p->subject_map, query);
    TMRM_FREE(cstring, query);
    return iterator;
}


static tmrm_iterator*
tmrm_storage_pgsql_proxy_direct_subclasses(tmrm_storage* s, tmrm_proxy* p)
{
    /* assert(p->subject_map->subclass); 
       assert(p->subject_map->superclass); */
    return tmrm_storage_pgsql_proxy_direct_class(s, p,
        p->subject_map->subclass, p->subject_map->superclass);
}


static tmrm_iterator*
tmrm_storage_pgsql_proxy_direct_superclasses(tmrm_storage* s, tmrm_proxy* p)
{
    /* assert(p->subject_map->subclass); 
       assert(p->subject_map->superclass); */
    return tmrm_storage_pgsql_proxy_direct_class(s, p,
        p->subject_map->superclass, p->subject_map->subclass);
}


static tmrm_iterator*
tmrm_storage_pgsql_proxy_direct_types(tmrm_storage* s, tmrm_proxy* p)
{
    /* assert(p->subject_map->type); 
       assert(p->subject_map->instance); */
    return tmrm_storage_pgsql_proxy_direct_class(s, p,
        p->subject_map->type, p->subject_map->instance);
}


static tmrm_iterator*
tmrm_storage_pgsql_proxy_direct_instances(tmrm_storage* s, tmrm_proxy* p)
{
    /* assert(p->subject_map->type); 
       assert(p->subject_map->instance); */
    return tmrm_storage_pgsql_proxy_direct_class(s, p,
        p->subject_map->instance, p->subject_map->type);
}


static tmrm_label
_proxy_get_new_id(tmrm_storage* s)
{
    char statement[]="SELECT nextval('proxy_id_seq')";
    char *proxy_id_str;
    tmrm_label proxy_id;
    PGresult* res;
    ExecStatusType status;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return 0;

    if(!(res = PQexec(c->conn, (char*)statement))) {
        fprintf(stdout, "postgresql nextval('proxy_id_seq') failed: %s\n",
                PQresultErrorMessage(res));
        PQclear(res);
        return 0;
    }

    status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK) {
        fprintf(stdout, "postgresql nextval('proxy_id_seq') failed: %s / %s\n",
                PQresStatus(status), PQresultErrorMessage(res));
        PQclear(res);
        return 0;
    }

    if (PQntuples(res) == 0) {
        PQclear(res);
        return 0;
    }

    proxy_id_str = PQgetvalue(res, 0, 0);
    proxy_id = atol(proxy_id_str);

    /*TMRM_DEBUG2("proxy_get_new_id found id '%d'\n", proxy_id);*/
    PQclear(res);

    return proxy_id;
}

static int
_create_proxy(tmrm_storage* s, tmrm_label id)
{
    char statement[] = "INSERT INTO proxy (id) VALUES (%d)";
    char* query;
    int ret;
    size_t len;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) return 1;

    len = strlen(statement) + INT_DIGITS;
    if(!(query=(char*)TMRM_MALLOC(cstring, len+1))) {
        return 1;
    }

    (void)snprintf(query, len, statement, (int)id);

    ret = _exec_sql(s, query);

    TMRM_FREE(cstring, query);
    return ret;
}

/**
* Allocates memory for a new tmrm_proxy structure.
*/
static tmrm_proxy*
_create_proxy_struct(tmrm_subject_map* m, tmrm_label label)
{
    tmrm_proxy *p;

    p = (tmrm_proxy*)TMRM_CALLOC(tmrm_proxy, 1, sizeof(tmrm_proxy));
    if (!p) {
        return NULL;
    }
    p->type = TMRM_TYPE_PROXY;
    p->label = label;
    p->subject_map = m;
    return p;
}

/**
* Executes a query without parameters. Returns 0 on success, or a non-zero
* value on failure.
*/
static int
_exec_sql(tmrm_storage* s, const char* query)
{
    PGresult* res;
    ExecStatusType status;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) {
        return 1;
    }

    if(!(res = PQexec(c->conn, query))) {
        fprintf(stdout, "postgresql query failed: '%s': %s\n",
            query,
            PQresultErrorMessage(res));
        PQclear(res);
        return 1;
    }

    status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK) {
        fprintf(stdout, "postgresql query failed: '%s', '%s' / '%s'\n",
            query, PQresStatus(status), PQresultErrorMessage(res));
        PQclear(res);
        return 1;
    }
    PQclear(res);

    return 0;
}

static tmrm_iterator*
_iterator_by_sql_query(tmrm_storage* s, tmrm_subject_map* subject_map, const char* query)
{
    PGresult* res;
    ExecStatusType status;
    tmrm_iterator *iterator;
    tmrm_storage_pgsql_iterator_context *context;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) {
        return NULL;
    }

    if(!(res = PQexec(c->conn, query))) {
        fprintf(stdout, "postgresql select proxy keys failed: %s\n",
            PQresultErrorMessage(res));
        PQclear(res);
        return NULL;
    }

    status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK) {
        fprintf(stdout, "postgresql failed: '%s': %s / %s\n",
            query, PQresStatus(status), PQresultErrorMessage(res));
        PQclear(res);
        return NULL;
    }

    context = (tmrm_storage_pgsql_iterator_context*)
        TMRM_CALLOC(tmrm_storage_pgsql_iterator_context, 1,
            sizeof(tmrm_storage_pgsql_iterator_context));

    if (!context) {
        PQclear(res);
        return NULL;
    }

    context->subject_map = subject_map;
    context->res = res;
    context->num_rows = PQntuples(res);
    context->current_row = 0;

    /* TMRM_DEBUG2("Found %d rows\n", context->num_rows); */

    iterator = tmrm_iterator_new(s->subject_map_sphere, (void*)context, 
        tmrm_storage_pgsql_list_next,
        tmrm_storage_pgsql_list_end,
        tmrm_storage_pgsql_proxy_list_get_element,
        tmrm_storage_pgsql_list_free);

    /* Note that we don't have to call PQclear(res) here. */
    return iterator;
}


static tmrm_iterator*
_iterator_by_sql_query_params(tmrm_storage* s, tmrm_subject_map *subject_map,
        const char* query, const char* const *param_values, int param_count)
{
    PGresult* res;
    ExecStatusType status;
    tmrm_iterator *iterator;
    tmrm_storage_pgsql_iterator_context *context;

    tmrm_storage_pgsql_context* c = (tmrm_storage_pgsql_context*)s->context;
    if (!c) {
        return NULL;
    }

    if(!(res = PQexecParams(c->conn, query, param_count,
            NULL /* Let the backend deduce the param type */,
            param_values,
            NULL, /* don’t need param lengths since text */ 
            NULL, /* default to all text params */ 
            0 /* ask for text results */))) {
        fprintf(stdout, "postgresql select proxy keys failed: %s\n",
            PQresultErrorMessage(res));
        PQclear(res);
        return NULL;
    }

    status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK) {
        fprintf(stdout, "postgresql failed: '%s': %s / %s\n",
            query, PQresStatus(status), PQresultErrorMessage(res));
        PQclear(res);
        return NULL;
    }

    context = (tmrm_storage_pgsql_iterator_context*)
        TMRM_CALLOC(tmrm_storage_pgsql_iterator_context, 1,
            sizeof(tmrm_storage_pgsql_iterator_context));

    if (!context) {
        PQclear(res);
        return NULL;
    }

    context->subject_map = subject_map;
    context->res = res;
    context->num_rows = PQntuples(res);
    context->current_row = 0;

    /* TMRM_DEBUG2("Found %d rows\n", context->num_rows); */

    iterator = tmrm_iterator_new(s->subject_map_sphere, (void*)context, 
        tmrm_storage_pgsql_list_next,
        tmrm_storage_pgsql_list_end,
        tmrm_storage_pgsql_proxy_list_get_element,
        tmrm_storage_pgsql_list_free);

    /* Note that we don't have to call PQclear(res) here. */
    return iterator;
}


static void
tmrm_storage_pgsql_register_factory(tmrm_storage_factory *factory)
{
    factory->init = tmrm_storage_pgsql_init;
    factory->free = tmrm_storage_pgsql_free;

    factory->bootstrap = tmrm_storage_pgsql_bootstrap;
    factory->remove = tmrm_storage_pgsql_remove;
    factory->bottom = tmrm_storage_pgsql_bottom;
    factory->merge = tmrm_storage_pgsql_merge;
    factory->proxy_create = tmrm_storage_pgsql_proxy_create;
    factory->proxy_update = tmrm_storage_pgsql_proxy_update;
    factory->add_property = tmrm_storage_pgsql_add_property;
    factory->add_property_literal = tmrm_storage_pgsql_add_property_literal;
    factory->proxy_remove_properties_by_key = tmrm_storage_pgsql_proxy_remove_properties_by_key;
    factory->proxy_properties = tmrm_storage_pgsql_proxy_properties;
    factory->proxy_remove = tmrm_storage_pgsql_proxy_remove;
    factory->proxy_by_label = tmrm_storage_pgsql_proxy_by_label;
    factory->proxies = tmrm_storage_pgsql_proxies;
    factory->proxy_label = tmrm_storage_pgsql_proxy_label;
    factory->proxy_keys = tmrm_storage_pgsql_proxy_keys;
    factory->proxy_values_by_key = tmrm_storage_pgsql_proxy_values_by_key;
    factory->proxy_is_value_by_key = tmrm_storage_pgsql_proxy_is_value_by_key;
    factory->proxy_keys_by_value = tmrm_storage_pgsql_proxy_keys_by_value;
    factory->literal_keys_by_value = tmrm_storage_pgsql_literal_keys_by_value;
    factory->literal_is_value_by_key = tmrm_storage_pgsql_literal_is_value_by_key;
    factory->proxy_add_type = tmrm_storage_pgsql_proxy_add_type;
    factory->proxy_add_superclass = tmrm_storage_pgsql_proxy_add_superclass;
    factory->proxy_direct_subclasses = tmrm_storage_pgsql_proxy_direct_subclasses;
    factory->proxy_direct_superclasses = tmrm_storage_pgsql_proxy_direct_superclasses;
    factory->proxy_direct_types = tmrm_storage_pgsql_proxy_direct_types;
    factory->proxy_direct_instances = tmrm_storage_pgsql_proxy_direct_instances;
}


void
tmrm_init_storage_pgsql(tmrm_subject_map_sphere *sms)
{
    tmrm_storage_register_factory(sms, "pgsql", "PostgreSQL storage module",
            &tmrm_storage_pgsql_register_factory);
}

