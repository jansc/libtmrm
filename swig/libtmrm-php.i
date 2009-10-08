%typemap(in) (const unsigned char* str, size_t len)
{
    if (Z_TYPE_PP($input) == IS_STRING)
    {
        $1 = (char*)Z_STRVAL_PP($input);
        $2 = Z_STRLEN_PP($input);
    }
    else
    {
        SWIG_PHP_Error(E_ERROR, "Type error in argument $argnum of $symname. Expected string");
    }
}


%module libtmrm
%{
    #include "libtmrm.h"
%}

%include "../src/libtmrm.h"

%pragma(php5) phpinfo="
   php_info_print_table_start();
   php_info_print_table_header(2, \"libtmrm Support\", \"enabled\");
   php_info_print_table_row(2, \"libtmrm version\", \"FIXME\");
   php_info_print_table_row(2, \"libtmrm php bindings version\", \"FIXME\");
   php_info_print_table_end();
"

