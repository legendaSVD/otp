#! /bin/sh
SAVE="$@"
COMMON_CFLAGS="-nologo -D__WIN32__ -DWIN32 -DWINDOWS -D_WIN32 -DNT -D_CRT_SECURE_NO_DEPRECATE"
MSG_FILE=/tmp/cl.exe.$$.1
ERR_FILE=/tmp/cl.exe.$$.2
MD_FORCED=false
PREPROCESSING=false
DEPENDENCIES=false
DEBUG_BUILD=false
OPTIMIZED_BUILD=false
LINKING=true
MD=-MD
DEBUG_FLAGS=""
OPTIMIZE_FLAGS=""
OUTFILE=""
CMD=""
SOURCES=""
LINKCMD=""
while test -n "$1" ; do
    x="$1"
    case "$x" in
	-Wall)
	    ;;
	-c)
	    LINKING=false;;
	-MM)
	    PREPROCESSING=true;
	    LINKING=false;
	    DEPENDENCIES=true;;
	-E)
	    PREPROCESSING=true;
	    LINKING=false;;
	-Owx)
	    OPTIMIZE_FLAGS="-Ob2ity -Gs -Zi";
	    DEBUG_FLAGS="";
	    DEBUG_BUILD=false;
	    if [ $MD_FORCED = false ]; then
		MD=-MD;
	    fi
	    OPTIMIZED_BUILD=true;;
        -O0|-Og)
            ;;
	-O*)
	    OPTIMIZE_FLAGS="-Ox -Zi";
	    DEBUG_FLAGS="";
	    DEBUG_BUILD=false;
	    if [ $MD_FORCED = false ]; then
		MD=-MD;
	    fi
	    OPTIMIZED_BUILD=true;;
	-g|-ggdb)
	    if [ $OPTIMIZED_BUILD = false ];then
		DEBUG_FLAGS="-Z7";
		if [ $MD_FORCED = false ]; then
		    MD=-MDd;
		fi
		LINKCMD="$LINKCMD -g";
		DEBUG_BUILD=true;
	    fi;;
	-mt|-MT)
	    MD="-MT";
	    MD_FORCED=true;;
	-md|-MD)
	    MD="-MD";
	    MD_FORCED=true;;
	-ml|-ML)
	    MD="-ML";
	    MD_FORCED=true;;
	-mdd|-MDD|-MDd)
	    MD="-MDd";
	    MD_FORCED=true;;
	-mtd|-MTD|-MTd)
	    MD="-MTd";
	    MD_FORCED=true;;
	-mld|-MLD|-MLd)
	    MD="-MLd";
	    MD_FORCED=true;;
	-o)
	    shift;
	    OUTFILE="$1";;
	-o*)
	    y=`echo $x | sed 's,^-[Io]\(.*\),\1,g'`;
	    OUTFILE="$y";;
	-I/*)
	    y=`echo $x | sed 's,^-[Io]\(/.*\),\1,g'`;
	    z=`echo $x | sed 's,^-\([Io]\)\(/.*\),\1,g'`;
	    MPATH=`cygpath -m $y`;
	    CMD="$CMD -$z\"$MPATH\"";;
	-I*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    CMD="$CMD $y";;
	-D*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    CMD="$CMD $y";;
	-EH*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    CMD="$CMD $y";;
	-l*)
	    y=`echo $x | sed 's,^-l\(.*\),\1,g'`;
	    LINKCMD="$LINKCMD $x";;
	/*.c)
	    SOURCES="$SOURCES $x";;
	*.c)
	    SOURCES="$SOURCES $x";;
	/*.cc)
	    SOURCES="$SOURCES $x";;
	*.cc)
	    SOURCES="$SOURCES $x";;
	/*.cpp)
	    SOURCES="$SOURCES $x";;
	*.cpp)
	    SOURCES="$SOURCES $x";;
	/*.o)
	    LINKCMD="$LINKCMD $x";;
	*.o)
	    LINKCMD="$LINKCMD $x";;
	*)
	    y=`echo $x | sed 's,",\\\",g'`;
	    LINKCMD="$LINKCMD $y";;
    esac
    shift
done
RES=0
ACCUM_OBJECTS=""
TMPOBJDIR=/tmp/tmpobj$$
mkdir $TMPOBJDIR
for x in $SOURCES; do
    start_time=`date '+%s'`
    if [ $LINKING = false ]; then
	case $OUTFILE in
	    /*.o)
		n=`echo $SOURCES | wc -w`;
		if [ $n -gt 1 ]; then
		    echo "cc.sh:Error, multiple sources, one object output.";
		    exit 1;
		else
		    output_filename=`cygpath -m $OUTFILE`;
		fi;;
	    *.o)
		n=`echo $SOURCES | wc -w`
		if [ $n -gt 1 ]; then
		    echo "cc.sh:Error, multiple sources, one object output."
		    exit 1
		else
		    output_filename=$OUTFILE
		fi;;
	    /*)
		o=`echo $x | sed 's,.*/,,' | sed 's,\.c$,.o,'`
		output_filename=`cygpath -m $OUTFILE`
		output_filename="$output_filename/${o}";;
	    *)
		o=`echo $x | sed 's,.*/,,' | sed 's,\.cp*$,.o,'`
		output_filename="./${OUTFILE}/${o}";;
	esac
    else
	o=`echo $x | sed 's,.*/,,' | sed 's,\.c$,.o,'`
	output_filename=$TMPOBJDIR/$o
	ACCUM_OBJECTS="$ACCUM_OBJECTS $output_filename"
    fi
    MPATH=`cygpath -m $x`
    if [ $PREPROCESSING = true ]; then
	output_flag="-E"
    else
	output_flag="-c -Fo`cygpath -m ${output_filename}`"
    fi
    params="$COMMON_CFLAGS $MD $DEBUG_FLAGS $OPTIMIZE_FLAGS \
	    $CMD ${output_flag} $MPATH"
    if [ "X$CC_SH_DEBUG_LOG" != "X" ]; then
	echo cc.sh "$SAVE" >>$CC_SH_DEBUG_LOG
	echo cl.exe $params >>$CC_SH_DEBUG_LOG
    fi
    eval cl.exe $params >$MSG_FILE 2>$ERR_FILE
    RES=$?
    if test $PREPROCESSING = false; then
	cat $ERR_FILE >&2
	tail -n +2 $MSG_FILE
    else
	tail -n +2 $ERR_FILE >&2
	if test $DEPENDENCIES = true; then
	    if test `grep -v $x $MSG_FILE | grep -c '#line'` != "0"; then
		o=`echo $x | sed 's,.*/,,' | sed 's,\.cp*$,.o,'`
		echo -n $o':'
		cat $MSG_FILE | grep '#line' | grep -v $x | awk -F\" '{printf("%s\n",$2)}' | sort -u | grep -v " " | cygpath -f - -m -s | cygpath -f - | awk '{printf("\\\n %s ",$0)}'
		echo
		echo
		after_sed=`date '+%s'`
		echo Made dependencies for $x':' `expr $after_sed '-' $start_time` 's' >&2
	    fi
	else
	    cat $MSG_FILE
	fi
    fi
    rm -f $ERR_FILE $MSG_FILE
    if [ $RES != 0 ]; then
	rm -rf $TMPOBJDIR
	exit $RES
    fi
done
if [ $LINKING = true ]; then
    case $OUTFILE in
	"")
	    first_source=""
	    for x in $SOURCES; do first_source=$x; break; done;
	    if [ -n "$first_source" ]; then
		e=`echo $x | sed 's,.*/,,' | sed 's,\.c$,.exe,'`;
		out_spec="-o $e";
	    else
		out_spec="";
	    fi;;
	*)
	    out_spec="-o $OUTFILE";;
    esac
    case $MD in
	-ML)
	    stdlib="-lLIBC";;
	-MLd)
	    stdlib="-lLIBCD";;
	-MD)
	    stdlib="-lMSVCRT";;
	-MDd)
	    stdlib="-lMSVCRTD";;
	-MT)
	    stdlib="-lLIBCMT";;
	-MTd)
	    stdlib="-lLIBMTD";;
    esac
    params="$out_spec $LINKCMD $stdlib"
    eval ld.sh $ACCUM_OBJECTS $params
    RES=$?
fi
rm -rf $TMPOBJDIR
exit $RES