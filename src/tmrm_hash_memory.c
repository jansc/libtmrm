/*
 * tmrm_hash_memory.c - (Temporary) implementation of hash functions.
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
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for abort() as used in errors */
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <libtmrm.h>
#include <tmrm_internal.h>
#include <tmrm_hash_internal.h>


/* private structures */
struct tmrm_hash_memory_node_value_s
{
    struct tmrm_hash_memory_node_value_s* next;
    void *value;
    size_t value_len;
};
typedef struct tmrm_hash_memory_node_value_s tmrm_hash_memory_node_value;


struct tmrm_hash_memory_node_s
{
    struct tmrm_hash_memory_node_s* next;
    void *key;
    size_t key_len;
    uint32_t hash_key;
    tmrm_hash_memory_node_value *values;
    int values_count;
};
typedef struct tmrm_hash_memory_node_s tmrm_hash_memory_node;


typedef struct
{
    /* the hash object */
    tmrm_hash* hash;
    /* An array pointing to a list of nodes (buckets) */
    tmrm_hash_memory_node** nodes;
    /* this many buckets used */
    int size;
    /* this many keys */
    int keys;
    /* this many values */
    int values;
    /* total array size */
    int capacity;

    /* array load factor expressed out of 1000.
     * Always true: (size/capacity * 1000) < load_factor,
     * or in the code: size * 1000 < load_factor * capacity
     */
    int load_factor;
} tmrm_hash_memory_context;



/* default load_factor out of 1000 */
static const int tmrm_hash_default_load_factor = 750;

/* starting capacity - MUST BE POWER OF 2 */
static const int tmrm_hash_initial_capacity = 8;


/* prototypes for local functions */
static tmrm_hash_memory_node* tmrm_hash_memory_find_node(tmrm_hash_memory_context* hash, void *key, size_t key_len, int *bucket, tmrm_hash_memory_node** prev);
static void tmrm_hash_memory_node_free(tmrm_hash_memory_node* node);
static int tmrm_hash_memory_expand_size(tmrm_hash_memory_context* hash);

/* Implementing the hash cursor */
static int tmrm_hash_memory_cursor_init(void *cursor_context, void *hash_context);
static int tmrm_hash_memory_cursor_get(void* context, tmrm_hash_datum* key, tmrm_hash_datum* value, unsigned int flags);
static void tmrm_hash_memory_cursor_finish(void* context);


/* functions implementing the API */
static int tmrm_hash_memory_create(tmrm_hash* new_hash, void* context);
static int tmrm_hash_memory_destroy(void* context);
static int tmrm_hash_memory_open(void* context, const char *identifier, int mode, int is_writable, int is_new, tmrm_hash* options);
static int tmrm_hash_memory_close(void* context);
static int tmrm_hash_memory_clone(tmrm_hash* new_hash, void *new_context, char *new_identifier, void* old_context);
static int tmrm_hash_memory_values_count(void *context);
static int tmrm_hash_memory_put(void* context, tmrm_hash_datum *key, tmrm_hash_datum *data);
static int tmrm_hash_memory_exists(void* context, tmrm_hash_datum *key, tmrm_hash_datum *value);
static int tmrm_hash_memory_delete_key(void* context, tmrm_hash_datum *key);
static int tmrm_hash_memory_delete_key_value(void* context, tmrm_hash_datum *key, tmrm_hash_datum *value);
static int tmrm_hash_memory_sync(void* context);
static int tmrm_hash_memory_get_fd(void* context);

static void tmrm_hash_memory_register_factory(tmrm_hash_factory *factory);




/*
 * perldelta 5.8.0 says under *Performance Enhancements*
 *
 *   Hashes now use Bob Jenkins "One-at-a-Time" hashing key algorithm
 *   http://burtleburtle.net/bob/hash/doobs.html  This algorithm is
 *   reasonably fast while producing a much better spread of values
 *   than the old hashing algorithm ...
 *
 * Changed here to hash the string backwards to help do URIs better
 *
 */

#define ONE_AT_A_TIME_HASH(hash,str,len) \
     do { \
        register const unsigned char *c_oneat = (unsigned char*)str+len-1; \
        register int i_oneat = len; \
        register uint32_t hash_oneat = 0; \
        while (i_oneat--) { \
            hash_oneat += *c_oneat--; \
            hash_oneat += (hash_oneat << 10); \
            hash_oneat ^= (hash_oneat >> 6); \
        } \
        hash_oneat += (hash_oneat << 3); \
        hash_oneat ^= (hash_oneat >> 11); \
        (hash) = (hash_oneat + (hash_oneat << 15)); \
    } while(0)



/* helper functions */


/**
 * Find the node for the given key or value.
 * 
 * If value is not NULL and value_len is non 0, the value will also be
 * compared in the search.
 *
 * If user_bucket is not NULL, the bucket used will be returned.  if
 * prev is no NULL, the previous node in the list will be returned.
 * 
 * @param hash The memory hash context
 * @param key Key string
 * @parma key_len Key string length
 * @param user_bucket Pointer to store bucket
 * @param prev Pointer to store previous node
 * @returns tmrm_hash_memory_node of content or NULL on failure
 */
static tmrm_hash_memory_node*
tmrm_hash_memory_find_node(tmrm_hash_memory_context* hash, 
			     void *key, size_t key_len,
			     int *user_bucket,
			     tmrm_hash_memory_node** prev) 
{
    tmrm_hash_memory_node* node;
    int bucket;
    uint32_t hash_key;

    /* empty hash */
    if (!hash->capacity)
        return NULL;

    ONE_AT_A_TIME_HASH(hash_key, key, key_len);

    if(prev)
        *prev = NULL;

    /* find slot in table */
    bucket = hash_key & (hash->capacity - 1);
    if(user_bucket)
        *user_bucket=bucket;

    /* check if there is a list present */ 
    node = hash->nodes[bucket];
    if (!node)
        /* no list there */
        return NULL;

    /* walk the list */
    while(node) {
        if(key_len == node->key_len && !memcmp(key, node->key, key_len))
            break;
        if(prev)
            *prev = node;
        node = node->next;
    }

    return node;
}


static void
tmrm_hash_memory_node_free(tmrm_hash_memory_node* node) 
{
    if(node->key)
        TMRM_FREE(cstring, node->key);
    if(node->values) {
        tmrm_hash_memory_node_value *vnode, *next;

        /* Empty the list of values */
        for (vnode = node->values; vnode; vnode = next) {
            next = vnode->next;
            if(vnode->value)
                TMRM_FREE(cstring, vnode->value);
            TMRM_FREE(tmrm_hash_memory_node_value, vnode);
        }
    }
    TMRM_FREE(tmrm_hash_memory_node, node);
}


static int
tmrm_hash_memory_expand_size(tmrm_hash_memory_context* hash) {
    int required_capacity = 0;
    tmrm_hash_memory_node **new_nodes;
    int i;

    if (hash->capacity) {
        /* big enough */
        if((1000 * hash->keys) < (hash->load_factor * hash->capacity))
            return 0;
        /* grow hash (keeping it a power of two) */
        required_capacity = hash->capacity << 1;
    } else {
        required_capacity = tmrm_hash_initial_capacity;
    }

    /* allocate new table */
    new_nodes = (tmrm_hash_memory_node**)TMRM_CALLOC(tmrm_hash_memory_nodes, 
            required_capacity,
            sizeof(tmrm_hash_memory_node*));
    if(!new_nodes)
        return 1;


    /* it is a new hash empty hash - we are done */
    if(!hash->size) {
        hash->capacity = required_capacity;
        hash->nodes = new_nodes;
        return 0;
    }


    for(i=0; i<hash->capacity; i++) {
        tmrm_hash_memory_node *node = hash->nodes[i];

        /* walk all attached nodes */
        while(node) {
            tmrm_hash_memory_node *next;
            int bucket;

            next = node->next;
            /* find slot in new table */
            bucket = node->hash_key & (required_capacity - 1);
            node->next = new_nodes[bucket];
            new_nodes[bucket] = node;

            node = next;
        }
    }

    /* now free old table */
    TMRM_FREE(tmrm_hash_memory_nodes, hash->nodes);

    /* attach new one */
    hash->capacity = required_capacity;
    hash->nodes = new_nodes;

    return 0;
}



/* Functions implementing hash API */

/**
 * Create a new memory hash.
 *
 * @param hash tmrm_hash hash
 * @param context Memory hash contxt
 * @returns Non 0 on failure
 */
static int
tmrm_hash_memory_create(tmrm_hash* hash, void* context) 
{
    tmrm_hash_memory_context* hcontext = (tmrm_hash_memory_context*)context;

    hcontext->hash = hash;
    hcontext->load_factor = tmrm_hash_default_load_factor;
    return tmrm_hash_memory_expand_size(hcontext);
}


/**
 * Destroy a memory hash.
 *
 * @param context Memory hash context
 * @returns Non 0 on failure
 */
static int
tmrm_hash_memory_destroy(void* context) 
{
    tmrm_hash_memory_context* hcontext = (tmrm_hash_memory_context*)context;

    if(hcontext->nodes) {
        int i;

        for(i=0; i<hcontext->capacity; i++) {
            tmrm_hash_memory_node *node = hcontext->nodes[i];

            /* this entry is used */
            if(node) {
                tmrm_hash_memory_node *next;
                /* free all attached nodes */
                while(node) {
                    next = node->next;
                    tmrm_hash_memory_node_free(node);
                    node = next;
                }
            }
        }
        TMRM_FREE(tmrm_hash_memory_nodes, hcontext->nodes);
    }

    return 0;
}


/**
 * Open memory hash with given parameters.
 * 
 * @param context Memory hash context
 * @param identifier Identifier - not used
 * @param mode Access mode - not used
 * @param is_writable Is hash writable? - not used
 * @param is_new Is hash new? - not used
 * @param options tmrm_hash of options - not used
 * @returns Non 0 on failure
 */
static int
tmrm_hash_memory_open(void* context, const char *identifier,
                        int mode, int is_writable, int is_new,
                        tmrm_hash* options) 
{
    /* NOP */
    return 0;
}


/**
 * Close the hash.
 *
 * @param context Memory hash context
 * @returns Non 0 on failure
 */
static int
tmrm_hash_memory_close(void* context) 
{
    /* NOP */
    return 0;
}


static int
tmrm_hash_memory_clone(tmrm_hash *hash, void* context, char *new_identifer,
                         void *old_context) 
{
    tmrm_hash_memory_context* hcontext = (tmrm_hash_memory_context*)context;
    tmrm_hash_memory_context* old_hcontext = (tmrm_hash_memory_context*)old_context;
    tmrm_hash_datum *key, *value;
    tmrm_iterator *iterator;
    int status = 0;

    /* copy data fields that might change */
    hcontext->hash = hash;
    hcontext->load_factor = old_hcontext->load_factor;

    /* Don't need to deal with new_identifier - not used for memory hashes */

    /* Use higher level functions to iterator this data
     * on the other hand, maybe this is a good idea since that
     * code is tested and works
     */

    key = tmrm_hash_datum_new(hash->sms, NULL, 0);
    value = tmrm_hash_datum_new(hash->sms, NULL, 0);

    iterator = tmrm_hash_get_all(old_hcontext->hash, key, value);
    while(!tmrm_iterator_end(iterator)) {
        tmrm_hash_datum* k = (tmrm_hash_datum*)tmrm_iterator_get_key(iterator);
        tmrm_hash_datum* v = (tmrm_hash_datum*)tmrm_iterator_get_value(iterator);

        if(tmrm_hash_memory_put(hcontext, k, v)) {
            status = 1;
            break;
        }
        tmrm_iterator_next(iterator);
    }
    if(iterator)
        tmrm_iterator_free(iterator);

    tmrm_hash_datum_free(value);
    tmrm_hash_datum_free(key);

    return status;
}


/**
 * Get the number of values in the hash.
 * 
 * @param context Memory hash cursor context
 * @returns Number of values in the hash or <0 on failure
 */
static int
tmrm_hash_memory_values_count(void *context) 
{
    tmrm_hash_memory_context* hash = (tmrm_hash_memory_context*)context;

    return hash->values;
}



typedef struct {
    tmrm_hash_memory_context* hash;
    int current_bucket;
    tmrm_hash_memory_node* current_node;
    tmrm_hash_memory_node_value *current_value;
} tmrm_hash_memory_cursor_context;



/**
 * Initialise a new hash cursor.
 * 
 * @param cursor_context Hash cursor context
 * @param hash_context Hash to operate over
 * @returns Non 0 on failure
 */
int
tmrm_hash_memory_cursor_init(void *cursor_context, void *hash_context) 
{
    tmrm_hash_memory_cursor_context *cursor = (tmrm_hash_memory_cursor_context*)cursor_context;

    cursor->hash = (tmrm_hash_memory_context*)hash_context;
    return 0;
}


/**
 * Retrieve a hash value for the given key.
 * 
 * @param context Memory hash cursor context
 * @param key Pointer to key to use
 * @param value Pointer to value to use
 * @param flags Flags
 * @returns Non 0 on failure
 */
static int
tmrm_hash_memory_cursor_get(void* context, 
                              tmrm_hash_datum *key,
                              tmrm_hash_datum *value,
                              unsigned int flags)
{
    tmrm_hash_memory_cursor_context *cursor = (tmrm_hash_memory_cursor_context*)context;
    tmrm_hash_memory_node_value *vnode = NULL;
    tmrm_hash_memory_node *node;


    /* First step, make sure cursor->current_node points to a valid node,
       if possible */

    /* Move to start of hash if necessary  */
    if (flags == TMRM_HASH_CURSOR_FIRST) {
        int i;

        cursor->current_node = NULL;
        /* find first used bucket (with keys) */
        cursor->current_bucket = 0;

        for(i = 0; i< cursor->hash->capacity; i++)
            if ((cursor->current_node = cursor->hash->nodes[i])) {
                cursor->current_bucket = i;
                break;
            }

        if (cursor->current_node)
            cursor->current_value=cursor->current_node->values;
    }

    /* If still have no current node, try to find it from the key */
    if (!cursor->current_node && key && key->data) {
        cursor->current_node = tmrm_hash_memory_find_node(cursor->hash,
                (char*)key->data,
                key->size,
                NULL, NULL);
        if (cursor->current_node)
            cursor->current_value = cursor->current_node->values;
    }


    /* If still have no node, failed */
    if (!cursor->current_node)
        return 1;

    /* Check for end of values */

    switch(flags) {
        case TMRM_HASH_CURSOR_SET:
            /* If key does not exist, failed above, so test if there are values */

            /* FALLTHROUGH */
        case TMRM_HASH_CURSOR_NEXT_VALUE:
            /* If want values and have reached end of values list, end */
            if (!cursor->current_value)
                return 1;
            break;

        case TMRM_HASH_CURSOR_FIRST:
        case TMRM_HASH_CURSOR_NEXT:
            /* If have reached last bucket, end */
            if (cursor->current_bucket >= cursor->hash->capacity)
                return 1;

            break;
        default:
            /*
            tmrm_log(cursor->hash->hash->world,
                    0, TMRM_LOG_ERROR, TMRM_FROM_HASH, NULL,
                    "Unknown hash method flag %d", flags);
            */
            return 1;
    }


    /* Ok, there is data, retrieve it */

    switch(flags) {
        case TMRM_HASH_CURSOR_SET:

            /* FALLTHROUGH */
        case TMRM_HASH_CURSOR_NEXT_VALUE:
            vnode = cursor->current_value;

            /* copy value */
            value->data = vnode->value;
            value->size = vnode->value_len;

            /* move on */
            cursor->current_value = vnode->next;
            break;

        case TMRM_HASH_CURSOR_FIRST:
        case TMRM_HASH_CURSOR_NEXT:
            node = cursor->current_node;

            /* get key */
            key->data = node->key;
            key->size = node->key_len;

            /* if want values, walk through them */
            if (value) {
                vnode = cursor->current_value;

                /* get value */
                value->data = vnode->value;
                value->size = vnode->value_len;

                /* move on */
                cursor->current_value = vnode->next;

                /* stop here if there are more values, otherwise need next
                 * key & values so drop through and move to the next node
                 */
                if(cursor->current_value)
                    break;
            }

            /* move on to next node in current bucket */
            if (!(node = cursor->current_node->next)) {
                int i;

                /* end of list - move to next used bucket */
                for (i = cursor->current_bucket+1; i< cursor->hash->capacity; i++)
                    if ((node = cursor->hash->nodes[i])) {
                        cursor->current_bucket = i;
                        break;
                    }

            }

            if((cursor->current_node=node))
                cursor->current_value=node->values;

            break;
        default:
            /*
            tmrm_log(cursor->hash->hash->world,
                    0, TMRM_LOG_ERROR, TMRM_FROM_HASH, NULL,
                    "Unknown hash method flag %d", flags);
            */
            return 1;
    }

    return 0;
}


/**
 * Finish the serialisation of the hash memory get.
 *
 * @param context Hash memory get iterator context
 */
static void
tmrm_hash_memory_cursor_finish(void* context)
{
/* tmrm_hash_memory_cursor_context *cursor = (tmrm_hash_memory_cursor_context*)context; */

}


/**
 * Stores a key/value pair in the hash.
 *
 * @param context Memory hash context
 * @param key Pointer to key to store
 * @param value Pointer to value to store
 * @return Non 0 on failure
 */
static int
tmrm_hash_memory_put(void* context, tmrm_hash_datum *key, 
		       tmrm_hash_datum *value) 
{
    tmrm_hash_memory_context* hash = (tmrm_hash_memory_context*)context;
    tmrm_hash_memory_node *node;
    tmrm_hash_memory_node_value *vnode;
    uint32_t hash_key;
    void *new_key = NULL;
    void *new_value;
    int bucket = (-1);
    int is_new_node;

    /* ensure there is enough space in the hash */
    if (tmrm_hash_memory_expand_size(hash))
        return 1;

    /* find node for key */
    node = tmrm_hash_memory_find_node(hash,
            key->data, key->size,
            NULL, NULL);

    is_new_node = (node == NULL);

    /* not found - new key */
    if(is_new_node) {
        ONE_AT_A_TIME_HASH(hash_key, key->data, key->size);

        bucket = hash_key & (hash->capacity - 1);

        /* allocate new node */
        node = (tmrm_hash_memory_node*)TMRM_CALLOC(tmrm_hash_memory_node, 1,
                sizeof(tmrm_hash_memory_node));
        if (!node)
            return 1;

        node->hash_key = hash_key;

        /* allocate key for new node */
        new_key = TMRM_MALLOC(cstring, key->size);
        if (!new_key) {
            TMRM_FREE(tmrm_hash_memory_node, node);
            return 1;
        }

        /* copy new key */
        memcpy(new_key, key->data, key->size);
        node->key = new_key;
        node->key_len = key->size;
    }


    /* always allocate new value */
    new_value = TMRM_MALLOC(cstring, value->size);
    if (!new_value) {
        if (is_new_node) {
            TMRM_FREE(cstring, new_key);
            TMRM_FREE(tmrm_hash_memory_node, node);
        }
        return 1;
    }

    /* always allocate new tmrm_hash_memory_node_value */
    vnode = (tmrm_hash_memory_node_value*)TMRM_CALLOC(
        tmrm_hash_memory_node_value, 1, sizeof(tmrm_hash_memory_node_value));
    if (!vnode) {
        TMRM_FREE(cstring, new_value);
        if (is_new_node) {
            TMRM_FREE(cstring, new_key);
            TMRM_FREE(tmrm_hash_memory_node, node);
        }
        return 1;
    }

    /* if we get here, all allocations succeeded */


    /* put new value node in list */
    vnode->next = node->values;
    node->values = vnode;

    /* note that in counter */
    node->values_count++;

    /* copy new value */
    memcpy(new_value, value->data, value->size);
    vnode->value = new_value;
    vnode->value_len = value->size;


    /* now update buckets and hash counts */
    if(is_new_node) {
        node->next = hash->nodes[bucket];
        hash->nodes[bucket] = node;

        hash->keys++;
    }


    hash->values++;

    /* Only increase bucket count use when previous value was NULL */
    if (!node->next)
        hash->size++;

    return 0;
}


/**
 * Test the existence of a key in the hash.
 * 
 * @param context Memory hash context
 * @param key Key
 * @param value Value
 * @returns >0 if the key/value exists in the hash, 0 if not, <0 on failure
 */
static int
tmrm_hash_memory_exists(void* context, 
                          tmrm_hash_datum *key, tmrm_hash_datum *value)
{
    tmrm_hash_memory_context* hash = (tmrm_hash_memory_context*)context;
    tmrm_hash_memory_node* node;
    tmrm_hash_memory_node_value *vnode;

    node = tmrm_hash_memory_find_node(hash,
            (char*)key->data, key->size,
            NULL, NULL);
    /* key not found */
    if (!node)
        return 0;

    /* no value wanted */
    if (!value)
        return 1;

    /* search for value in list of values */
    for (vnode = node->values; vnode; vnode = vnode->next) {
        if (value->size == vnode->value_len && 
                !memcmp(value->data, vnode->value, value->size))
            break;
    }

    return (vnode != NULL);
}



/**
 * Deletes a key/value pair from the hash.
 * 
 * @param context Memory hash context
 * @param key Pointer to key to delete
 * @param value Pointer to value to delete
 * @returns Non 0 on failure
 */
static int
tmrm_hash_memory_delete_key_value(void* context, tmrm_hash_datum *key,
                                    tmrm_hash_datum *value)
{
    tmrm_hash_memory_context* hash = (tmrm_hash_memory_context*)context;
    tmrm_hash_memory_node *node, *prev, *next;
    tmrm_hash_memory_node_value *vnode, *vprev;
    int bucket;

    node = tmrm_hash_memory_find_node(hash, 
            (char*)key->data, key->size,
            &bucket, &prev);
    /* key not found anywhere */
    if(!node)
        return 1;

    /* search for value in list of values */
    vnode = node->values;
    vprev = NULL;
    while(vnode) {
        if(value->size == vnode->value_len && 
                !memcmp(value->data, vnode->value, value->size))
            break;
        vprev = vnode;
        vnode = vnode->next;
    }

    /* key/value combination not found */
    if (!vnode)
        return 1;

    /* found - delete it from list */
    if (!vprev) {
        /* at start of list so delete from there */
        node->values = vnode->next;
    } else
        vprev->next = vnode->next;

    /* free value and value node */
    if (vnode->value)
        TMRM_FREE(tmrm_hash_memory_node_value, vnode->value);
    TMRM_FREE(tmrm_hash_memory_node_value, vnode);

    /* update hash counts */
    hash->values--;

    /* check if last value was removed */
    if (node->values)
        /* no, so return success */
        return 0;


    /* yes - all values gone so need to delete entire key node */

    if (!prev) {
        /* is at start of list, so delete from there */
        if (!(hash->nodes[bucket] = node->next))
            /* hash bucket occupancy is one less if bucket is now empty */
            hash->size--;
        next = NULL;
    } else
        next = prev->next = node->next;

    /* free node */
    tmrm_hash_memory_node_free(node);

    /* see if there are remaining values for this key */
    if (!next) {
        /* no - so was last value for that key, reduce key count */
        hash->keys--;
    } else {
        int found = 0;
    
        node = next;
        while (node) {
            if (key->size == node->key_len && !memcmp(key, node->key, key->size)){
                found = 1;
                break;
            }
            node = node->next;
        }

        /* no further key values found - so was last value for that key */
        if (!found)
            hash->keys--;
    }

    return 0;
}


/**
 * Deletes a key and all its values from the hash.
 * 
 * @param context Memory hash context
 * @param key Pointer to key to delete
 * @return Non 0 on failure
 */
static int
tmrm_hash_memory_delete_key(void* context, tmrm_hash_datum *key) 
{
    tmrm_hash_memory_context* hash = (tmrm_hash_memory_context*)context;
    tmrm_hash_memory_node *node, *prev;
    int bucket;

    node = tmrm_hash_memory_find_node(hash, 
            (char*)key->data, key->size,
            &bucket, &prev);
    /* not found anywhere */
    if (!node)
        return 1;

    /* search list from here */
    if (!prev) {
        /* is at start of list, so delete from there */
        if (!(hash->nodes[bucket]=node->next))
            /* hash bucket occupancy is one less if bucket is now empty */
            hash->size--;
    } else
        prev->next=node->next;

    /* update hash counts */
    hash->keys--;
    hash->values-= node->values_count;

    /* free node */
    tmrm_hash_memory_node_free(node);
    return 0;
}


/**
 * Flush the hash to disk. Not used
 * 
 * @param context Memory hash context
 * @returns 0
 */
static int
tmrm_hash_memory_sync(void* context) 
{
    /* Not applicable */
    return 0;
}


/**
 * Get the file descriptor representing the hash. Not used.
 *
 * @param context Memory hash context
 * @returns -1
 */
static int
tmrm_hash_memory_get_fd(void* context) 
{
    /* Not applicable */
    return -1;
}


/* local function to register memory hash functions */

/**
 * Register the memory hash module with the hash factory.
 * 
 * @param factory Hash factory prototype
 */
static void
tmrm_hash_memory_register_factory(tmrm_hash_factory *factory) 
{
    factory->context_length = sizeof(tmrm_hash_memory_context);
    factory->cursor_context_length = sizeof(tmrm_hash_memory_cursor_context);

    factory->create  = tmrm_hash_memory_create;
    factory->destroy = tmrm_hash_memory_destroy;

    factory->open    = tmrm_hash_memory_open;
    factory->close   = tmrm_hash_memory_close;
    factory->clone   = tmrm_hash_memory_clone;

    factory->values_count = tmrm_hash_memory_values_count;

    factory->put     = tmrm_hash_memory_put;
    factory->exists  = tmrm_hash_memory_exists;
    factory->delete_key  = tmrm_hash_memory_delete_key;
    factory->delete_key_value  = tmrm_hash_memory_delete_key_value;
    factory->sync    = tmrm_hash_memory_sync;
    factory->get_fd  = tmrm_hash_memory_get_fd;

    factory->cursor_init   = tmrm_hash_memory_cursor_init;
    factory->cursor_get    = tmrm_hash_memory_cursor_get;
    factory->cursor_finish = tmrm_hash_memory_cursor_finish;
}

/**
 * Initialise the memory hash module.
 * 
 * Initialises the memory hash module and sets the default hash load factor.
 *
 * The recommended and current default value is 0.75, i.e. 750/1000.  
 * To use the default value (whatever it is) use a value less than 0.
 *
 * @param sms Subject map sphere object
 */
void
tmrm_init_hash_memory(tmrm_subject_map_sphere *sms) 
{
    /* use default load factor */
    if(sms->hash_load_factor <= 0 || sms->hash_load_factor > 999)
        sms->hash_load_factor = tmrm_hash_default_load_factor;

    tmrm_hash_register_factory(sms,
            "memory", &tmrm_hash_memory_register_factory);
}
