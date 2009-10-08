/*
 * tmrm_tests.c - Unit tests for libtmrm
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <libtmrm.h>
#include <libtmrm_config.h>
#include <tmrm_storage.h>
#include <tmrm_hash.h>
#include <tmrm_hash_internal.h>

#if STORAGE_POSTGRESQL
#include <libpq-fe.h>

/* Change these if you want to run the tests on a different
   setup */
#define POSTGRESQL_DB "host=localhost dbname=tmrm_test"
#define POSTGRESQL_OPTIONS "host='localhost',dbname='tmrm_test',user='jans'"

#define POSTGRESQL_DB_TEMPLATE "host=localhost dbname=template1"
#define POSTGRESQL_OPTIONS_TEMPLATE "host='localhost',dbname='tmrm_test2',user='jans',new='yes'"

#define POSTGRESQL_DBNAME "tmrm_test2"
#endif

void setup(void);
void teardown(void);

#if STORAGE_POSTGRESQL
PGconn* pg_conn;
void pgsql_new_storage_setup(void);
void pgsql_new_storage_teardown(void);
#endif

void
setup (void)
{
#if STORAGE_POSTGRESQL
    PGresult* res;

    pg_conn = PQconnectdb(POSTGRESQL_DB);
    if (PQstatus(pg_conn) != CONNECTION_OK) {
        fprintf(stdout, "Connection to postgresql database failed: %s\n",
            PQerrorMessage(pg_conn));
    }
    res = PQexec(pg_conn, "DELETE FROM type_instance");
    res = PQexec(pg_conn, "DELETE FROM superclass_subclass");
    res = PQexec(pg_conn, "DELETE FROM property");
    res = PQexec(pg_conn, "DELETE FROM proxy WHERE id>1");
#endif
    printf("\n===============================================================\n");
}

void
teardown (void)
{
    /*printf("teardown()\n");*/
#if STORAGE_POSTGRESQL
    if (pg_conn != NULL) {
        PQfinish(pg_conn);
    }
#endif
}

#if STORAGE_POSTGRESQL
void pgsql_new_storage_setup(void) {
    PGresult* res;

    printf("\n===============================================================\n");

    pg_conn = PQconnectdb(POSTGRESQL_DB_TEMPLATE);
    if (PQstatus(pg_conn) != CONNECTION_OK) {
        fprintf(stdout, "Connection to postgresql database failed: %s\n",
            PQerrorMessage(pg_conn));
    }
    /* try to remove the subject map that we're about to create 
       (in case it exists from failed previous tests) */
    res = PQexec(pg_conn, "DROP DATABASE " POSTGRESQL_DBNAME);
}

void pgsql_new_storage_teardown(void) {
    /* drop the test database */
    pgsql_new_storage_setup();
}
#endif


START_TEST (test_pgsql_storage)
{
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;

    printf("=> test_pgsql_storage\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);

    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST


START_TEST (test_clone_proxy)
{
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;
    tmrm_proxy *p, *copy;
    const char *label_p, *label_copy;

    printf("=> test_clone_proxy\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);

    m = tmrm_subject_map_new(sms, storage, "mymap");

    p = tmrm_subject_map_bottom(m);
    fail_if(p == NULL, "Could not get proxy bottom");
    copy = tmrm_proxy_clone(p);
    fail_if(copy == NULL, "Could not copy bottom proxy");

    label_p = tmrm_proxy_label(p);
    fail_if(label_p == NULL, "Could not get label of p");

    label_copy = tmrm_proxy_label(copy);
    fail_if(label_copy == NULL, "Could not get label of copy");

    fail_unless(strcmp(label_copy, label_p) == 0, "Labels not equal");

    tmrm_proxy_free(p);
    tmrm_proxy_free(copy);

    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST


START_TEST (test_create_proxy)
{
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;
    int res;
    tmrm_proxy *p1, *p2, *p3, *p4, *bottom, *key, *anon;
    tmrm_iterator *it;
    const char *label;
    tmrm_object *object;
    tmrm_list *list;
    tmrm_list_elmt *elem;
    tmrm_proxy *p;
    tmrm_object *obj;
    tmrm_multiset *set;
    int i; // counter for testing the results of iterators

    printf("=> test_create_proxy\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);

    m = tmrm_subject_map_new(sms, storage, "mymap");

    p4 = tmrm_proxy_by_label(m, "XYZ");
    fail_unless(p4 == NULL, "Did not find proxy XYZ");

    bottom = tmrm_subject_map_bottom(m);
    fail_if(bottom == NULL, "Not able to fetch the bottom proxy");

    TMRM_DEBUG1("Creating new proxy\n");
    p1 = tmrm_proxy_new(m);
    fail_unless(p1 != NULL, "Could not create p1");

    p2 = tmrm_proxy_new(m);
    fail_unless(p2 != NULL, "Could not create p1");

    p3 = tmrm_proxy_new(m);
    fail_unless(p3 != NULL, "Could not create p3");

    anon = tmrm_proxy_new(m);
    fail_if(anon == NULL, "Could not create proxy without label");

    label = tmrm_proxy_label(anon);
    fail_if(label == NULL, "Could not create auto-generated label");

    TMRM_DEBUG2("Auto-generated label = '%s'\n", label);

    /* Try to add a proxy with _bottom_ as key */
    res = tmrm_proxy_add_property(p1, bottom, bottom);
    fail_unless(res == 0, "Could not add property with bottom as key");

    res = tmrm_proxy_add_property(p2, p1, bottom);
    fail_unless(res == 0, "Could not add property with p1 as key");

    res = tmrm_proxy_add_property(p2, bottom, bottom);
    fail_unless(res == 0, "Could not add property with bottom as key");

    res = tmrm_proxy_add_type(p2, p1);
    fail_unless(res == 0, "Could not add type p1 to p2");

    res = tmrm_proxy_add_superclass(p2, p1);
    fail_unless(res == 0, "Could not add superclass p1 to p2");

    /* Retrieve subclasses */
    tmrm_subject_map_export_to_yaml(m, stdout);
    it = tmrm_proxy_direct_subclasses(p1);
    fail_if(it == NULL);

    /*fail_if(tmrm_object_get_type((tmrm_object*)it) != TMRM_TYPE_ITERATOR,
        "it is not an iterator");*/

    i = 0;
    while(!tmrm_iterator_end(it)) {
        obj = tmrm_iterator_get_value(it);
        fail_if(obj == NULL, "Could not get object");
        fail_if(tmrm_object_get_type(obj) != TMRM_TYPE_PROXY, "Invalid object type");
        p = tmrm_object_to_proxy(obj);
        TMRM_DEBUG2("====> subclass of p1: %s\n", tmrm_proxy_label(p));
        res = tmrm_iterator_next(it);
        fail_unless(res == 0, "Could not get next element");
        i++;
    }
    fail_unless(i == 1, "direct_subclasses(p1='%s') returned an incorrect count of"
        " proxies: %d", tmrm_proxy_label(p1), i);
    tmrm_iterator_free(it);

    list = tmrm_proxy_subclasses(p1);
    fail_if(list == NULL, "Could not retrieve subclass list of proxies");
    printf("list size = %d\n", tmrm_list_size(list));
    tmrm_list_free(list);

    /* Retrieve superclasses */
    it = tmrm_proxy_direct_superclasses(p2);
    fail_if(it == NULL);

    i = 0;
    while(!tmrm_iterator_end(it)) {
        res = tmrm_iterator_next(it);
        fail_unless(res == 0, "Could not get next element");
        i++;
    }
    fail_unless(i == 1, "direct_superclasses(p2) returned an incorrect count of"
        " proxies");
    tmrm_iterator_free(it);

    /* Retrieve types of p2 */
    it = tmrm_proxy_direct_types(p2);
    fail_if(it == NULL);

    i = 0;
    while(!tmrm_iterator_end(it)) {
        res = tmrm_iterator_next(it);
        fail_unless(res == 0, "Could not get next element");
        i++;
    }
    fail_unless(i == 1, "direct_types(p2) returned an incorrect count of"
        " proxies: %d", i);
    tmrm_iterator_free(it);

    /* Retrieve types of p1 */
    it = tmrm_proxy_direct_instances(p1);
    i = 0;
    while(!tmrm_iterator_end(it)) {
        res = tmrm_iterator_next(it);
        fail_unless(res == 0, "Could not get next element");
        i++;
    }
    fail_unless(i == 1, "direct_types(p2) returned an incorrect count of"
        " proxies: %d", i);
    tmrm_iterator_free(it);

    /* Retrieve an iterator over all keys of p2 */
    set = tmrm_proxy_keys(p2);
    fail_if(set == NULL);

    list = tmrm_multiset_as_list(set);
    fail_if(list == NULL, "Could not convert list to multiset");

    elem = tmrm_list_head(list);
    while (elem != NULL) {
        p = (tmrm_proxy*)tmrm_list_data(elem);
        /*TMRM_DEBUG2("Found element '%s'\n", tmrm_proxy_label(p));*/
        elem = tmrm_list_next(elem);
    }
    tmrm_list_free(list);

/*
    while (!tmrm_iterator_end(it)) {
        key = tmrm_iterator_get_object(it);
        fail_if(key == NULL, "Could not get proxy object");
        res = tmrm_iterator_next(it);
        fail_unless(res == 0, "Could not get next element");
        fail_if(label == NULL, "Could not get label");
        TMRM_DEBUG2("Found proxy with label %s\n", label);
    }
    fail_if(tmrm_iterator_end(it) == 0);
*/

    tmrm_multiset_free(set);

    /* Try to delete a proxy */
    res = tmrm_proxy_remove(p1);
    fail_unless(res == 0, "Could not delete proxy");

    /*tmrm_proxy_free(p1);*/
    tmrm_proxy_free(p2);
    tmrm_proxy_free(p3);
    tmrm_proxy_free(bottom);
    tmrm_subject_map_free(m);
    
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST

/**
Creates an property with a literal value
*/
START_TEST (test_create_property_literal)
{
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;
    int res;
    tmrm_proxy *proxy = NULL, *bottom = NULL;
    tmrm_literal *lit = NULL;
    const char *label;
    const char *lit_datatype;
    const char *lit_value;

    printf("=> test_create_property_literal\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);

    m = tmrm_subject_map_new(sms, storage, "mymap");

    bottom = tmrm_subject_map_bottom(m);
    fail_if(bottom == NULL, "Did not find proxy _bottom_");

    proxy = tmrm_proxy_new(m);
    fail_if(proxy == NULL, "Could not create proxy");

    lit = tmrm_literal_new("foobar", "http://www.w3.org/2001/XMLSchema#string");
    fail_if(lit == NULL, "Could not create literal");

    lit_datatype = tmrm_literal_datatype(lit);
    fail_if(lit_datatype == NULL, "Could not get literal datatype");
    fail_unless(strcmp(lit_datatype,
                "http://www.w3.org/2001/XMLSchema#string") == 0,
            "Got invalid literal datatype");

    lit_value = tmrm_literal_value(lit);
    fail_if(lit_value == NULL, "Could not get literal value");
    fail_unless(strcmp(lit_value, "foobar") == 0,
            "Got invalid literal value");

    tmrm_proxy_add_property_literal(proxy, bottom, lit);

    label = tmrm_proxy_label(proxy);
    fail_if(label == NULL, "Could not get proxy label");

    tmrm_literal_free(lit);

    tmrm_proxy_free(proxy);
    tmrm_proxy_free(bottom);

    tmrm_subject_map_free(m);
    
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST

/* Creates a proxy with some values (both proxy and literals)
   and loops through proxy_values_by_key
*/
START_TEST (test_values_by_key)
{
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;
    int res;
    tmrm_proxy *proxy, *bottom, *proxy_res;
    tmrm_literal *lit;
    tmrm_iterator *it;
    tmrm_object_type object_type;
    tmrm_multiset *set;
    tmrm_list *list;
    tmrm_list_elmt *elem;
    tmrm_object *obj;

    printf("=> test_values_by_key\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);

    m = tmrm_subject_map_new(sms, storage, "mymap");

    bottom = tmrm_subject_map_bottom(m);
    fail_if(bottom == NULL, "Did not find proxy _bottom_");

    proxy = tmrm_proxy_new(m);
    fail_if(proxy == NULL, "Could not create proxy");

    lit = tmrm_literal_new("foobar", "http://www.w3.org/2001/XMLSchema#string");
    fail_if(lit == NULL, "Could not create literal");

    res = tmrm_proxy_add_property_literal(proxy, bottom, lit);
    fail_unless(res == 0, "Could not add literal property");

    tmrm_literal_free(lit);

    res = tmrm_proxy_add_property(proxy, bottom, bottom);
    fail_unless(res == 0, "Could not add property");

    set = tmrm_proxy_values_by_key(proxy, bottom);
    fail_if(set == NULL, "Could not get multiset");

    list = tmrm_multiset_as_list(set);
    fail_if(list == NULL, "Could not convert list to multiset");

    elem = tmrm_list_head(list);
    while (elem != NULL) {
        obj = tmrm_list_data(elem);
        fail_if(obj == NULL, "Could not get value object");
        object_type = tmrm_object_get_type(obj);
        switch(object_type) {
            case TMRM_TYPE_PROXY: TMRM_DEBUG1("Found proxy\n"); break;
            case TMRM_TYPE_LITERAL: TMRM_DEBUG1("Found literal\n"); break;
            case TMRM_TYPE_ITERATOR: TMRM_DEBUG1("Found iterator\n"); break;
            default: fail("Found invalid object");
        }

        elem = tmrm_list_next(elem);
    }
    tmrm_list_free(list);
    tmrm_multiset_free(set);

    /* Try to list all properties of a proxy */
    it = tmrm_proxy_get_properties(proxy);
    fail_if(it == NULL, "Could not retrieve iterator over properties");
    printf("proxy '%s' {\n", tmrm_proxy_label(proxy));
    while(!tmrm_iterator_end(it)) {
        /* Get the key of the property: */
        obj = tmrm_iterator_get_key(it);
        fail_if(obj == NULL, "Could not get key");
        object_type = tmrm_object_get_type(obj);
        fail_if(object_type != TMRM_TYPE_PROXY, "Key has invalid type %d", (int)object_type);
        fail_unless(tmrm_object_is_proxy(obj), "Key has invalid type (tmrm_object_is_proxy)");

        /* proxy_res = (tmrm_proxy*)value; */
        proxy_res = tmrm_object_to_proxy(obj);

        fail_if(proxy_res == NULL, "Could not convert object into proxy object");
        printf("\t'%s': ", tmrm_proxy_label(proxy_res));
        tmrm_proxy_free(proxy_res);

        obj = tmrm_iterator_get_value(it);
        fail_if(obj == NULL, "Could not get value");
        object_type = tmrm_object_get_type(obj);
        fail_if(object_type != TMRM_TYPE_PROXY && object_type != TMRM_TYPE_LITERAL,
            "obj: object_type is neither proxy nor value");
        if (object_type == TMRM_TYPE_PROXY) {
            proxy_res = tmrm_object_to_proxy(obj);
            printf("'%s'\n", tmrm_proxy_label(proxy_res));
            tmrm_proxy_free(proxy_res);
        } else {
            lit = tmrm_object_to_literal(obj);
            printf("\"%s\"^^%s\n", tmrm_literal_value(lit), tmrm_literal_datatype(lit));
            tmrm_literal_free(lit);
        }

        res = tmrm_iterator_next(it);
        fail_unless(res == 0, "Could not get next element");
    }
    printf("}\n");

    tmrm_iterator_free(it);
    

    /* Try to remove all properties with the key bottom */
    res = tmrm_proxy_remove_properties_by_key(proxy, bottom);
    fail_unless(res == 0, "Could not remove properties");

    tmrm_proxy_free(proxy);
    tmrm_proxy_free(bottom);

    tmrm_subject_map_free(m);
    
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST

START_TEST (test_literal_keys_by_value)
{
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;
    tmrm_proxy *proxy, *p1, *p2, *bottom;
    tmrm_literal* lit;
    tmrm_multiset* res;
    tmrm_list* list;
    tmrm_list_elmt* elem;

    printf("=> test_literal_keys_by_value\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);
    m = tmrm_subject_map_new(sms, storage, "mymap");

    bottom = tmrm_subject_map_bottom(m);

    lit = tmrm_literal_new("43", "http://www.w3.org/2001/XMLSchema#string");

    /* add some proxies with literals */
    p1 = tmrm_proxy_new(m);
    tmrm_proxy_add_property_literal(p1, bottom, lit);

    p2 = tmrm_proxy_new(m);
    tmrm_proxy_add_property_literal(p2, p1, lit);
    tmrm_proxy_free(p2);
    tmrm_literal_free(lit);

    tmrm_literal_new("43", "http://www.w3.org/2001/XMLSchema#integer");
    p2 = tmrm_proxy_new(m);
    tmrm_proxy_add_property_literal(p2, bottom, lit);
    tmrm_proxy_free(p2);
    tmrm_literal_free(lit);

    tmrm_literal_new("43", "http://www.w3.org/2001/XMLSchema#string");

    /* retrieve a list of these proxies with tmrm_literal_keys_by_value() */
    res = tmrm_literal_keys_by_value(lit, m);
    fail_if(res == NULL, "Could not get multiset of keys");

    list = tmrm_multiset_as_list(res);
    fail_if(list == NULL, "Could not convert list to multiset");

    elem = tmrm_list_head(list);
    fail_if(elem == NULL, "Expected list element");

    while (elem != NULL) {
        TMRM_DEBUG1("Found list element\n");
    
        proxy = (tmrm_proxy*)tmrm_list_data(elem);
        fail_if(tmrm_object_get_type((tmrm_object*)proxy) != TMRM_TYPE_PROXY, "Invalid object type");
        fail_if(proxy == NULL, "Proxy must not be NULL");
        elem = tmrm_list_next(elem);
    }
    tmrm_list_free(list);

    tmrm_multiset_free(res);
    tmrm_literal_free(lit);
    tmrm_proxy_free(bottom);

    tmrm_subject_map_free(m);
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);

}
END_TEST

/** Tests the tmrm_proxy_subclasses() function */
START_TEST (test_proxy_subclasses)
{
    /* Creates a sample superclass/subclass hierachy:
       p4 is subclass of p3
       p4 is subclass of p2
       p2 is subclass of p1
       p3 is subclass of p1
       p3 is subclass of p0
       p0 is subclass of p4

       subclasses of p0 = (p0, p3, p4)
       subclasses of p1 = (p0, p1, p2, p3, p4)

       superclasses of p0 = {p0, p1, p3, p4}
     */
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;
    int res, i;
    tmrm_proxy *proxy[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    tmrm_proxy *p;
    tmrm_proxy *bottom;
    tmrm_literal *lit;
    tmrm_list *list;
    tmrm_iterator *it;
    tmrm_multiset *set;
    tmrm_list_elmt *elem;
    tmrm_object *obj;

    printf("=> test_proxy_subclasses\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);
    m = tmrm_subject_map_new(sms, storage, "mymap");

    bottom = tmrm_subject_map_bottom(m);
    fail_if(bottom == NULL, "Did not find proxy _bottom_");

    for (i = 0; i < 5; i++) {
        proxy[i] = tmrm_proxy_new(m);
        fail_if(proxy[i] == NULL, "Could not create proxy");

        char *label = (char*)malloc(10);
        sprintf(label, "p%d", i);
        lit = tmrm_literal_new(label, "http://www.w3.org/2001/XMLSchema#string");
        fail_if(lit == NULL, "Could not create literal");
        res = tmrm_proxy_add_property_literal(proxy[i], bottom, lit);
        tmrm_literal_free(lit);
        free(label);
    }

    /* Create the superclass/subclass hierarchy as specified above */
    res = tmrm_proxy_add_superclass(proxy[4], proxy[3]);
    res = tmrm_proxy_add_superclass(proxy[4], proxy[2]);
    res = tmrm_proxy_add_superclass(proxy[2], proxy[1]);
    res = tmrm_proxy_add_superclass(proxy[3], proxy[1]);
    res = tmrm_proxy_add_superclass(proxy[3], proxy[0]);
    res = tmrm_proxy_add_superclass(proxy[0], proxy[4]);

    /* Check the subclasses of p0: subclasses of p0 = (p0, p3, p4) */
    list = tmrm_proxy_subclasses(proxy[0]);
    fail_if(list == NULL, "Could not retrieve subclass list of p0");
    fail_if(tmrm_list_size(list) != 3,
        "Should have 3 subclasses, got %d", tmrm_list_size(list));
    tmrm_list_free(list);

    /* Check the subclasses of p1: subclasses of p1 = (p0, p1, p2, p3, p4) */
    list = tmrm_proxy_subclasses(proxy[1]);
    fail_if(list == NULL, "Could not retrieve subclass list of p1");
    fail_if(tmrm_list_size(list) != 5,
        "Should have 5 subclasses, got %d", tmrm_list_size(list));
    tmrm_list_free(list);

    fail_if(tmrm_proxy_sub(proxy[0], proxy[0]) != 1, "p0 not a subclass of p0");
    fail_if(tmrm_proxy_sub(proxy[3], proxy[0]) != 1, "p3 not a subclass of p0");
    fail_if(tmrm_proxy_sub(proxy[3], proxy[1]) != 1, "p3 is a subclass of p1");

    /* Check the superclasses of p1: p1 */
    list = tmrm_proxy_superclasses(proxy[1]);
    fail_if(list == NULL, "Could not retrieve superclass list of p1");
    fail_if(tmrm_list_size(list) != 1,
        "Should a list with one element, got %d", tmrm_list_size(list));
    tmrm_list_free(list);

    /* Check superclasses of p3: p0, p1, p2, p3, p4 */
    list = tmrm_proxy_superclasses(proxy[0]);
    fail_if(list == NULL, "Could not retrieve superclass list of p3");
    fail_if(tmrm_list_size(list) != 5,
        "Should a list with 5 elements, got %d", tmrm_list_size(list));
    tmrm_list_free(list);

    res = tmrm_proxy_isa(proxy[1], proxy[3]);
    fail_if(res != 0, "Wrong result from proxy_isa: %d", res);

    tmrm_proxy_add_type(proxy[1], proxy[3]);
    res = tmrm_proxy_isa(proxy[1], proxy[3]);
    fail_unless(res == 1, "p1 is not a subclass of p3: %d", res);

    res = tmrm_proxy_isa(proxy[1], proxy[0]);
    fail_unless(res == 1, "p1 is not a subclass of p0: %d", res);

    for (i = 0; i < 5; i++) {
        tmrm_proxy_free(proxy[i]);
    }
    tmrm_proxy_free(bottom);

    /* Get a multiset with all proxies in the subject map */
    set = tmrm_subject_map_proxies(m);
    fail_if(set == NULL, "Could not get list of proxies");
    
    list = tmrm_multiset_as_list(set);
    fail_if(list == NULL, "Could not convert list to multiset");

    elem = tmrm_list_head(list);
    while (elem != NULL) {
        obj = tmrm_list_data(elem);
        fail_if(obj == NULL, "Could not retrieve tmrm_object");
        fail_if(tmrm_object_get_type(obj) != TMRM_TYPE_PROXY, "Invalid object type");
        p = tmrm_object_to_proxy(obj);
        TMRM_DEBUG2("Found element '%s'\n", tmrm_proxy_label(p));
        elem = tmrm_list_next(elem);
    }
    tmrm_list_free(list);
    tmrm_multiset_free(set);
    /* Get an iterator over all proxies in the subject map */
    it = tmrm_subject_map_iterator(m);
    fail_if(it == NULL, "Could not get iterator over subject map");

    while (!tmrm_iterator_end(it)) {
        obj = tmrm_iterator_get_object(it);
        fail_if(obj == NULL, "Could not retrieve tmrm_object");
        fail_if(tmrm_object_get_type(obj) != TMRM_TYPE_PROXY, "Invalid object type");
        p = tmrm_object_to_proxy(obj);
        fail_if(p == NULL, "Could not get proxy from proxy list");
        res = tmrm_iterator_next(it);
        fail_unless(res == 0, "Could not get next element");
        /*TMRM_DEBUG2("Found proxy with label %s\n", tmrm_proxy_label(p));*/
    }
    tmrm_iterator_free(it);

    tmrm_subject_map_export_to_yaml(m, stdout);

    tmrm_subject_map_free(m);
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST


START_TEST (test_subject_map_yaml_import)
{
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;
    tmrm_literal *lit;
    tmrm_proxy *bottom;
    tmrm_multiset *set;
    int res;
    FILE *yaml;
    const char* filename = "yaml/t3.yaml";

    printf("=> test_subject_map_yaml_import\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);
    m = tmrm_subject_map_new(sms, storage, "mymap");

    yaml = fopen(filename, "r");
    fail_if(yaml == NULL, "Could not open file %s", filename);

    tmrm_subject_map_import_from_yaml(m, yaml);

    fclose(yaml);

    lit = tmrm_literal_new("superclass", "http://www.w3.org/2001/XMLSchema#string");
    fail_if(lit == NULL, "Could not create literal for superclass");

    bottom = tmrm_subject_map_bottom(m);

    TMRM_DEBUG2("Got bottom with label '%s'\n", tmrm_proxy_label(bottom));

    set = tmrm_literal_is_value_by_key(lit, bottom);
    fail_if(set == NULL, "Could not retrieve multiset");

    TMRM_DEBUG2(" =====> multiset with %d elements\n", tmrm_multiset_size(set));

    tmrm_proxy_free(bottom);
    tmrm_literal_free(lit);

    tmrm_subject_map_export_to_yaml(m, stdout);

    tmrm_subject_map_free(m);
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST


START_TEST (test_is_value_by_key)
{
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;
    int res, i;
    tmrm_proxy *proxy[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    tmrm_proxy *p;
    tmrm_proxy *bottom;
    tmrm_literal *lit;
    tmrm_list *list;
    tmrm_list_elmt *elem;
    tmrm_iterator *it;
    tmrm_multiset *set;

    printf("=> test_is_value_by_key\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);
    m = tmrm_subject_map_new(sms, storage, "mymap");

    bottom = tmrm_subject_map_bottom(m);
    fail_if(bottom == NULL, "Did not find proxy _bottom_");

    for (i = 0; i < 5; i++) {
        char *label = (char*)malloc(10);
        proxy[i] = tmrm_proxy_new(m);
        fail_if(proxy[i] == NULL, "Could not create proxy");

        sprintf(label, "p%d", i);
        lit = tmrm_literal_new(label, "http://www.w3.org/2001/XMLSchema#string");
        fail_if(lit == NULL, "Could not create literal");
        res = tmrm_proxy_add_property_literal(proxy[i], bottom, lit);
        tmrm_literal_free(lit);
        free(label);
    }

    tmrm_proxy_add_property(proxy[0], proxy[1], proxy[2]);

    /* tmrm_proxy_update is called automatically at the moment.
       this may become optional in the near future.
       tmrm_proxy_update(proxy[0]); */

    tmrm_proxy_add_property(proxy[3], proxy[3], proxy[2]);
    /* tmrm_proxy_update(proxy[3]); */

    set = tmrm_proxy_is_value_by_key(proxy[2], proxy[3]);
    fail_if(set == NULL, "Could not retrieve multiset");

    i = tmrm_multiset_size(set);
    fail_unless(i == 1, "is_value_by_key(p2, p3) returned an incorrect count of"
        " proxies: %d", i);
    tmrm_multiset_free(set);

    lit = tmrm_literal_new("p3", "http://www.w3.org/2001/XMLSchema#string");
    fail_if(lit == NULL, "Could not create literal");

    set = tmrm_literal_is_value_by_key(lit, proxy[3]);
    fail_if(set == NULL, "Could not create multiset");

    i = tmrm_multiset_size(set);
    fail_unless(i == 0, "literal_is_value_by_key(lit, p3) retured an incorrect "
        "of proxies: %d", i);
    tmrm_multiset_free(set);

    set = tmrm_literal_is_value_by_key(lit, bottom);
    fail_if(set == NULL, "Could not create multiset");

    i = tmrm_multiset_size(set);
    fail_unless(i == 1, "literal_is_value_by_key(lit, bottom) retured an incorrect "
        "of proxies: %d", i);
    tmrm_multiset_free(set);
    tmrm_literal_free(lit);


    for (i = 0; i < 5; i++) {
        tmrm_proxy_free(proxy[i]);
    }

    tmrm_proxy_free(bottom);

    tmrm_subject_map_free(m);
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST

START_TEST(test_new_list)
{
    tmrm_list* list;

    printf("=> test_new_list\n");

    list = tmrm_list_new(tmrm_object_free);
    fail_if(list == NULL, "Could not create list");

    tmrm_list_free(list);
}
END_TEST

START_TEST(test_new_tuple)
{
    tmrm_tuple* tuple;
    tmrm_object *values[1];

    printf("=> test_new_tuple\n");

    tuple = tmrm_tuple_new(0, NULL, NULL);
    fail_unless(tuple == NULL, "Empty tuple should not be possible");

    values[0] = NULL;
    tuple = tmrm_tuple_new(1, values, NULL);
    fail_if(tuple == NULL, "Could not create tuple");

    tmrm_tuple_free(tuple);

}
END_TEST

START_TEST(test_tuple_get_at)
{
    tmrm_tuple* tuple;
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;
    tmrm_object *obj, *val, *values[1];
    tmrm_proxy* p;

    printf("=> test_tuple_get_at\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);
    m = tmrm_subject_map_new(sms, storage, "mymap");
    fail_if(m == NULL, "Could not create subject map m");

    p = tmrm_subject_map_bottom(m);
    fail_if(p == NULL, "Could not get proxy by label: _bottom_");

    obj = tmrm_proxy_to_object(p);
    fail_if(obj == NULL, "Could not create new tmrm_object");

    values[0] = obj;

    tuple = tmrm_tuple_new(1, values, (tmrm_tuple_free_method*)tmrm_object_free);
    fail_if(tuple == NULL, "Could not create tuple");

    val = tmrm_tuple_get_at(tuple, -10);
    fail_unless(val == NULL, "Unexpected return value for index -10");
    val = tmrm_tuple_get_at(tuple, 1);
    fail_unless(val == NULL, "Unexpected return value for index 1");
    val = tmrm_tuple_get_at(tuple, 0);
    fail_if(val == NULL, "Unexpected return value for index 0");

    TMRM_DEBUG2("Found proxy label %s\n", tmrm_proxy_label(tmrm_object_to_proxy(val)));

    tmrm_tuple_free(tuple);
    tmrm_subject_map_free(m);
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST


START_TEST(test_new_hash)
{
    tmrm_subject_map_sphere* sms;
    tmrm_hash* h;
    const char* test_hash_string =
        "field1='value1'";
    int res_bool;
    int size;
    char* res_str;

    printf("=> test_new_hash\n");

    sms = tmrm_subject_map_sphere_new();
    fail_if(sms == NULL, "Could not create subject map sphere");

    /* Try to create a hash object with a non-existant backend */
    h = tmrm_hash_new_from_string(sms, (const char*)"XXX", (const char*)test_hash_string);
    fail_unless(h == NULL, "Should not be able to create hash with non-existant backend");

    /* Try to create a hash object with the memory backend */
    h = tmrm_hash_new_from_string(sms, "memory", test_hash_string);
    fail_if(h == NULL, "Could not create hash with memory backend");

    size = tmrm_hash_size(h);
    fail_if(size != 1, "Hash has invalid size (%d)", size);

    tmrm_hash_put_strings(h, "key", "value");

    size = tmrm_hash_size(h);
    fail_if(size != 2, "Hash has invalid size (%d)", size);

    /* Try to retrieve the value for key */
    res_str = tmrm_hash_get(h, "key");
    fail_if(res_str == NULL, "Could not retrieve value for 'key'");
    fail_unless(strncmp(res_str, "value", 5) == 0, "Invalid value for 'key': %s", res_str);
    free(res_str);

    tmrm_hash_free(h);

    /* Try to create a hash object with a (random) backend */
    h = tmrm_hash_new_from_string(sms, NULL, "");
    fail_if(h == NULL, "Could not create hash with random backend");
    size = tmrm_hash_size(h);
    fail_if(size != 0, "Hash has invalid size (%d)", size);

    /* tmrm_hash_put_strings(h, "key", "value");
    tmrm_hash_print(h, stdout); */

    tmrm_hash_free(h);

    tmrm_subject_map_sphere_free(sms);
}
END_TEST

START_TEST(test_hash_new_from_string)
{
    tmrm_subject_map_sphere* sms;
    tmrm_hash* h;
    const char * const test_hash_string =
        "field1='value1', field2='\\'value2', field3='\\\\', field4='\\\\\\'', field5 = 'a', field5='true' ";
    int res_bool;
    char* res_str;
    long res_long;

    printf("=> test_hash_new_from_string\n");

    sms = tmrm_subject_map_sphere_new();
    fail_if(sms == NULL, "Could not create subject map sphere");

    h = tmrm_hash_new_from_string(sms, NULL, test_hash_string);
    fail_if(h == NULL, "Could not create hash");

    tmrm_hash_print(h, stdout);

    /* Try to fetch an unexisting key */
    res_bool = tmrm_hash_get_as_boolean(h, "field6");
    fail_unless(res_bool < 0, "Could not retrieve field6 (bool): %d", res_bool);
    res_str = tmrm_hash_get(h, "FOO");
    fail_unless(res_str == NULL, "Could not retrieve field6 (str): %s", res_str);
    res_long = tmrm_hash_get_as_long(h, "BAR99");
    fail_unless(res_long < 0, "Could not retrieve field6 (long): %i", res_long);

    tmrm_hash_free(h);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST

START_TEST(test_hash_get)
{
    tmrm_subject_map_sphere* sms;
    tmrm_hash* h;
    const char * const test_hash_string =
        "field1='123', field2='true', field3='false', field4='yes', field5 = 'no', field6='xxx'";
    int res_bool;
    char* res_str;
    long res_long;

    printf("=> test_hash_get\n");

    sms = tmrm_subject_map_sphere_new();
    fail_if(sms == NULL, "Could not create subject map sphere");

    h = tmrm_hash_new_from_string(sms, NULL, test_hash_string);
    fail_if(h == NULL, "Could not create hash");

    res_long = tmrm_hash_get_as_long(h, "field1");
    fail_unless(res_long == 123, "Invalid long: %i", res_long);
    res_long = tmrm_hash_get_as_long(h, "field2");
    fail_unless(res_long < 0, "Invalid long: %i", res_long);
    res_bool = tmrm_hash_get_as_boolean(h, "field2");
    fail_unless(res_bool > 0, "Invalid bool: %d", res_bool);
    res_bool = tmrm_hash_get_as_boolean(h, "field4");
    fail_unless(res_bool > 0, "Invalid bool: %d", res_bool);
    res_bool = tmrm_hash_get_as_boolean(h, "field3");
    fail_unless(res_bool == 0, "Invalid bool: %d", res_bool);
    res_bool = tmrm_hash_get_as_boolean(h, "field5");
    fail_unless(res_bool == 0, "Invalid bool: %d", res_bool);
    res_bool = tmrm_hash_get_as_boolean(h, "field6");
    fail_unless(res_bool < 0, "Invalid bool: %d", res_bool);

    tmrm_hash_free(h);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST


START_TEST(test_hash_get_del) 
{
    tmrm_subject_map_sphere* sms;
    tmrm_hash *h, *h2;
    const char * const test_hash_string =
        "field1='foo', field2='bar', field2='baz'";
    char* value;
    tmrm_iterator* it;

    printf("=> test_hash_get_del\n");

    sms = tmrm_subject_map_sphere_new();
    fail_if(sms == NULL, "Could not create subject map sphere");

    h = tmrm_hash_new_from_string(sms, NULL, test_hash_string);
    fail_if(h == NULL, "Could not create hash");

    fail_if(tmrm_hash_size(h) != 3, "Size should be 3");
    value = tmrm_hash_get_del(h, "fieldXX");
    fail_if(tmrm_hash_size(h) != 3, "Size should still be 3");
    fail_unless(value == NULL, "value should be NULL");
    value = tmrm_hash_get_del(h, "field1");
    fail_if(strcmp(value, "foo"), "Expected value 'foo'");
    fail_if(tmrm_hash_size(h) != 2, "Size should still be 2 (now %d)",
        tmrm_hash_size(h));
    free(value);
    value = tmrm_hash_get_del(h, "field2");
    fail_unless(strcmp(value, "bar") || strcmp(value, "baz"),
        "value should be 'bar' or 'baz'");
    fail_if(tmrm_hash_size(h) != 0, "Size should still be 0 (now %d)",
        tmrm_hash_size(h));
    free(value);

    h2 = tmrm_hash_new_from_hash(h);
    fail_if(h2 == NULL, "Could not clone hash");
    fail_if(tmrm_hash_size(h) != tmrm_hash_size(h2), "Hash sizes differ");
    

    tmrm_hash_free(h);
    tmrm_hash_free(h2);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST


START_TEST(test_new_multiset)
{
    tmrm_multiset* ms;
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;

    printf("=> test_new_multiset\n");

    ms = tmrm_multiset_new(NULL);
    fail_unless(ms == NULL,
        "Should not be able to create multiset without subject map");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);
    m = tmrm_subject_map_new(sms, storage, "mymap");

    ms = tmrm_multiset_new(m);
    fail_if(ms == NULL,
        "Could not create multiset");
    tmrm_multiset_free(ms);

    tmrm_subject_map_free(m);
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST

START_TEST(test_add_multiset)
{
    tmrm_multiset* ms;
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;
    tmrm_proxy* p;
    tmrm_object* obj;
    int size, ret;

    printf("=> test_add_multiset\n");

    sms = tmrm_subject_map_sphere_new();
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS);
    m = tmrm_subject_map_new(sms, storage, "mymap");

    ms = tmrm_multiset_new(m);
    fail_if(ms == NULL,
        "Could not create multiset");

    size = tmrm_multiset_size(ms);
    fail_unless(size == 0, "Multiset size should be 0");

    /* Get a proxy and add it to the multiset */ 
    p = tmrm_subject_map_bottom(m);
    fail_if(p == NULL, "Could not get proxy by label: _bottom_");

    obj = tmrm_proxy_to_object(p);
    fail_if(obj == NULL, "Could not create object");

    ret = tmrm_multiset_insert(ms, obj);
    fail_unless(ret == 0, "Could not insert element into multiset: %d", ret);

    size = tmrm_multiset_size(ms);
    fail_unless(size == 1, "Multiset size should be 1 (and not %d)", size);

    /* Try to add the same proxy once more (it's a multiset after all) */
    ret = tmrm_multiset_insert(ms, obj);
    fail_unless(ret == 0, "Could not insert element into multiset: %d", ret);

    tmrm_object_free(obj);
    
    size = tmrm_multiset_size(ms);
    fail_unless(size == 2, "Multiset size should be 2");
    
    tmrm_subject_map_free(m);
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST

#if STORAGE_POSTGRESQL
START_TEST(test_pgsql_create_storage)
{
    tmrm_storage* storage;
    tmrm_subject_map_sphere* sms;
    tmrm_subject_map* m;

    sms = tmrm_subject_map_sphere_new();
    fail_if(sms == NULL, "Could not create subject map sphere");
    storage = tmrm_storage_new(sms, "pgsql", POSTGRESQL_OPTIONS_TEMPLATE);
    fail_if(storage == NULL, "Could not create storage");
    m = tmrm_subject_map_new(sms, storage, "mymap");
    fail_if(m == NULL, "Could not create subject map");

    tmrm_subject_map_free(m);
    tmrm_storage_free(storage);
    tmrm_subject_map_sphere_free(sms);
}
END_TEST
#endif

Suite*
libtmrm_suite (void)
{
    Suite *s = suite_create("libtmrm");

    TCase *tc_sm = tcase_create("Subject Map");
    /* tcase_add_checked_fixture(tc_sm, setup, teardown); */
    tcase_add_test(tc_sm, test_pgsql_storage);
    tcase_add_test(tc_sm, test_subject_map_yaml_import);
    tcase_add_checked_fixture(tc_sm, setup, teardown);
    suite_add_tcase(s, tc_sm);

    TCase *tc_list = tcase_create("List");
    tcase_add_test(tc_list, test_new_list);
    tcase_add_checked_fixture(tc_list, setup, teardown);
    suite_add_tcase(s, tc_list);

    TCase *tc_ms = tcase_create("Multi Set");
    tcase_add_test(tc_ms, test_new_multiset);
    tcase_add_test(tc_ms, test_add_multiset);
    tcase_add_checked_fixture(tc_ms, setup, teardown);
    suite_add_tcase(s, tc_ms);

    TCase *tc_tuple = tcase_create("Tuple");
    tcase_add_test(tc_tuple, test_new_tuple);
    tcase_add_test(tc_tuple, test_tuple_get_at);
    tcase_add_checked_fixture(tc_tuple, setup, teardown);
    suite_add_tcase(s, tc_tuple);

    TCase *tc_hash = tcase_create("Hash");
    tcase_add_test(tc_hash, test_new_hash);
    tcase_add_test(tc_hash, test_hash_new_from_string);
    tcase_add_test(tc_hash, test_hash_get);
    tcase_add_test(tc_hash, test_hash_get_del);
    tcase_add_checked_fixture(tc_hash, setup, teardown);
    suite_add_tcase(s, tc_hash);

    TCase *tc_proxy = tcase_create("Proxy");
    tcase_add_test(tc_proxy, test_create_proxy);
    tcase_add_test(tc_proxy, test_clone_proxy);
    tcase_add_test(tc_proxy, test_create_property_literal);
    tcase_add_test(tc_proxy, test_values_by_key);
    tcase_add_test(tc_proxy, test_literal_keys_by_value);
    tcase_add_test(tc_proxy, test_proxy_subclasses);
    tcase_add_test(tc_proxy, test_is_value_by_key);
    tcase_add_checked_fixture(tc_proxy, setup, teardown);
    suite_add_tcase(s, tc_proxy);

#if STORAGE_POSTGRESQL
    TCase *tc_pgsql = tcase_create("PostgresSQL");
    tcase_add_test(tc_pgsql, test_pgsql_create_storage);
    tcase_add_checked_fixture(tc_pgsql,
        pgsql_new_storage_setup, pgsql_new_storage_teardown);
    /* not needed: tcase_set_timeout(tc_pgsql, 0);*/
    suite_add_tcase(s, tc_pgsql);
#endif

    return s;
}

int
main (void)
{
    int number_failed;
    Suite *s = libtmrm_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all (sr, CK_VERBOSE);
    srunner_set_log (sr, "tmrmtest.log");
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
