#! /bin/sh
SAVE="$@"
CMD=""
OUTPUT_FILENAME=""
if [ -z "$MINGW_EXE_PATH" ]; then
    echo "You have to set MINGW_EXE_PATH to run rc.sh" >&2
    exit 1
fi
RCC=$MINGW_EXE_PATH/windres.exe
if [ -z "$RCC" ]; then
    echo 'windres.exe not found!' >&2
    exit 1
fi
while test -n "$1" ; do
    x="$1"
    case "$x" in
	-o)
	    shift
	    MPATH=`cygpath -m $1`;
	    OUTPUT_FILENAME="$MPATH";;
	-o/*)
	    y=`echo $x | sed 's,^-[Io]\(/.*\),\1,g'`;
	    MPATH=`cygpath -m $y`;
	    OUTPUT_FILENAME="$MPATH";;
	-I)
	    shift
	    MPATH=`cygpath -m $1`;
	    CMD="$CMD -I\"$MPATH\"";;
	-I/*)
	    y=`echo $x | sed 's,^-[Io]\(/.*\),\1,g'`;
	    MPATH=`cygpath -m $y`;
	    CMD="$CMD -I\"$MPATH\"";;
	/*)
	    MPATH=`cygpath -m $x`;
	    CMD="$CMD \"$MPATH\"";;
	*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    CMD="$CMD \"$y\"";;
    esac
    shift
done
p=$$
if [ -n "$OUTPUT_FILENAME" ]; then
    CMD="-o $OUTPUT_FILENAME $CMD"
fi
if [ "X$RC_SH_DEBUG_LOG" != "X" ]; then
    echo rc.sh "$SAVE" >>$RC_SH_DEBUG_LOG
    echo windres.exe $CMD >>$RC_SH_DEBUG_LOG
fi
eval $RCC "$CMD"  >/tmp/rc.exe.${p}.1 2>/tmp/rc.exe.${p}.2
RES=$?
cat /tmp/rc.exe.${p}.2 >&2
cat /tmp/rc.exe.${p}.1
rm -f /tmp/rc.exe.${p}.2 /tmp/rc.exe.${p}.1
exit $RES