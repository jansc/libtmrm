#!/bin/bash
swig -php libtmrm-php.i
gcc `php-config --includes` -c -I../src -L../src libtmrm_wrap.c
ld -bundle -flat_namespace -undefined suppress ../src/.libs/libtmrm.dylib libtmrm_wrap.o -o libtmrm.so

sudo cp libtmrm.so /opt/local/lib/php/extensions/no-debug-non-zts-20060613/
