#!/bin/sh

TEXI2HTML=$1 ; shift
MKDIRS=$1 ; shift
TEXIFILE=$1 ; shift
OUTDIR=$1 ; shift
FLAGS=$@

# copy images
IMAGEDIR=`dirname "$TEXIFILE"`
cd "$IMAGEDIR"
IMAGEFILES=`find -name "*.jpg"` `find -name "*.png"` `find -name "*.gif"`
cd -
for i in $IMAGEFILES; do
    if test ! -e $OUTDIR/`dirname $i`; then
	$MKDIRS "$OUTDIR/`dirname $i`"
    fi
    cp -p "$IMAGEDIR/$i" "$OUTDIR/$i"
done

# run texi2html
$TEXI2HTML $FLAGS -subdir="$OUTDIR" "$TEXIFILE"
