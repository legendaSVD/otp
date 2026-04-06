#! /bin/sh
lookup_prog_in_path ()
{
    PROG=$1
    save_ifs=$IFS
    IFS=:
    for p in $PATH; do
	if [ -f $p/$PROG.exe ]; then
	    echo $p/$PROG
	    break;
	fi
    done
    IFS=$save_ifs
}
remove_path_element()
{
    EL=$1
    PA=$2
    ACC=""
    save_ifs=$IFS
    IFS=/
    set $PA
    N=$
    while [ $N -gt 1 ]; do
	if [ '!' -z "$1" ]; then
	    ACC="${ACC}/$1"
	fi
	N=`expr $N - 1`
	shift
    done
    UP=`echo $1 | tr [:lower:] [:upper:]`
    ELUP=`echo $EL | tr [:lower:] [:upper:]`
    IFS=$save_ifs
    if [ "$UP" = "$ELUP" ]; then
	echo "$ACC"
    else
	echo "${ACC}/$1"
    fi
}
add_path_element()
{
    EL=$1
    PA=$2
    ELUP=`echo $EL | tr [:lower:] [:upper:]`
    for x in ${PA}/*; do
	UP=`basename "$x" | tr [:lower:] [:upper:]`
	if [ "$UP" = "$ELUP" ]; then
	    echo "$x"
	    return 0;
	fi
    done
    echo "$PA"
}
if [ "$1" = "win64" ]; then
    AMD64DIR=true
    VCREDIST=vcredist_x64
    VCREDIST2=vcredist.x64
    COMPONENTS="cl amd64 bin vc"
elif [ "$1" = "win32" ]; then
    AMD64DIR=false
    VCREDIST=vcredist_x86
    VCREDIST2=vcredist.x86
    COMPONENTS="cl bin vc"
elif [ "$1" = "arm64" ]; then
    AMD64DIR=false
    VCREDIST=vcredist_arm64
    VCREDIST2=vcredist.arm64
else
    echo "TARGET argument should win32, win64 or arm64"
    exit 2
fi
if [ x"$VCToolsRedistDir" != x"" ]; then
    File="$VCToolsRedistDir/$VCREDIST.exe"
    if [ -r "$File" ]; then
	echo "$File"
	exit 0
    fi
    File="$VCToolsRedistDir/$VCREDIST2.exe"
    if [ -r "$File" ]; then
	echo "$File"
	exit 0
    fi
fi
CLPATH=`lookup_prog_in_path cl`
if [ -z "$CLPATH" ]; then
    echo "Can not locate cl.exe and vcredist_x86/x64/arm64.exe - OK if using mingw" >&2
    exit 1
fi
echo $CLPATH
BPATH=$CLPATH
for x in $COMPONENTS; do
    NBPATH=`remove_path_element $x "$BPATH"`
    if [ "$NBPATH" = "$BPATH" ]; then
	echo "Failed to locate $VCREDIST.exe because cl.exe was in an unexpected location" >&2
	exit 2
    fi
    BPATH="$NBPATH"
done
BPATH_LIST=$BPATH
RCPATH=`lookup_prog_in_path rc`
fail=false
if [ '!' -z "$RCPATH" ]; then
    BPATH=$RCPATH
    allow_fail=false
    if [ $AMD64DIR = true ]; then
	COMPONENTS="rc x64 bin @ANY v6.0A v7.0A v7.1"
    else
	COMPONENTS="rc bin @ANY v6.0A v7.0A v7.1"
    fi
    for x in $COMPONENTS; do
	if [ $x = @ANY ]; then
	    allow_fail=true
	else
	    NBPATH=`remove_path_element $x "$BPATH"`
	    if [ $allow_fail = false -a "$NBPATH" = "$BPATH" ]; then
		fail=true
		break;
	    fi
	    BPATH="$NBPATH"
	fi
    done
    if [ $fail = false ]; then
	BPATH_LIST="$BPATH_LIST $BPATH"
    fi
fi
for BP in $BPATH_LIST; do
    for verdir in "sdk v2.0" "sdk v3.5" "v6.0A" "v7.0"  "v7.0A" "v7.1" "VC redist 1033"; do
	BPATH=$BP
	fail=false
	allow_fail=false
	for x in $verdir @ANY bootstrapper packages $VCREDIST Redist VC @ALL $VCREDIST.exe; do
	    if [ $x = @ANY ]; then
		allow_fail=true
	    elif [ $x = @ALL ]; then
		allow_fail=false
	    else
		NBPATH=`add_path_element $x "$BPATH"`
		if [ $allow_fail = false -a "$NBPATH" = "$BPATH" ]; then
		    fail=true
		    break;
		fi
		BPATH="$NBPATH"
	    fi
	done
	if [ $fail = false ]; then
	    break;
	fi
    done
    if [ $fail = false ]; then
	echo $BPATH
	exit 0
    fi
done
if [ -f $ERL_TOP/$VCREDIST.exe ]; then
    echo $ERL_TOP/$VCREDIST.exe
    exit 0
fi
if [ -f $ERL_TOP/../$VCREDIST.exe ]; then
    echo $ERL_TOP/../$VCREDIST.exe
    exit 0
fi
echo "Failed to locate $VCREDIST.exe because directory structure was unexpected" >&2
exit 3