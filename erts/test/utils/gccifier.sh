#!/bin/sh
CC=`echo "$1" | sed -e "s/-CC//"`
shift
echo "->"
echo "$CC $*"
$CC $*
echo ""