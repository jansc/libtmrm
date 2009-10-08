/*
 * tmrm_multiset.c - implementation of multi sets.
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

#include <libtmrm.h>
#include <tmrm_internal.h>

/**
 * Constructor: Constructs a new and empty multi set.
 * The constructor returns NULL on failure.
 */
/*@null@*/ tmrm_multiset*
tmrm_multiset_new(tmrm_subject_map *map) {
    tmrm_multiset *ms;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(map, tmrm_subject_map,
        (tmrm_multiset*)NULL);
    if (!(ms = (tmrm_multiset*)TMRM_CALLOC(tmrm_multiset, 1,
                    sizeof(tmrm_multiset)))) {
        return (tmrm_multiset*)NULL;
    }
    ms->type = TMRM_TYPE_MULTISET;
    ms->subject_map = map;
    if ((ms->list = tmrm_list_new(NULL)) == NULL) {
        TMRM_FREE(tmrm_multiset, ms);
        return NULL;
    }
    return ms;
}

/*@null@*/ tmrm_multiset*
tmrm_multiset_new_from_iterator(tmrm_subject_map *map, tmrm_iterator *it) {
    tmrm_multiset *ms;
    void *element;
    int status;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(map, tmrm_subject_map,
        (tmrm_multiset*)NULL);
    if ((ms = tmrm_multiset_new(map)) == NULL) return NULL;
    while (!tmrm_iterator_end(it)) {
        element = tmrm_iterator_get_object(it);
        if (element == NULL) continue;
        if ((status = tmrm_iterator_next(it)) != 0) {
            tmrm_multiset_free(ms);
            return NULL;
        }
        if ((status = tmrm_multiset_insert(ms, element)) != 0) {
            tmrm_multiset_free(ms);
            return NULL;
        }
    }

    TMRM_DEBUG2("Returning multiset with %d elements\n", tmrm_multiset_size(ms));
    
    return ms;
}


/* Copy constructor. */
tmrm_multiset* tmrm_multiset_clone(tmrm_multiset *ms_src) {
    tmrm_multiset *ms;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(ms_src, tmrm_multiset,
        (tmrm_multiset*)NULL);
    if ((ms = tmrm_multiset_new(ms_src->subject_map)) == NULL) return NULL;
    ms->list = tmrm_list_clone(ms_src->list);
    
    return ms;
}


/**
 * Inserts a new element into the set.
 */
int
tmrm_multiset_insert(tmrm_multiset *ms, const tmrm_object *data) {
    int ret;
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(ms, tmrm_multiset,
        -1);
    
    ret = tmrm_list_ins_next(ms->list, NULL, data);
    return ret;
}

/**
 * Returns the number of elements in the set or -1 if an error occurrs (called
 * with NULL pointer).
 */
int
tmrm_multiset_size(tmrm_multiset *ms) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(ms, tmrm_multiset, -1);
    if (ms->list == NULL) {
        return -1;
    }
    return tmrm_list_size(ms->list);
}

/**
 * Returns a list with all elements of the multi set. The calling function is
 * responsible for deleting the list with tmrm_list_free().
 * @todo Maybe we should provide a generic free function that frees all instances
 *      of proxies and literals. 
 * @return TMRM_STATUS_OK on success
 */
tmrm_list*
tmrm_multiset_as_list(const tmrm_multiset *ms) {
    tmrm_list *tmp_list;
    tmrm_list_elmt *elem;
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(ms, tmrm_multiset, NULL);
    
    /* FIXME who is responsible for the objects? Should we copy proxy objects? */
    tmp_list = tmrm_list_new(NULL);
    // Copy all elements of our internal list to 
    elem = tmrm_list_head(ms->list);
    while (elem != NULL) {
        tmrm_list_ins_next(tmp_list, NULL, tmrm_list_data(elem));
        elem = tmrm_list_next(elem);
    }
    return tmp_list;
}


/* Adds all elements of ms2 to the multi set ms. */
int tmrm_multiset_add(tmrm_multiset *ms, const tmrm_multiset *ms2) {
    tmrm_list *ms2_list;
    tmrm_list_elmt *elem;
    void *data;
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(ms, tmrm_multiset, -1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(ms2, tmrm_multiset, -1);

    ms2_list = tmrm_multiset_as_list(ms2);
    if (ms2_list == NULL) return -1;
    elem = tmrm_list_head(ms2_list);
    while (elem != NULL) {
        data = tmrm_list_data(elem);
        if (data != NULL) {
            tmrm_multiset_insert(ms, data);
        }
        elem = tmrm_list_next(elem);
    }
    tmrm_list_free(ms2_list);
    return 0;
}

/* Destructor: */
void
tmrm_multiset_free(/*@only@*/ tmrm_multiset *ms) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN(ms, tmrm_multiset);

    if (ms->list != NULL) {
        tmrm_list_free(ms->list);
    }
    ms->subject_map = NULL;
    TMRM_FREE(tmrm_multiset, ms);
}


/***************************************************************************/
