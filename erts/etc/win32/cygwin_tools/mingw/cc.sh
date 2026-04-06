#! /bin/sh
SAVE="$@"
COMMON_CFLAGS="-mwindows -D__WIN32__ -DWIN32 -DWINDOWS -D_WIN32 -DNT -DWIN32_MINGW"
MSG_FILE=/tmp/gcc.exe.$$.1
ERR_FILE=/tmp/gcc.exe.$$.2
MD_FORCED=false
PREPROCESSING=false
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
LINKSOURCES=""
DEPENDING=false
if [ -z "$MINGW_EXE_PATH" ]; then
    echo "You have to set MINGW_EXE_PATH to run cc.sh" >&2
    exit 1
fi
while test -n "$1" ; do
    x="$1"
    case "$x" in
	-Wall)
	    ;;
	-c)
	    LINKING=false;;
	-E)
	    PREPROCESSING=true;
	    LINKING=false;;
	-MM|-M)
	    DEPENDING=true;
	    LINKING=false;;
	-O*)
	    OPTIMIZE_FLAGS="$x";
	    DEBUG_FLAGS="-ggdb";
	    DEBUG_BUILD=false;
	    if [ $MD_FORCED = false ]; then
		MD=-MD;
	    fi
	    OPTIMIZED_BUILD=true;;
	-g|-ggdb)
	    if [ $OPTIMIZED_BUILD = false ];then
		DEBUG_FLAGS="-ggdb";
		if [ $MD_FORCED = false ]; then
		    MD=-MDd;
		fi
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
	-l*)
	    y=`echo $x | sed 's,^-l\(.*\),\1,g'`;
	    LINKCMD="$LINKCMD $x";;
	/*.c)
	    SOURCES="$SOURCES $x";;
	*.c)
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
rm -rf $TMPOBJDIR
mkdir $TMPOBJDIR
for x in $SOURCES; do
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
		o=`echo $x | sed 's,.*/,,' | sed 's,\.c$,.o,'`
		output_filename="./${OUTFILE}/${o}";;
	esac
    else
	o=`echo $x | sed 's,.*/,,' | sed 's,\.c$,.o,'`
	output_filename=$TMPOBJDIR/$o
	ACCUM_OBJECTS="$ACCUM_OBJECTS $output_filename"
    fi
    MPATH=`cygpath -m $x`
    if [ $DEPENDING = true ]; then
	output_flag="-MM"
    elif [ $PREPROCESSING = true ]; then
	output_flag="-E"
    else
	output_flag="-c -o `cygpath -m ${output_filename}`"
    fi
    params="$COMMON_CFLAGS $DEBUG_FLAGS $OPTIMIZE_FLAGS \
	    $CMD ${output_flag} $MPATH"
    if [ "X$CC_SH_DEBUG_LOG" != "X" ]; then
	echo cc.sh "$SAVE" >>$CC_SH_DEBUG_LOG
	echo $MINGW_EXE_PATH/gcc $params >>$CC_SH_DEBUG_LOG
    fi
    eval $MINGW_EXE_PATH/gcc $params >$MSG_FILE 2>$ERR_FILE
    RES=$?
    if [ $PREPROCESSING = false -a $DEPENDING = false ]; then
	cat $ERR_FILE >&2
	cat $MSG_FILE
    else
	cat $ERR_FILE >&2
	if [ $DEPENDING = true ]; then
	    cat $MSG_FILE | sed 's|\([a-z]\):/|/cygdrive/\1/|g'
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