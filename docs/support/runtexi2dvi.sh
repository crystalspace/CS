#!/bin/sh

TEXI2DVI=$1 ; shift
MKDIRS=$1 ; shift
OUTPUT=$1 ; shift
TEXIFILE=$1 ; shift
FLAGS=$@

# Poor man's (portable) 'dirname' command.
dirpart() {
    expr "$1" : '\(..*\)[/\\].*'
}

# copy images
OLDDIR=`pwd`
OUTDIR=`dirpart "$OUTPUT"`
IMAGEDIR=`dirpart "$TEXIFILE"`
cd "$IMAGEDIR"
IMAGEFILES=`find . -name \*.eps -print`
cd "$OLDDIR"
for i in $IMAGEFILES; do
    test -r $OUTDIR/`dirpart $i` || $MKDIRS "$OUTDIR/`dirpart $i`"
    cp "$IMAGEDIR/$i" "$OUTDIR/$i"
done

# run texi2html
$TEXI2DVI $FLAGS --batch --quiet -o "$OUTPUT" "$TEXIFILE"
