#! /bin/sh
CMD=""
while test -n "$1" ; do
    x="$1"
    case "$x" in
	-out:)
	    shift
	    case "$1" in
		/*)
		    MPATH=`w32_path.sh -d $1`;;
		 *)
		    MPATH=$1;;
	    esac
	    CMD="$CMD -out:\"$MPATH\"";;
	-out:/*)
	    y=`echo $x | sed 's,^-out:\(/.*\),\1,g'`;
	    MPATH=`w32_path.sh -d $y`;
	    CMD="$CMD -out:\"$MPATH\"";;
	/*)
	    MPATH=`w32_path.sh -d $x`;
	    CMD="$CMD \"$MPATH\"";;
	*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    CMD="$CMD \"$y\"";;
    esac
    shift
done
eval lib.exe /nologo $CMD