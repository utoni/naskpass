#!/bin/sh

rm -f config.cache
mkdir -p build

echo "Looking in current directory for macros."
aclocal -I .
echo "Adding missing files."
automake --add-missing --force-missing
echo "Autoconf, Autoheader, Automake"
autoconf
autoheader
automake --foreign --add-missing --force-missing --copy
exit $?

