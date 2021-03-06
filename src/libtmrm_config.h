/* src/libtmrm_config.h.  Generated from libtmrm_config.h.in by configure.  */
/* src/libtmrm_config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* BDB has close method with 2 args */
/* #undef HAVE_BDB_CLOSE_2_ARGS */

/* BDB defines DBC */
/* #undef HAVE_BDB_CURSOR */

/* BDB cursor method has 4 arguments */
/* #undef HAVE_BDB_CURSOR_4_ARGS */

/* BDB defines DB_TXN */
/* #undef HAVE_BDB_DB_TXN */

/* BDB has fd method with 2 args */
/* #undef HAVE_BDB_FD_2_ARGS */

/* Have BDB support */
/* #undef HAVE_BDB_HASH */

/* BDB has open method with 6 args */
/* #undef HAVE_BDB_OPEN_6_ARGS */

/* BDB has open method with 7 args */
/* #undef HAVE_BDB_OPEN_7_ARGS */

/* BDB has set_flags method */
/* #undef HAVE_BDB_SET_FLAGS */

/* BDB has dbopen method */
/* #undef HAVE_DBOPEN */

/* BDB has db_create method */
/* #undef HAVE_DB_CREATE */

/* Define to 1 if you have the <db.h> header file. */
#define HAVE_DB_H 1

/* BDB has db_open method */
/* #undef HAVE_DB_OPEN */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <dmalloc.h> header file. */
/* #undef HAVE_DMALLOC_H */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <libpq-fe.h> header file. */
#define HAVE_LIBPQ_FE_H 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <yaml.h> header file. */
#define HAVE_YAML_H 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Name of package */
#define PACKAGE "libtmrm"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "jans@ravn.no"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libtmrm"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libtmrm 0.0.3"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libtmrm"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.0.3"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Building BDB storage */
/* #undef STORAGE_BDB */

/* Building PostgreSQL storage */
#define STORAGE_POSTGRESQL 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Version number of package */
#define VERSION "0.0.3"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to rpl_malloc if the replacement function should be used. */
/* #undef malloc */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */
