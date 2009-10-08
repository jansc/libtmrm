#!/bin/sh
ctags \
    --links=no --totals \
    -R --exclude=blib --exclude=.svn \
    --languages=c --langmap=c:+.h,c:+.y,c:+.lex
