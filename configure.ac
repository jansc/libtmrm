dnl Process this file with autoconf to produce a configure script.
dnl
dnl configure.ac - top level autoconf file for libtmrm
dnl http://libtmrm.ravn.no
dnl
dnl This file is licensed under the 
dnl GNU Lesser General Public License (LGPL) V2.1 or any newer version
dnl
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Library General Public License as
dnl published by the Free Software Foundation; either version 2 of the
dnl License, or (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Library General Public License for more details.
dnl
dnl You should have received a copy of the GNU Library General Public
dnl License along with this program; if not, write to the
dnl Free Software Foundation, Inc., 59 Temple Place - Suite 330,
dnl Boston, MA 02111-1307, USA.
dnl
dnl Copyright (C) 2008 Jan Schreiber http://purl.org/net/jans
dnl Copyright (C) 2008 Ravn Webveveriet AS, NO http://www.ravn.no
dnl
dnl Parts of the code are taken from the Redland library, http://librdf.org
dnl
dnl Copyright (C) 2000-2007 David Beckett http://purl.org/net/dajobe/
dnl Copyright (C) 2000-2005 University of Bristol, UK http://www.bristol.ac.uk/
dnl 

# Prelude.
AC_INIT([libtmrm], [0.0.3], [jans@ravn.no])
AC_PREREQ([2.53])
AC_CONFIG_MACRO_DIR(m4)


AM_INIT_AUTOMAKE([1.7 check-news std-options -Wobsolete -Wportability -Wsyntax -Wunsupported])
AC_CONFIG_SRCDIR(src/libtmrm.c)
AM_CONFIG_HEADER(src/libtmrm_config.h)
AM_MAINTAINER_MODE

AC_CANONICAL_HOST

# place to put some extra build scripts installed
#AC_CONFIG_AUX_DIR([build-aux])

# Libtool versioning
#
# CURRENT
#      The most recent interface number that this library implements.
# 
# REVISION
#      The implementation number of the CURRENT interface.
# 
# AGE
#      The difference between the newest and oldest interfaces that this
#      library implements.  In other words, the library implements all the
#      interface numbers in the range from number `CURRENT - AGE' to
#      `CURRENT'.
#
# Rules:
#  1. Start with version information of `0:0:0' for each libtool library.
#
#  2. Update the version information only immediately before a public
#     release of your software.  More frequent updates are unnecessary,
#     and only guarantee that the current interface number gets larger
#     faster.
#
#  3. If the library source code has changed at all since the last
#     update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
#
#  4. If any interfaces have been added, removed, or changed since the
#     last update, increment CURRENT, and set REVISION to 0.
#
#  5. If any interfaces have been added since the last public release,
#     then increment AGE.
#
#  6. If any interfaces have been removed since the last public release,
#     then set AGE to 0.
#
# syntax: CURRENT[:REVISION[:AGE]]
LIBTMRM_LIBTOOL_VERSION=0:0:0
AC_SUBST(LIBTMRM_LIBTOOL_VERSION)


# Check for programs.
AM_PROG_CC_STDC
AM_PROG_CC_C_O
AM_SANITY_CHECK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

m4_undefine([AC_PROG_CXX])
m4_defun([AC_PROG_CXX],[])
m4_undefine([AC_PROG_F77])
m4_defun([AC_PROG_F77],[])

AC_LIBTOOL_DLOPEN
AC_LIBLTDL_INSTALLABLE
AC_PROG_LIBTOOL


# Find a tar command for 'make dist'
AC_CHECK_PROGS(TAR, gnutar gtar tar)

AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)


# Externally linked libraries - appear in redland-config
# -Dfoo -Idir
LIBTMRM_CPPFLAGS=$CPPFLAGS
# -Ldir
LIBTMRM_LDFLAGS=$LDFLAGS
# -llib
LIBTMRM_LIBS=$LIBS

# do these first because somehow they get messed up by bdb checks
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Check for libraries.

# This macro is defined in check.m4 and tests if check.h and
# libcheck.a are installed in your system. It sets CHECK_CFLAGS and
# CHECK_LIBS accordingly.
#  AM_PATH_CHECK([MINIMUM-VERSION,
#                [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([memset])
AC_FUNC_MALLOC
# Used by lex:
AC_CHECK_FUNCS([strdup])

# Used by swig:
AC_CHECK_FUNCS([strstr])

# Profiling and coverage if we use gcc (to be tested)
#if test "$GCC" = "yes"
#then
#   AC_PATH_PROG([PROFILER], [gcov], "no", [$PATH])
#   if test "x$PROFILER" != "xno"; then
#      PROFILER_FLAGS="-fprofile-arcs -ftest-coverage"
#   fi
#else
#   AC_RUN_IFELSE(
#      [AC_LANG_PROGRAM([], [dnl
##ifdef __SUNPRO_C
#   return 0;
##else
#   return 1;
##endif
#      ])
#    ],[
#       AC_PATH_PROG([PROFILER], [tcov], "no", [$PATH])
#       if test "x$PROFILER" != "xno"; then
#          PROFILER_FLAGS=-xprofile=tcov
#       fi
#    ])
#fi
#
#AC_SUBST(PROFILER_FLAGS)


dnl Berkeley DB
dnl Copied from the redland configure.ac
AC_ARG_WITH(bdb, [  --with-bdb=DIR          Berkeley DB install area (default=/usr)], bdb_prefix="$withval", bdb_prefix="none") 
AC_ARG_WITH(bdb_lib, [  --with-bdb-lib=DIR      Berkeley DB lib directory (default=/usr/lib)], bdb_lib_dir="$withval", bdb_lib_dir="none") 
AC_ARG_WITH(bdb_include, [  --with-bdb-include=DIR  Berkeley DB include directory (default=/usr/include)], bdb_include_dir="$withval", bdb_include_dir="none") 
AC_ARG_WITH(bdb_dbname, [  --with-bdb-dbname=NAME  Berkeley DB library name (auto)], bdb_dbname="$withval", bdb_dbname="none")

bdb_version=unknown
bdb_available=Missing

if test "x$bdb_prefix" != "xno" ; then

  if test "x$bdb_prefix" = "xyes" ; then
    bdb_prefix="none"
  fi

  # Only prefix given - set lib and include
  if test "X$bdb_prefix" != "Xnone" -a "X$bdb_lib_dir" = "Xnone" -a "X$bdb_include_dir" = "Xnone" ; then
    bdb_lib_dir="$bdb_prefix/lib"
    bdb_include_dir="$bdb_prefix/include"
  fi

  # Nothing given - search
  if test "X$bdb_prefix" = "Xnone" -a "X$bdb_lib_dir" = "Xnone" -a "X$bdb_include_dir" = "Xnone" -a "X$bdb_dbname" = "Xnone" ; then
    bdb_prefix="/usr"
    bdb_lib_dir="/usr/lib"
    bdb_include_dir="/usr/include"

    extraprefix=
    # OSX fink area added if found and /sw/bin is in the PATH
    if test -d /sw/include -a -d /sw/lib ; then
      if echo $PATH |grep /sw/bin >/dev/null; then
	extraprefix=/sw
      fi
    fi

    # Sigh!  And this still might not be good enough.
    for bdbc_version in 4.6 4.5 4.4 4.3 4.2 4.1 4.0 4 3.3 3.2 3.1 3 2; do
      bdbc_major=`echo $bdbc_version | sed -e 's/^\(.\).*$/\1/'`;
      bdbc_version2=`echo $bdbc_version | sed -e 's/^\(.\).\(.\)$/\1\2/'`

      for bdbc_prefix in $extraprefix /usr/local/BerkeleyDB.$bdbc_version /usr/local /opt/local /usr; do
	bdbc_libdir=$bdbc_prefix/lib

	for bdbc_incdir in $bdbc_prefix/include/db$bdbc_version $bdbc_prefix/include/db$bdbc_version2 $bdbc_prefix/include/db$bdbc_major $bdbc_prefix/include ; do
	  if test -r $bdbc_incdir/db.h ; then
	    header_bdbc_version=`sed -ne 's/^.*DB_VERSION_STRING.*Berkeley DB \(...\).*/\1/p' $bdbc_incdir/db.h`
	    if test "X$header_bdbc_version" = "X$bdbc_version" ; then 
	      for bdbc_name in db-$bdbc_version db$bdbc_version db-$bdbc_version2 db$bdbc_version2; do
		if test -r $bdbc_libdir/lib$bdbc_name.a -o -r $bdbc_libdir/lib$bdbc_name.so -o -r $bdbc_libdir/lib$bdbc_name.dylib; then
		  bdb_version=$bdbc_version
		  bdb_prefix=$bdbc_prefix
		  bdb_include_dir=$bdbc_incdir
		  bdb_lib_dir=$bdbc_libdir
		  bdb_dbname=$bdbc_name
		  break 4
		fi
	      done
	    fi
	  fi

	done
       done
     done

  fi

  AC_MSG_CHECKING(BDB version)
  AC_MSG_RESULT($bdb_version)
  AC_MSG_CHECKING(location of BDB libs)
  AC_MSG_RESULT($bdb_lib_dir)
  AC_MSG_CHECKING(location of BDB includes)
  AC_MSG_RESULT($bdb_include_dir)

  if test "$bdb_prefix" != no; then
    nLDFLAGS=$LDFLAGS
    nCPPFLAGS=

    if test "$bdb_lib_dir" != /usr/lib; then
      nLDFLAGS="-L$bdb_lib_dir";
    fi

    if test "$bdb_dbname" = none; then
      # Yuck, yuck, yuck - can't they create a bdb-config program like GNOME?
      bdb_dbname=`cd $bdb_lib_dir; ls -1 libdb*.a 2>/dev/null |sed -e 's/^lib//' -e 's/\.a$//' | head -1`
      if test "X$bdb_dbname" = X; then
	AC_MSG_WARN(Cannot find the BDB library name from the files in $bdb_lib_dir)
	AC_MSG_WARN(Assuming it is 'db' and will link like -ldb)
	AC_MSG_WARN(Use --with-bdb-dbname=NAME if this is wrong)
	bdb_dbname=db
      fi
    fi

    AC_MSG_CHECKING(name of BDB library)
    AC_MSG_RESULT($bdb_dbname)

    if test "$bdb_include_dir" != /usr/include; then
      nCPPFLAGS="-I$bdb_include_dir";
    fi

    LDFLAGS="$nLDFLAGS $LDFLAGS"
    CPPFLAGS="$nCPPFLAGS $CPPFLAGS"
    LIBS="-l$bdb_dbname $LIBS"

    AC_CHECK_HEADERS(db.h)
    if test "$ac_cv_header_db_h" = yes ; then
      have_libdb=no

      AC_MSG_CHECKING(for BDB V1 dbopen)
      AC_TRY_LINK([#include <stdio.h>
		   #include <db.h>], [dbopen(NULL, 0, 0, DB_BTREE, NULL);],
		  AC_DEFINE(HAVE_DBOPEN, 1, [BDB has dbopen method])
		  if test $have_libdb = no; then
		    bdb_available="Version 1.x - not supported"
		    have_libdb=no
		  fi
		  AC_MSG_RESULT(yes),
		  AC_MSG_RESULT(no))

      AC_MSG_CHECKING(for BDB V2 db_open)
      AC_TRY_LINK([#include <stdio.h>
		   #include <db.h>], [db_open(NULL, DB_BTREE, 0, 0, NULL, NULL, NULL);],
		  AC_DEFINE(HAVE_DB_OPEN, 1, [BDB has db_open method])
		  have_libdb=yes
		  AC_MSG_RESULT(yes),
		  AC_MSG_RESULT(no))

      AC_MSG_CHECKING(for BDB V3/V4.x db_create)
      AC_TRY_LINK([#include <stdio.h>
		   #include <db.h>], [db_create(NULL, NULL, 0);],
		  AC_DEFINE(HAVE_DB_CREATE, 1, [BDB has db_create method])
		  have_libdb=yes
		  AC_MSG_RESULT(yes),
		  AC_MSG_RESULT(no))

      AC_MSG_CHECKING(for BDB V4.0 DB->open with 6 arguments)
      AC_TRY_LINK([#include <stdio.h>
		   #include <db.h>], [DB *bdb; bdb->open(bdb, NULL, NULL, DB_BTREE, 0, 0);],
		  AC_DEFINE(HAVE_BDB_OPEN_6_ARGS, 1, [BDB has open method with 6 args])
		  have_libdb=yes
		  AC_MSG_RESULT(yes),
		  AC_MSG_RESULT(no))

      AC_MSG_CHECKING(for BDB DB->close with 2 arguments)
      AC_TRY_LINK([#include <stdio.h>
		   #include <db.h>], [DB *bdb; bdb->close(bdb, 0);],
		  AC_DEFINE(HAVE_BDB_CLOSE_2_ARGS, 1, [BDB has close method with 2 args])
		  AC_MSG_RESULT(yes),
		  AC_MSG_RESULT(no))

      AC_MSG_CHECKING(for BDB DB->fd with 2 arguments)
      AC_TRY_LINK([#include <stdio.h>
		   #include <db.h>], [DB *bdb; bdb->fd(bdb, 0);],
		  AC_DEFINE(HAVE_BDB_FD_2_ARGS, 1, [BDB has fd method with 2 args])
		  AC_MSG_RESULT(yes),
		  AC_MSG_RESULT(no))

      AC_MSG_CHECKING(for BDB DB->set_flags)
      AC_TRY_LINK([#include <stdio.h>
		   #include <db.h>], [DB *bdb; bdb->set_flags(bdb, 0);],
		  AC_DEFINE(HAVE_BDB_SET_FLAGS, 1, [BDB has set_flags method])
		  AC_MSG_RESULT(yes),
		  AC_MSG_RESULT(no))

      AC_MSG_CHECKING(for BDB V4.1+ DB->open with 7 arguments)
      AC_TRY_LINK([#include <stdio.h>
		   #include <db.h>], [DB *bdb; bdb->open(bdb, NULL, NULL, NULL, DB_BTREE, 0, 0);],
		  AC_DEFINE(HAVE_BDB_OPEN_7_ARGS, 1, [BDB has open method with 7 args])
		  have_libdb=yes
		  AC_MSG_RESULT(yes),
		  AC_MSG_RESULT(no))

      AC_MSG_CHECKING(whether DB_TXN defined in db.h)
      dnl In BDB, DB_TXN is a pointer to a structure never defined
      AC_TRY_LINK([#include <stdio.h>
		   #include <db.h>], [DB_TXN* ptr=(DB_TXN*)NULL],
		  AC_DEFINE(HAVE_BDB_DB_TXN, 1, [BDB defines DB_TXN])
		  AC_MSG_RESULT(yes),
		  AC_MSG_RESULT(no))
      AC_MSG_CHECKING(whether DBC defined in db.h)
      AC_TRY_LINK([#include <db.h>], [size_t len=sizeof(DBC)],
		  AC_DEFINE(HAVE_BDB_CURSOR, 1, [BDB defines DBC])
		  AC_MSG_RESULT(yes),
		  AC_MSG_RESULT(no))
      AC_MSG_CHECKING(number of arguments to db_cursor)
      AC_TRY_LINK([#include <stdio.h>
		   #include <db.h>], [DB* db; db->cursor(db, NULL, NULL, 0);],
		  AC_DEFINE(HAVE_BDB_CURSOR_4_ARGS, 1, [BDB cursor method has 4 arguments])
		  AC_MSG_RESULT(4),
		  AC_MSG_RESULT(3))
    fi

    if test "$have_libdb" = yes; then
      bdb_available="Version $bdb_version (library $bdb_dbname in $bdb_lib_dir)"
      LIBTMRM_LIBS="$LIBTMRM_LIBS $nLDFLAGS -l$bdb_dbname"
      LIBTMRM_CPPFLAGS="$LIBTMRM_CPPFLAGS $nCPPFLAGS"
    fi

  fi

fi


CPPFLAGS="$LIBTMRM_CPPFLAGS"
LDFLAGS="$LIBTMRM_LDFLAGS"
LIBS="$LIBTMRM_LIBS"

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h stdlib.h unistd.h string.h fcntl.h dmalloc.h time.h sys/time.h sys/stat.h getopt.h limits.h malloc.h)
AC_HEADER_TIME

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_BIGENDIAN

dnl hashes
HASH_OBJS=
HASH_SRCS=

AC_MSG_CHECKING(for bdb hash support)
if test "$have_libdb" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_BDB_HASH, 1, [Have BDB support])
  HASH_OBJS="$HASH_OBJS tmrm_storage_db.lo"
  HASH_SRCS="$HASH_SRCS tmrm_storage_db.c"
else
  AC_MSG_RESULT(no)
fi


AC_SUBST(HASH_OBJS)
AC_SUBST(HASH_SRCS)

YAML_REQUIRE(0.1.1)

#AC_MSG_CHECKING([for libyaml])
#AC_ARG_WITH(yaml,AC_HELP_STRING([--with-yaml=DIR],[prefix for the libyaml headers and library]),
#        [AC_SUBST([DISTCHECK_CONFIGURE_FLAGS],
#            ["$DISTCHECK_CONFIGURE_FLAGS '--with-yaml=$with_yaml'"]))
#    [yaml_dir=$withval],
#    [])
#LIBS="$LIBTMRM_LIBS -L/opt/local/lib"
#LDFLAGS="$LIBTMRM_LDFLAGS -lyaml"
#CPPFLAGS="$LIBTMRM_CPPFLAGS -I/opt/local/include" 
#AC_CHECK_HEADERS(yaml.h)


dnl Storages
persistent_storages="/postgresql/"
persistent_store=no
all_storages="bdb postgresql"
always_available_storages=""

dnl default availabilities and enablements
for storage in $all_storages; do
  eval $storage'_storage_available=no'
  eval $storage'_storage=no'
done
for storage in $always_available_storages; do
  eval $storage'_storage_available=yes'
  eval $storage'_storage=yes'
done

storages_available=""
if test "$have_libdb" = yes; then
  storages_available="$storages_available bdb($bdb_version)"
fi


# Copied from librdf...
AC_ARG_WITH(postgresql, [  --with-postgresql(=CONFIG|yes|no)  Location of pg_config (default=auto)], pg_config="$withval", postgresql_config="")
if test "X$pg_config" != "Xno"; then
  if test "X$pg_config" != "Xyes" ; then
    PG_CONFIG=$pg_config
  fi
  pg_config=yes
fi
if test $pg_config = yes -a "X$PG_CONFIG" = "X" ; then
  AC_CHECK_PROGS(PG_CONFIG, pg_config)
fi

AC_MSG_CHECKING(for postgresql library)
if test "X$PG_CONFIG" != X ; then
  postgresql_version=`$PG_CONFIG --version | sed -e 's/^.* //'`
  storages_available="$storages_available postgresql($postgresql_version)"
  postgresql_storage_available=yes

  AC_MSG_RESULT(yes - version $postgresql_version)

  CPPFLAGS="$LIBTMRM_CPPFLAGS -I`$PG_CONFIG --includedir`"
  AC_CHECK_HEADERS(libpq-fe.h)
  CPPFLAGS="$LIBTMRM_CPPFLAGS"

  LIBS="$LIBTMRM_LIBS -L`$PG_CONFIG --libdir` -lpq"
else
  AC_MSG_RESULT(no)
fi


# This is needed because autoheader can't work out which computed
# symbols must be pulled from acconfig.h into config.h.in
if test "x" = "y"; then
#  AC_DEFINE(STORAGE_FILE,   1, [Building file storage])
  AC_DEFINE(STORAGE_BDB, 1, [Building BDB storage])
#  AC_DEFINE(STORAGE_MEMORY, 1, [Building memory storage])
#  AC_DEFINE(STORAGE_MYSQL,  1, [Building MySQL storage])
#  AC_DEFINE(STORAGE_SQLITE, 1, [Building SQLite storage])
#  AC_DEFINE(STORAGE_TSTORE, 1, [Building 3store storage])
  AC_DEFINE(STORAGE_POSTGRESQL, 1, [Building PostgreSQL storage])
fi


AC_MSG_CHECKING(storages required)
AC_ARG_ENABLE(storages, [  --enable-storages=LIST   Use storages (default=all)], storages="$enableval")
if test "X$storages" = Xall -o "X$storages" = X; then
  storages="$all_storages"
  AC_MSG_RESULT(all)
else
  AC_MSG_RESULT($storages)
fi

for storage in $storages; do
  if eval test \$$storage'_storage_available' = yes; then
    eval $storage'_storage=yes'
    NAME=`echo $storage | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
    n=STORAGE_${NAME}
    AC_DEFINE_UNQUOTED($n)
    storages_enabled="$storages_enabled $storage"

    if echo $persistent_storages | grep "/$storage/" >/dev/null; then
      persistent_store=yes
    fi
  fi

done

# AM_CONDITIONAL(STORAGE_FILE,   test $file_storage   = yes)
AM_CONDITIONAL(STORAGE_BDB, test $bdb_storage = yes)
# AM_CONDITIONAL(STORAGE_MEMORY, test $memory_storage = yes)
# AM_CONDITIONAL(STORAGE_MYSQL,  test $mysql_storage  = yes)
# AM_CONDITIONAL(STORAGE_SQLITE, test $sqlite_storage = yes)
# AM_CONDITIONAL(STORAGE_TSTORE, test $tstore_storage = yes)
AM_CONDITIONAL(STORAGE_POSTGRESQL, test $postgresql_storage = yes)


if test $postgresql_storage  = yes; then
    POSTGRESQL_CPPFLAGS="-I`$PG_CONFIG --includedir`"
    POSTGRESQL_LIBS="-L`$PG_CONFIG --libdir` -lpq"
fi

AC_SUBST(POSTGRESQL_CPPFLAGS)
AC_SUBST(POSTGRESQL_LIBS)

LIBTMRM_CPPFLAGS="$LIBTMRM_CPPFLAGS $POSTGRESQL_CPPFLAGS $YAML_CPPFLAGS"
LIBTMRM_LIBS="$LIBTMRM_LIBS $POSTGRESQL_LIBS"

CPPFLAGS="$LIBTMRM_CPPFLAGS"
LDFLAGS="$LIBTMRM_LDFLAGS $YAML_LDFLAGS"
LIBS="$LIBTMRM_LIBS"

LIBTMRM_LIBTOOLLIBS=libtmrm.la

AC_SUBST(LIBTMRM_EXTERNAL_CPPFLAGS)
AC_SUBST(LIBTMRM_EXTERNAL_LIBS)

AC_SUBST(LIBTMRM_INTERNAL_CPPFLAGS)
AC_SUBST(LIBTMRM_INTERNAL_LIBS)

AC_SUBST(LIBTMRM_CPPFLAGS)
AC_SUBST(LIBTMRM_LDFLAGS)
AC_SUBST(LIBTMRM_LIBTOOLLIBS)
AC_SUBST(LIBTMRM_PKGCONFIG_LIBS)

# for use with dmalloc
# AC_SUBST(MEM)
# AC_SUBST(MEM_LIBS)
AC_SUBST(STANDARD_CFLAGS)

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
#AC_FUNC_MALLOC

# Output files

abs_top_srcdir=`cd $srcdir; pwd`
AC_SUBST(abs_top_srcdir)
abs_top_builddir=`pwd`
AC_SUBST(abs_top_builddir)

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile])

AC_OUTPUT

AC_MSG_RESULT([
libtmrm build summary:
])

AC_MSG_RESULT([
  Berkeley/Sleepycat DB    : $bdb_available
  Backend stores available :$storages_available
  Backend stores enabled   :$storages_enabled
])
