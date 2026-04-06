#! /bin/sh
CMD=""
save_IFS=$IFS
IFS=":"
NEWCLASSPATH=""
for x in $CLASSPATH; do
  TMP=`msys2win_path.sh -m $x`
  if [ -z "$NEWCLASSPATH" ]; then
      NEWCLASSPATH="$TMP"
  else
      NEWCLASSPATH="$NEWCLASSPATH;$TMP"
  fi
done
IFS=$save_IFS
CLASSPATH="$NEWCLASSPATH"
export CLASSPATH
SAVE="$@"
while test -n "$1" ; do
    x="$1"
    case "$x" in
	-I/*|-o/*|-d/*)
	    y=`echo $x | sed 's,^-[Iod]\(/.*\),\1,g'`;
	    z=`echo $x | sed 's,^-\([Iod]\)\(/.*\),\1,g'`;
	    MPATH=`msys2win_path.sh -m $y`;
	    CMD="$CMD -$z\"$MPATH\"";;
	-d|-I|-o)
	    shift;
	    MPATH=`msys2win_path.sh -m $1`;
	    CMD="$CMD $x $MPATH";;
	/*)
	    MPATH=`msys2win_path.sh -m $x`;
	    CMD="$CMD \"$MPATH\"";;
	*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    CMD="$CMD \"$y\"";;
    esac
    shift
done
eval javac.exe "$CMD"