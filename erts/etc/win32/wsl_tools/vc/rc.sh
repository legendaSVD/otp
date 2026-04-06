#! /bin/sh
SAVE="$@"
CMD=""
OUTPUT_FILENAME=""
RCC=""
save_ifs=$IFS
IFS=:
for p in $PATH; do
    if [ -f $p/rc.exe ]; then
	if [ -n "`$p/rc.exe -? 2>&1 | grep -i "resource compiler"`" ]; then
            RCC="rc.exe /nologo"
            break
        else
            echo "Bad rc.exe in path"  >&2
            exit 1
	fi
    fi
done
IFS=$save_ifs
if [ -z "$RCC" ]; then
    echo 'rc.exe not found!' >&2
    exit 1
fi
while test -n "$1" ; do
    x="$1"
    case "$x" in
	-o)
	    shift
	    MPATH=`w32_path.sh -d $1`;
	    OUTPUT_FILENAME="$MPATH";;
	-o/*)
	    y=`echo $x | sed 's,^-[Io]\(/.*\),\1,g'`;
	    MPATH=`w32_path.sh -d $y`;
	    OUTPUT_FILENAME="$MPATH";;
	-I)
	    shift
	    MPATH=`w32_path.sh -d $1`;
	    CMD="$CMD -I\"$MPATH\"";;
	-I/*)
	    y=`echo $x | sed 's,^-[Io]\(/.*\),\1,g'`;
	    MPATH=`w32_path.sh -d $y`;
	    CMD="$CMD -I\"$MPATH\"";;
	/*)
	    MPATH=`w32_path.sh -d $x`;
	    CMD="$CMD \"$MPATH\"";;
	*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    CMD="$CMD \"$y\"";;
    esac
    shift
done
p=$$
if [ -n "$OUTPUT_FILENAME" ]; then
    CMD="-Fo$OUTPUT_FILENAME $CMD"
fi
if [ "X$RC_SH_DEBUG_LOG" != "X" ]; then
    echo rc.sh "$SAVE" >>$RC_SH_DEBUG_LOG
    echo rc.exe /nologo $CMD >>$RC_SH_DEBUG_LOG
fi
eval $RCC "$CMD"  >/tmp/rc.exe.${p}.1 2>/tmp/rc.exe.${p}.2
RES=$?
if [ $RES != 0 ]; then
    echo Failed: $RCC "$CMD"
fi
tail -n +2 /tmp/rc.exe.${p}.2 >&2
cat /tmp/rc.exe.${p}.1
rm -f /tmp/rc.exe.${p}.2 /tmp/rc.exe.${p}.1
exit $RES