#!/bin/sh
echo `/usr/sbin/psrinfo -v | sed 's/.* \([0-9]*\)\ MHz.*/\1/;s/.*[^0-9].*//g'` | sed 's/ /+/g'