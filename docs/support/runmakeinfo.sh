#!/bin/sh

MAKEINFO=$1 ; shift
MKDIRS=$1 ; shift
OUTDIR=$1 ; shift
TEXIFILE=$1 ; shift
FORMAT=$1 ; shift
FLAGS=$@

# copy images
IMAGEDIR=`dirname "$TEXIFILE"`
cd "$IMAGEDIR"
case $FORMAT in
    html)
	IMAGEFILES="`find -name '*.jpg'` `find -name '*.png'`" 
	FLAGS="--html $FLAGS"
	;;
    xml)
	IMAGEFILES="`find -name '*.png'` `find -name '*.jpg'`"
	FLAGS="--xml $FLAGS"
	;;
    docbook)
	IMAGEFILES="`find -name '*.png'` `find -name '*.jpg'`"
	FLAGS="--docbook $FLAGS"
	;;
    info)
	IMAGEFILES="`find -name '*.txt'`"
	;;
    *)
	echo "Unknown Format: $FORMAT"
	exit;;
esac
cd -
for i in $IMAGEFILES; do
    if test ! -e "$OUTDIR/`dirname $i`"; then	
	$MKDIRS "$OUTDIR/`dirname $i`"         
    fi
    cp -p "$IMAGEDIR/$i" "$OUTDIR/$i"
done

TEXIDIR=`dirname "$TEXIFILE"`
TEXIDIR=`cd "$TEXIDIR" ; pwd`
cd "$OUTDIR"
$MAKEINFO -I"$TEXIDIR" $TEXIDIR/`basename "$TEXIFILE"` $FLAGS
cd -

