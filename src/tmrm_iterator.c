/*
 * tmrm_iterator.c - implementation of iterators
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
 * Creates a new iterator object or NULL if an error occurs. The caller is
 * responsible for freeing the object with tmrm_iterator_free().
 *
 * All of the function below are called with the context as a parameter.
 * @param sms The subject map sphere that the iterator belongs to.
 * @param context A place to store the internal state of the iterator. The
 *        Following methods can use the context:
 * @param next_method A function pointer to a function that moves the iterator
 *        to the next element.
 * @param end_method A function pointer to a function that tests if the end of
 *        the iterable set has been reached.
 * @param get_element_method A function pointer to a function that returns the
 *        element at the current position (or NULL if there is none).
 * @param free_method A function pointer to a function that frees the memory
 *        associated with the context object. Note that this function must not
 *        free the tmrm_iterator object itself.
 * @see tmrm_iterator_free
 *
 * @returns NULL on failure.
 */
/*@null@*/ tmrm_iterator*
tmrm_iterator_new(tmrm_subject_map_sphere* sms, void* context, int (*next_method)(void*),
    int (*end_method)(void*), tmrm_object* (*get_element_method)(void*, tmrm_iterator_flag), void (*free_method)(void*))
{
    tmrm_iterator *it;

    if (!(it = (tmrm_iterator*)TMRM_CALLOC(tmrm_iterator, 1, sizeof(tmrm_iterator)))) {
        return (tmrm_iterator*)NULL;
    }
    it->type = TMRM_TYPE_ITERATOR;
    it->sms = sms;
    it->context = context;
    it->next_method = next_method;
    it->end_method = end_method;
    it->get_element_method = get_element_method;
    it->free_method = free_method;

    return it;
}

int
tmrm_iterator_next(tmrm_iterator* iterator)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(iterator, tmrm_iterator, -1);
    if (iterator->next_method)
        return iterator->next_method(iterator->context);

    return 0;
}

int
tmrm_iterator_end(tmrm_iterator* iterator)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(iterator, tmrm_iterator, -1);
    if (iterator->end_method)
        return iterator->end_method(iterator->context);

    return 0;
}


/*@null@*/ tmrm_object*
tmrm_iterator_get_object(tmrm_iterator* iterator)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(iterator, tmrm_iterator, (tmrm_object*)NULL);
    if (iterator->get_element_method)
        return iterator->get_element_method(iterator->context,
            TMRM_ITERATOR_GET_METHOD_GET_OBJECT);

    return NULL;
}


/*@null@*/ tmrm_object*
tmrm_iterator_get_key(tmrm_iterator* iterator)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(iterator, tmrm_iterator,
            (tmrm_object*)NULL);
    if (iterator->get_element_method)
        return iterator->get_element_method(iterator->context,
            TMRM_ITERATOR_GET_METHOD_GET_KEY);

    return NULL;
}


/*@null@*/ tmrm_object*
tmrm_iterator_get_value(tmrm_iterator* iterator)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(iterator, tmrm_iterator,
        (tmrm_object*)NULL);
    if (iterator->get_element_method)
        return iterator->get_element_method(iterator->context,
            TMRM_ITERATOR_GET_METHOD_GET_VALUE);

    return NULL;
}


void
tmrm_iterator_free(tmrm_iterator* iterator)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN(iterator, tmrm_iterator);

    if (iterator->free_method)
        iterator->free_method(iterator->context);
    TMRM_FREE(tmrm_iterator, iterator);
}
/***************************************************************************/

