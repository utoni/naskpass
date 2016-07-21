#!/bin/sh
set -e
set -x

DIR=$(dirname $0)
PWD=$(pwd)

cd ${DIR}
./autogen.sh
./configure $*
make
cd ${PWD}
