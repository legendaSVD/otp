#! /bin/bash
if [ -z "$1" ]; then
	echo "error: $0: No rootdir given"
 	exit 1
else
    RDIR=$1
fi
if [ -z "$2" ]; then
	echo "error: $0: No bindir given"
 	exit 1
else
    BDIR=$2
fi
DRDIR=`msys2win_path.sh $RDIR | sed 's,\\\,\\\\\\\\,g'`
DBDIR=`msys2win_path.sh $BDIR | sed 's,\\\,\\\\\\\\,g'`
cat > $RDIR/bin/erl.ini <<EOF
[erlang]
Bindir=$DBDIR
Progname=erl
Rootdir=$DRDIR
EOF