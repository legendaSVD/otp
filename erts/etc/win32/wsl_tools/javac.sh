#! /bin/sh
CMD=""
SAVE="$@"
while test -n "$1" ; do
    x="$1"
    case "$x" in
	-I/*|-o/*|-d/*)
	    y=`echo $x | sed 's,^-[Iod]\(/.*\),\1,g'`;
	    z=`echo $x | sed 's,^-\([Iod]\)\(/.*\),\1,g'`;
	    MPATH=`wslpath -m $y`;
	    CMD="$CMD -$z\"$MPATH\"";;
	-d|-I|-o)
	    shift;
	    MPATH=`wslpath -m $1`;
	    CMD="$CMD $x $MPATH";;
	/*)
	    MPATH=`wslpath -m $x`;
	    CMD="$CMD \"$MPATH\"";;
	*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    CMD="$CMD \"$y\"";;
    esac
    shift
done
export WSLENV=CLASSPATH/l
eval javac.exe "$CMD"