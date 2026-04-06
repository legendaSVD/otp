#! /bin/bash
if [ -z "$1" ]; then
    if [ -z $ERL_TOP ]; then
	echo "error: $0: No rootdir available"
 	exit 1
    else
	RDIR=$ERL_TOP
    fi
else
    RDIR=$1
fi
DDIR=`msys2win_path.sh $RDIR | sed 's,\\\,\\\\\\\\,g'`
cat > $RDIR/bin/erl.ini <<EOF
[erlang]
Bindir=$DDIR\\\\bin\\\\win32
Progname=erl
Rootdir=$DDIR
EOF