#! /bin/sh
if [ "$1" = "-n" ]; then
    SWITCH=$1
    shift
else
    SWITCH=""
fi
cat > hello.c <<EOF
int main(void)
{
    printf("Hello world\n");
    return 0;
}
EOF
cl.exe -MD hello.c  > /dev/null 2>&1
if [ '!' -f hello.exe.manifest ]; then
    DLLNAME=`dumpbin.exe -imports hello.exe | egrep MSVCR.*dll`
    DLLNAME=`echo $DLLNAME`
    if [ -z "$DLLNAME" ]; then
        DLLNAME=`dumpbin.exe -imports hello.exe | egrep VCRUNTIME.*dll`
	DLLNAME=`echo $DLLNAME`
    fi
    if [ '!' -z "$1" ]; then
	FILETOLOOKIN=$1
    else
	FILETOLOOKIN=$DLLNAME
    fi
    cat > helper.c <<EOF
int main(void)
{
  DWORD dummy;
  DWORD versize;
  int i,n;
  unsigned char *versinfo;
  char buff[100];
  char *vs_verinfo;
  unsigned int vs_ver_size;
  WORD *translate;
  unsigned int tr_size;
  if (!(versize = GetFileVersionInfoSize(REQ_MODULE,&dummy))) {
    fprintf(stderr,"No version info size in %s!\n",REQ_MODULE);
    exit(1);
  }
  versinfo=malloc(versize);
  if (!GetFileVersionInfo(REQ_MODULE,dummy,versize,versinfo)) {
    fprintf(stderr,"No version info in %s!\n",REQ_MODULE);
    exit(2);
  }
  if (!VerQueryValue(versinfo,"\\\\VarFileInfo\\\\Translation",&translate,&tr_size)) {
    fprintf(stderr,"No translation info in %s!\n",REQ_MODULE);
    exit(3);
  }
  n = tr_size/(2*sizeof(*translate));
  for(i=0; i < n; ++i) {
    sprintf(buff,"\\\\StringFileInfo\\\\%04x%04x\\\\FileVersion",
	    translate[i*2],translate[i*2+1]);
    if (VerQueryValue(versinfo,buff,&vs_verinfo,&vs_ver_size) && vs_ver_size > 2) {
        if(vs_verinfo[1] == 0) // Wide char (depends on compiler version!!)
            printf("%S\n",(unsigned short *) vs_verinfo);
        else
            printf("%s\n",(char *) vs_verinfo);
        return 0;
    }
  }
  fprintf(stderr,"Failed to find file version of %s\n",REQ_MODULE);
  return 0;
}
EOF
    cl.exe -MD helper.c version.lib > /dev/null 2>&1
    if [ '!' -f helper.exe ]; then
	echo "Failed to build helper program." >&2
	exit 1
    fi
    NAME=$DLLNAME
    VERSION=`./helper.exe`
else
    VERSION=`grep '<assemblyIdentity' hello.exe.manifest | sed 's,.*version=.\([0-9\.]*\).*,\1,g' | grep -v '<'`
    NAME=`grep '<assemblyIdentity' hello.exe.manifest | sed 's,.*name=.[A-Za-z\.]*\([0-9]*\).*,msvcr\1.dll,g' | grep -v '<'`
fi
if [ "$SWITCH" = "-n" ]; then
    ASKEDFOR=$NAME
else
    ASKEDFOR=$VERSION
fi
if [ -z "$ASKEDFOR" ]; then
    exit 1
fi
echo $ASKEDFOR
exit 0