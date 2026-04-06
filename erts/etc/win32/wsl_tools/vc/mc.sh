#! /bin/sh
SAVE="$@"
CMD=""
OUTPUT_DIRNAME=""
MCC=""
save_ifs=$IFS
IFS=:
for p in $PATH; do
    if [ -f $p/mc.exe ]; then
	if [ -n "`$p/mc.exe -? 2>&1 >/dev/null </dev/null \
                 | grep -i \"message compiler\"`" ]; then
	    MCC=`echo "mc.exe" | sed 's/ /\\\\ /g'`
            break
        else
            echo "Bad mc.exe in path"  >&2
            exit 1
	fi
    fi
done
IFS=$save_ifs
if [ -z "$MCC" ]; then
    echo 'mc.exe not found!' >&2
    exit 1
fi
while test -n "$1" ; do
    x="$1"
    case "$x" in
	-o)
	    shift
	    OUTPUT_DIRNAME="$1";;
	-o/*)
	    y=`echo $x | sed 's,^-[Io]\(/.*\),\1,g'`;
	    OUTPUT_DIRNAME="$y";;
	-I)
	    shift
	    MPATH=`w32_path.sh -d $1`;
	    CMD="$CMD -I\"$MPATH\"";;
	-I/*)
	    y=`echo $x | sed 's,^-[Io]\(/.*\),\1,g'`;
	    MPATH=`w32_path.sh -d $y`;
	    CMD="$CMD -I\"$MPATH\"";;
	*)
	    MPATH=`w32_path.sh -d -a $x`;
	    CMD="$CMD \"$MPATH\"";;
    esac
    shift
done
p=$$
if [ "X$MC_SH_DEBUG_LOG" != "X" ]; then
    echo mc.sh "$SAVE" >>$MC_SH_DEBUG_LOG
    echo mc.exe $CMD >>$MC_SH_DEBUG_LOG
fi
if [ -n "$OUTPUT_DIRNAME" ]; then
    cd $OUTPUT_DIRNAME
    RES=$?
    if [ "$RES" != "0" ]; then
	echo "mc.sh: Error: could not cd to $OUTPUT_DIRNAME">&2
	exit $RES
    fi
fi
eval $MCC "$CMD"  >/tmp/mc.exe.${p}.1 2>/tmp/mc.exe.${p}.2
RES=$?
if [ $RES != 0 ]; then
    echo Failed: $MCC "$CMD"
fi
tail -n +2 /tmp/mc.exe.${p}.2 >&2
cat /tmp/mc.exe.${p}.1
rm -f /tmp/mc.exe.${p}.2 /tmp/mc.exe.${p}.1
exit $RES