#!/bin/sh

TEXI2DVI=$1 ; shift
MKDIRS=$1 ; shift
OUTPUT=$1 ; shift
TEXIFILE=$1 ; shift
FLAGS=$@

# copy images
OUTDIR=`dirname "$OUTPUT"`
IMAGEDIR=`dirname "$TEXIFILE"`
cd "$IMAGEDIR"
IMAGEFILES=`find -name "*.eps"`
cd -
for i in $IMAGEFILES; do
    if test ! -e $OUTDIR/`dirname $i`; then
	$MKDIRS "$OUTDIR/`dirname $i`"
    fi
    cp -p "$IMAGEDIR/$i" "$OUTDIR/$i"
done

# run texi2html
$TEXI2DVI $FLAGS --batch --quiet -o "$OUTPUT" "$TEXIFILE"
