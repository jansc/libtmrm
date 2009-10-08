/*
 * tmrm_literal.c - implementation of literals
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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <libtmrm.h>
#include <tmrm_internal.h>


/**
 * Creates and returns a new literal.
 * @returns NULL on failure.
 */
/*@null@*/ tmrm_literal*
tmrm_literal_new(const tmrm_char_t* value, const tmrm_char_t* datatype)
{
    tmrm_char_t *str;
    tmrm_literal* lit;

    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(value, tmrm_char_t, (tmrm_literal*)NULL);
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(datatype, tmrm_char_t, (tmrm_literal*)NULL);

    if (!(lit = (tmrm_literal*)TMRM_CALLOC(tmrm_literal, 1,
                    sizeof(tmrm_literal)))) {
        return (tmrm_literal*)NULL;
    }
    if (!(str = (tmrm_char_t*)TMRM_MALLOC(str, strlen((char*)value) + 1))) {
        TMRM_FREE(tmrm_literal, lit);
        return (tmrm_literal*)NULL;
    }
    (void)strcpy((char*)str, (char*)value);
    lit->value = str;

    if (!(str = (tmrm_char_t*)TMRM_MALLOC(str, strlen((char*)datatype) + 1))) {
        TMRM_FREE(cstring, str);
        TMRM_FREE(tmrm_literal, lit);
        return (tmrm_literal*)NULL;
    }
    (void)strcpy((char*)str, (char*)datatype);
    lit->datatype = str;
    lit->type = TMRM_TYPE_LITERAL;
    return lit;
}


/**
 * Returns the datatype of a literal.
 * @returns NULL on failure.
 */
const tmrm_char_t*
tmrm_literal_datatype(const tmrm_literal* lit) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(lit, tmrm_literal, (const tmrm_char_t*)NULL);
    return lit->datatype;
}


/**
 * Returns the value of a literal.
 */
const tmrm_char_t*
tmrm_literal_value(const tmrm_literal* lit) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(lit, tmrm_literal, (const tmrm_char_t*)NULL);
    return lit->value;
}


/**
 * Casts the proxy p into a tmrm_object.
 */
tmrm_object*
tmrm_literal_to_object(tmrm_literal *p) {
    TMRM_ASSERT_OBJECT_POINTER_RETURN_VALUE(p, tmrm_literal, NULL);
    return (tmrm_object*)p;
}


/**
 * Frees the memory occupied by a literal. 
 * May not be called with lit being a NULL pointer.
 */
void
tmrm_literal_free(tmrm_literal* l)
{
    TMRM_ASSERT_OBJECT_POINTER_RETURN(l, tmrm_literal);
    if (l->value != NULL) {
        TMRM_FREE(cstring, l->value);
    }
    if (l->datatype != NULL) {
        TMRM_FREE(cstring, l->datatype);
    }
    TMRM_FREE(tmrm_literal, l);
}

