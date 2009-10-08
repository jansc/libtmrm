/*
 * tmrm_tuple.h - tmrm_tuple pubic API
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

#ifndef TMRM_TUPLE_H
#define TMRM_TUPLE_H


/**
 * @file tmrm_tuple.h
 * @brief tmrm_tuple public API.
 */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * tmrm_tuple represents a tuple in the path language.
 *
 * @defgroup tmrm_tuple tmrm_tuple
 * @ingroup libtmrm_public
 * @{
 */

typedef void (tmrm_tuple_free_method(tmrm_object* object));


/* Creates a new tuple with 'size' elements. */
/*@null@*/ tmrm_tuple* tmrm_tuple_new(size_t size, tmrm_object* values[],
    tmrm_tuple_free_method* method);


/* Copy constructor. */
tmrm_tuple* tmrm_tuple_clone(tmrm_tuple *t_src);


/* Returns the size of a tuple. */
int tmrm_tuple_size(tmrm_tuple* t);


/* Appends all elements of t2 to t1. */
int tmrm_tuple_concat(tmrm_tuple* t1, tmrm_tuple* t2);


/**
 * Zips the value and keys tuple into a new proxy. 'values' and 'keys'
 * must have equal size. The type of all values in keys must be tmrm_proxy.
 * 'values' can contain both tmrm_proxy and tmrm_literal objects.
 * The proxy must be free'd with tmrm_proxy_free().
 * Returns 0 on success.
 */
int tmrm_tuple_zip(tmrm_tuple* values, tmrm_tuple* keys, tmrm_proxy** p);


/**
 * Returns 1 if the tuples t and other are equal, 0 otherwise.
 * Two tuples are compared by comparing all their values (using
 * the compare function).
 */
int tmrm_tuple_equals(tmrm_tuple* t, tmrm_tuple* other,
    int(*compare)(const tmrm_object *, const tmrm_object *));


/**
 * Compares two tuples with the use of an ordering tuple. The
 * ordering tuple is a magic tuple that only contains ordering
 * values. => enum? 
 *  * TMRM_ORDER_DESC, TMRM_ORDER_ASC, TMRM_ORDER_IGNORE
 */
int tmrm_tuple_compare(tmrm_tuple* t, tmrm_tuple* other, tmrm_tuple* order,
    int(*compare)(const tmrm_object *, const tmrm_object*));


/* Returns the value at position 'idx'. */
tmrm_object* tmrm_tuple_get_at(tmrm_tuple* t, size_t idx);


/* Do we need tmrm_tuple_set_at(tmrm_tuple* t, int idx, void* val); ? */


/* Slices a tuple t t(j..k) = PROD [from i=j to k] <vi> and t(i) = t(i..i) */
int tmrm_tuple_slice(tmrm_tuple* t, size_t i, size_t j);


/**
 * Free's a tuple and its values.
 */
void tmrm_tuple_free(/*@only@*/ tmrm_tuple* t);

/** @} */

#ifdef __cplusplus
}
#endif

#endif

