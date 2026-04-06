#! /bin/sh
BAT_FILE=/tmp/w$$.bat
if [ -z "$1" -o -z "$2" ]; then
    echo "Usage:" "$0" '<key> <valuename>'
    exit 1
fi
BACKED=`echo "$1" | sed 's,/,\\\\,g'`
if [ -d $WINDIR/sysnative ]; then
    REG_CMD="$WINDIR\\sysnative\\reg.exe"
else
    REG_CMD="reg.exe"
fi
cat > $BAT_FILE <<EOF
@echo off
$REG_CMD query "$BACKED" /v "$2"
EOF
RESULT=`cmd.exe //C $BAT_FILE`
echo $RESULT | sed "s,.*$2 REG_[^ ]* ,,"