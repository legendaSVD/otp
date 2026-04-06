#! /bin/sh
CMD=""
CLASSPATH=`cygpath -m -p $CLASSPATH`
export CLASSPATH
SAVE="$@"
while test -n "$1" ; do
    x="$1"
    case "$x" in
	-I/*|-o/*|-d/*)
	    y=`echo $x | sed 's,^-[Iod]\(/.*\),\1,g'`;
	    z=`echo $x | sed 's,^-\([Iod]\)\(/.*\),\1,g'`;
	    MPATH=`cygpath -m $y`;
	    CMD="$CMD -$z\"$MPATH\"";;
	-d|-I|-o)
	    shift;
	    MPATH=`cygpath -m $1`;
	    CMD="$CMD $x $MPATH";;
	/*)
	    MPATH=`cygpath -m $x`;
	    CMD="$CMD \"$MPATH\"";;
	*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    CMD="$CMD \"$y\"";;
    esac
    shift
done
eval javac.exe $CMD