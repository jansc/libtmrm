## Process this file with automake to produce Makefile.in
TESTS = tmrm_tests
check_PROGRAMS = tmrm_tests
tmrm_tests_SOURCES = tmrm_tests.c
tmrm_tests_CFLAGS = @CHECK_CFLAGS@
#tmrm_tests_LDADD = @CHECK_LIBS@
tmrm_tests_LDADD = @CHECK_LIBS@ $(top_builddir)/src/libtmrm.la
