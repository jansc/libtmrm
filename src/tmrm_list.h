/*
 * tmrm_list.h - Header file for lists
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

#ifndef TMRM_LIST_H
#define TMRM_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <libtmrm.h>

/*
 * Define a structure for linked list elements.
 */
typedef struct list_elmt_s {
    tmrm_object *data;
    struct list_elmt_s *next;
} tmrm_list_elmt;

typedef void (tmrm_list_free_handler(tmrm_object* object));

/*
 * Define a structure for linked lists.
 */
typedef struct tmrm_list_s {
    int size;
    /*int (*match)(const void *key1, const void *key2);*/
    tmrm_list_free_handler *free_handler;
    tmrm_list_elmt *head;
    tmrm_list_elmt *tail;
} tmrm_list;


/* Public Interface */
/*@null@*/ tmrm_list* tmrm_list_new(/*@null@*/ tmrm_list_free_handler *free_handler);

tmrm_list* /*@null@*/tmrm_list_clone(tmrm_list *l_src);

void tmrm_list_free(/*@only@*/ tmrm_list *list);

/* Do we need this?
void tmrm_list_copy(tmrm_list *list, int deep_copy); */

int tmrm_list_ins_next(tmrm_list *list, tmrm_list_elmt *element, const tmrm_object *data);

int tmrm_list_rem_next(tmrm_list *list, tmrm_list_elmt *element, tmrm_object **data);

#define tmrm_list_size(list) ((list)->size)

#define tmrm_list_head(list) ((list)->head)

#define tmrm_list_tail(list) ((list)->tail)

#define tmrm_list_is_head(list, element) ((element) == (list)->head ? 1 : 0)

#define tmrm_list_is_tail(element) ((element)->next == NULL ? 1 : 0)

#define tmrm_list_data(element) ((element)->data)

#define tmrm_list_next(element) ((element)->next)

/**
 * Converts an iterator to a list by iterating over all elements and adding
 * them to a new list.
 *
 * @returns NULL on failure or a list consisting of the elements of it
 * otherwise.
 */
/*@null@*/ tmrm_list* tmrm_list_from_iterator(tmrm_iterator* it,
    void (*free_handler)(tmrm_object *data));


#ifdef __cplusplus
}
#endif

#endif
