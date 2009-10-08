/*
 * tmrm_hash_cursor.c - (Temporary) implementation of hash functions.
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
#include <stdarg.h>
#include <ctype.h>

#include <libtmrm.h>
#include <tmrm_internal.h>
#include <tmrm_hash.h>
#include <tmrm_hash_internal.h>



/* private structure */
struct tmrm_hash_cursor_s {
    tmrm_hash *hash;
    void *context;
};


/**
 * Create a new tmrm_hash_cursor over a tmrm_hash.
 *
 * @param hash the hash object
 * @returns A new tmrm_hash_cursor or NULL on failure.
 */
tmrm_hash_cursor*
tmrm_hash_cursor_new(tmrm_hash* hash) 
{
    tmrm_hash_cursor* cursor;
    void *cursor_context;

    cursor = (tmrm_hash_cursor*)TMRM_CALLOC(cursor, 1, 
            sizeof(tmrm_hash_cursor));
    if(!cursor)
        return NULL;

    cursor_context = (char*)TMRM_CALLOC(cursor_context, 1,
            hash->factory->cursor_context_length);
    if(!cursor_context) {
        TMRM_FREE(tmrm_hash_cursor, cursor);
        return NULL;
    }

    cursor->hash = hash;
    cursor->context = cursor_context;

    if(hash->factory->cursor_init(cursor->context, hash->context)) {
        tmrm_hash_cursor_free(cursor);
    cursor = NULL;
  }

  return cursor;
}


/**
 * Destroy a tmrm_hash_cursor object.
 *
 * @param cursor hash cursor object
 */
void
tmrm_hash_cursor_free(tmrm_hash_cursor* cursor) 
{
    if(cursor->context) {
        cursor->hash->factory->cursor_finish(cursor->context);
        TMRM_FREE(void, cursor->context);
    }

    TMRM_FREE(tmrm_hash_cursor, cursor);
}


int
tmrm_hash_cursor_set(tmrm_hash_cursor *cursor,
                       tmrm_hash_datum *key,
                       tmrm_hash_datum *value)
{
    return cursor->hash->factory->cursor_get(cursor->context, key, value, 
            TMRM_HASH_CURSOR_SET);
}


int
tmrm_hash_cursor_get_next_value(tmrm_hash_cursor *cursor, 
                                  tmrm_hash_datum *key,
                                  tmrm_hash_datum *value)
{
    return cursor->hash->factory->cursor_get(cursor->context, key, value, 
            TMRM_HASH_CURSOR_NEXT_VALUE);
}


int
tmrm_hash_cursor_get_first(tmrm_hash_cursor *cursor,
                             tmrm_hash_datum *key, tmrm_hash_datum *value)
{
    return cursor->hash->factory->cursor_get(cursor->context, key, value, 
            TMRM_HASH_CURSOR_FIRST);
}


int
tmrm_hash_cursor_get_next(tmrm_hash_cursor *cursor, tmrm_hash_datum *key,
                            tmrm_hash_datum *value)
{
    return cursor->hash->factory->cursor_get(cursor->context, key, value, 
            TMRM_HASH_CURSOR_NEXT);
}
