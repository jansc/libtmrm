/*
 * tmrm_tuple.c - implementation of tuples
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
#include <tmrm_tuple.h>

#include <string.h>


/**
 * Creates a new tuple object with a given size. The caller is
 * responsible for freeing the object with tmrm_tuple_free().
 * The tuple may be initialized with values which is an array of tmrm_object
 * pointers. The function copies the _pointers_ but not the underlying
 * objects.
 *
 * @param size The size of the tuple.
 * @param values array of tmrm_object pointers
 * @param free_method Pointer to a function that free's _one_ value
 * @see tmrm_tuple_free
 *
 * @returns NULL on failure (out of memory).
 */
/*@null@*/ tmrm_tuple*
tmrm_tuple_new(size_t size, tmrm_object* values[], tmrm_tuple_free_method* free_method)
{
    tmrm_tuple *t;

    if (size <= 0) return (tmrm_tuple*)NULL;

    if (!(t = (tmrm_tuple*)TMRM_CALLOC(tmrm_tuple, 1, sizeof(tmrm_tuple)))) {
        return (tmrm_tuple*)NULL;
    }
    t->type = TMRM_TYPE_TUPLE;
    t->size = size;
    t->values = (tmrm_object**)TMRM_CALLOC(tmrm_object, size, sizeof(tmrm_object*));

    if (!t->values) {
        TMRM_FREE(tmrm_tuple, t);
        return (tmrm_tuple*)NULL;
    }
    t->free_method = free_method;

    if (size > 0) {
        memcpy(t->values, values, sizeof(tmrm_object*)*size);
    }

    return t;
}


/* Copy constructor. */
tmrm_tuple* tmrm_tuple_clone(tmrm_tuple *t_src)
{
    /* FIXME */
    return NULL;
}


/**
 * Returns the count of elements in the tuple, or a value below zero if an
 * error occurs.
 */
int
tmrm_tuple_size(tmrm_tuple* t) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(t, tmrm_tuple, -1);
    
    return (int)t->size;
}


/**
 * Appends all elements of t2 to t1. Returns 0 on success.
 * t and t2 may be the same tuple.
 *
 * @return 0 on success
 */
int tmrm_tuple_concat(tmrm_tuple* t, tmrm_tuple* t2) {
    tmrm_object** new_values;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(t, tmrm_tuple, -1);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(t2, tmrm_tuple, -1);

    new_values = (tmrm_object**)TMRM_CALLOC(tmrm_object, t->size + t2->size, sizeof(tmrm_object*));
    if (!new_values)
        return -1;

    memcpy(new_values[0], t->values, sizeof(tmrm_object*) * t->size);
    memcpy(new_values[t->size], t2->values, sizeof(tmrm_object*) * t2->size);

    /* FIXME t->values can be NULL */
    TMRM_FREE(tmrm_object*, t->values);

    t->values = new_values;

    return 0;
}


/**
 * Slices a tuple t t(j..k) = PROD [from i=j to k] &lt;vi&gt; and t(i) = t(i..i)
 *
 * @return 0 on success
 */
int
tmrm_tuple_slice(tmrm_tuple* t, size_t j, size_t k) {
    tmrm_object** new_values;
    size_t i;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(t, tmrm_tuple, -1);

    /* FIXME t->size can be 0 */

    if (j > k || j < 0 || k > t->size - 1) {
        return -1;
    }

    new_values = (tmrm_object**)TMRM_CALLOC(tmrm_object, k - j + 1, sizeof(tmrm_object*));
    if (!new_values)
        return -1;

    for(i = j; i <= k; i++) {
        new_values[i-j] = t->values[i];
    }

    TMRM_FREE(tmrm_object**, t->values);
    t->values = new_values;
    t->size = k - j + 1;
    return 0;
}



/**
 * Returns the value at position 'idx' or NULL if an error occurs.
 */
tmrm_object*
tmrm_tuple_get_at(tmrm_tuple* t, size_t idx) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(t, tmrm_object*, (tmrm_object*)NULL);
    
    if (idx < 0 || idx > t->size - 1)
        return NULL;

    return t->values[idx];
}


/**
 * Free's all values of the tuple with the free function and 
 * the tuple itself.
 */
void
tmrm_tuple_free(/*@only@*/ tmrm_tuple *t)
{
    size_t i;
    TMRM_ASSERT_OBJECT_POINTER_RETURN(t, tmrm_tuple);

    if (t->free_method)
        for (i = 0; i < t->size; i++)
            t->free_method(t->values[i]);

    if (t->values)
        TMRM_FREE(void, t->values);

    TMRM_FREE(tmrm_tuple, t);
}

/***************************************************************************/
