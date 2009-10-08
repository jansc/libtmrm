/*
 * tmrm_list.c - Implementation of lists
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
#include <string.h>

#include <libtmrm.h>
#include <tmrm_list.h>
#include <tmrm_internal.h>

/**
 * Allocates and initialized a new list object. The pointer has to be free'd
 * with tmrm_list_free().
 *
 * @param free_handler May be a custom function that is called for each
 * remaining element in the list or NULL. In the latter case
 * the memory occupied by the remaining objects is not free'd.
 */
tmrm_list*
tmrm_list_new(/*@NULL@*/ tmrm_list_free_handler* free_handler) {
    tmrm_list* list;
    list = (tmrm_list*)TMRM_CALLOC(tmrm_list, 1, sizeof(tmrm_list));
    if (list == NULL) return NULL;
    list->size = 0;
    list->free_handler = free_handler;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

/**
 * Copy constructor. Clones a list.
 */
tmrm_list*
/*@null@*/tmrm_list_clone(tmrm_list *l_src) {
    /* FIXME */
    return NULL;
}


/* Removes each element and calls a user-defined function to free
 * dynamically allocated data.
 */
void
tmrm_list_free(/*@only@*/ tmrm_list *list) {
    tmrm_object *data;
    while (tmrm_list_size(list) > 0) {
        if (tmrm_list_rem_next(list, NULL, &data) == 0
                && list->free_handler != NULL) {
            list->free_handler(data);
        }
    }
    /* No operations are allowed now, but clear the structure as a precaution. */
    memset(list, 0, sizeof(tmrm_list));
    return;
}

int 
tmrm_list_ins_next(tmrm_list *list, tmrm_list_elmt *element, const tmrm_object *data) {
    tmrm_list_elmt *new_element;

    new_element = (tmrm_list_elmt *)TMRM_CALLOC(tmrm_list_elmt, 1, sizeof(tmrm_list_elmt));
    if (new_element == NULL)
        return -1;

    /* Insert the element into the list. */
    new_element->data = data;
    if (element == NULL) {
        /* Handle insertion at the head of the list. */
        if (tmrm_list_size(list) == 0)
            list->tail = new_element;

        new_element->next = list->head;
        list->head = new_element;
    } else {
        /* Handle insertion somewhere other than at the head. */
        if (element->next == NULL)
            list->tail = new_element;
        new_element->next = element->next;
        element->next = new_element;
    }
    list->size++;
    return 0;
}

int
tmrm_list_rem_next(tmrm_list *list, tmrm_list_elmt *element, tmrm_object **data) {
    tmrm_list_elmt *old_element;

    /* Do not allow removal from an empty list. */
    if (tmrm_list_size(list) == 0)
        return -1;

    /* Remove the element from the list. */
    if (element == NULL) {
        /* Handle removal from the head of the list. */
        *data = list->head->data;
        old_element = list->head;
        list->head = list->head->next;
        if (tmrm_list_size(list) == 1)
            list->tail = NULL;
    } else {
        /* Handle removal from somewhere other than the head. */
        if (element->next == NULL)
            return -1;

        *data = element->next->data;
        old_element = element->next;
        element->next = element->next->next;

        if (element->next == NULL)
            list->tail = element;
    }
    TMRM_FREE(tmrm_list_elmt, old_element);
    list->size--;

    return 0;
}


/*@null@*/ tmrm_list*
tmrm_list_from_iterator(tmrm_iterator* it, void (*free_handler)(tmrm_object *data)) {
    tmrm_list *list;
    tmrm_object *data;

    list = tmrm_list_new(free_handler);
    if (!list) return NULL;

    while (!tmrm_iterator_end(it)) {
        data = tmrm_iterator_get_object(it);
        tmrm_list_ins_next(list, NULL, data);
        if (!tmrm_iterator_next(it)) break;
    }
    return list;
}

