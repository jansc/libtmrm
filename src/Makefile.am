#SUBDIRS=win32

lib_LTLIBRARIES = libtmrm.la

#lib_LTLIBRARIES = libtmrm.la
#libtmrm_la_SOURCES = libtmrm.c libtmrm.h
#libtmrm_PROGRAM = libtmrm
#libtmrm_SOURCES = ...

include_HEADERS = libtmrm.h \
tmrm_types.h \
tmrm_internal.h \
tmrm_storage.h \
tmrm_storage_internal.h \
tmrm_hash.h \
tmrm_hash_internal.h \
tmrm_tuple.h \
tmrm_list.h


libtmrm_la_SOURCES = libtmrm.c tmrm_storage.c \
libtmrm.h \
tmrm_types.h \
tmrm_internal.h \
tmrm_literal.c \
tmrm_multiset.c \
tmrm_iterator.c \
tmrm_proxy.c \
tmrm_storage.h \
tmrm_storage_internal.h \
tmrm_tuple.h \
tmrm_tuple.c \
tmrm_hash.h \
tmrm_hash_internal.h \
tmrm_hash.c \
tmrm_hash_cursor.c \
tmrm_hash_memory.c \
tmrm_list.h \
tmrm_list.c


if STORAGE_POSTGRESQL
libtmrm_la_SOURCES += tmrm_storage_pgsql.c
endif


if STORAGE_BDB
libtmrm_la_SOURCES += tmrm_storage_db.c 
endif

libtmrm_la_LIBADD = \
@HASH_OBJS@ \
@LIBTMRM_INTERNAL_LIBS@

libtmrm_la_DEPENDENCIES = \
@HASH_OBJS@ \
@LIBTMRM_INTERNAL_LIBS@

libtmrm_la_LDFLAGS = -version-info @LIBTMRM_LIBTOOL_VERSION@ \
@LIBTMRM_LDFLAGS@ @LIBTMRM_EXTERNAL_LIBS@ # $(MEM_LIBS)

# Use tar, whatever it is called (better be GNU tar though)
TAR=@TAR@

# Why is this not in the default makefile?
CC=@CC@

AM_CPPFLAGS=@LIBTMRM_INTERNAL_CPPFLAGS@ @LIBTMRM_CPPFLAGS@ @LIBTMRM_EXTERNAL_CPPFLAGS@
AM_CFLAGS=@LIBTMRM_INTERNAL_CPPFLAGS@ @LIBTMRM_CPPFLAGS@ @LIBTMRM_EXTERNAL_CPPFLAGS@ # $(MEM)
STANDARD_CFLAGS=@STANDARD_CFLAGS@ # $(MEM)

LOCAL_LIB_DIR=lib

@SET_MAKE@




