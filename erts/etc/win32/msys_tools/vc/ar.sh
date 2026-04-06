#! /bin/sh
CMD=""
while test -n "$1" ; do
    x="$1"
    case "$x" in
	-out:)
	    shift
	    case "$1" in
		/*)
		    MPATH=`msys2win_path.sh -m $1`;;
		 *)
		    MPATH=$1;;
	    esac
	    CMD="$CMD -out:\"$MPATH\"";;
	-out:/*)
	    y=`echo $x | sed 's,^-out:\(/.*\),\1,g'`;
	    MPATH=`msys2win_path.sh -m $y`;
	    CMD="$CMD -out:\"$MPATH\"";;
	/*)
	    MPATH=`msys2win_path.sh -m $x`;
	    CMD="$CMD \"$MPATH\"";;
	*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    CMD="$CMD \"$y\"";;
    esac
    shift
done
eval lib.exe $CMD