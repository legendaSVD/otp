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
DRDIR=`w32_path.sh -d $RDIR`
DBDIR=`w32_path.sh -d $BDIR`
cat > $RDIR/bin/erl.ini <<EOF
[erlang]
Bindir=$DBDIR
Progname=erl
Rootdir=$DRDIR
EOF