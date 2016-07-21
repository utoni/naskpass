#!/bin/sh

set -x

rm -f config.cache
mkdir -p build

aclocal -I .
automake --add-missing --force-missing
autoconf
autoheader
automake --foreign --add-missing --force-missing --copy
exit $?

