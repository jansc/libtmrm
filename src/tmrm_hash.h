/*
 * tmrm_hash.h - (Temporary) implementation of hash functions.
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

#ifndef TMRM_HASH_H
#define TMRM_HASH_H



#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/* The hash implementation is taken from librdf. It may be replaced some day,
   but it's fine now... */

/* public constructors */
tmrm_hash* tmrm_hash_new(tmrm_subject_map_sphere *sms, const char *name);
tmrm_hash* tmrm_hash_new_from_string(tmrm_subject_map_sphere *sms,
        const char *name, const char *string);
tmrm_hash* tmrm_hash_new_from_array_of_strings(tmrm_subject_map_sphere *sms,
        const char *name, const char **array);

/* public copy constructor */
tmrm_hash* tmrm_hash_new_from_hash(tmrm_hash* old_hash);

/* public destructor */
void tmrm_hash_free(tmrm_hash *hash);

/* public methods */

/* retrieve the number of key/value pairs in the hash */
int tmrm_hash_size(tmrm_hash* hash);

/* retrieve one value for a given hash key */
char* tmrm_hash_get(tmrm_hash* hash, const char *key);

/* lookup a hash key and decode value as a boolean */
int tmrm_hash_get_as_boolean(tmrm_hash* hash, const char *key);

/* lookup a hash key and decode value as a long */
long tmrm_hash_get_as_long(tmrm_hash* hash, const char *key);

/* retrieve one value for key and delete from hash all other values */
char* tmrm_hash_get_del(tmrm_hash* hash, const char *key);

/* insert a key/value pair */
int tmrm_hash_put_strings(tmrm_hash* hash, const char *key, const char *value);

void tmrm_hash_print(tmrm_hash* hash, FILE *fh);
void tmrm_hash_print_keys(tmrm_hash* hash, FILE *fh);
void tmrm_hash_print_values(tmrm_hash* hash, const char *key_string, FILE *fh);

#ifdef __cplusplus
}
#endif

#endif
