#!/bin/sh

TEXI2HTML=$1 ; shift
MKDIRS=$1 ; shift
TEXIFILE=$1 ; shift
OUTDIR=$1 ; shift
FLAGS=$@

# Poor man's (portable) 'dirname' command.
dirpart() {
    expr "$1" : '\(..*\)[/\\].*'
}

# copy images
OLDDIR=`pwd`
IMAGEDIR=`dirpart "$TEXIFILE"`
cd "$IMAGEDIR"
IMAGEFILES=`find . \( -name \*.jpg -o -name \*.png -o -name \*.gif \) -print`
cd "$OLDDIR"
for i in $IMAGEFILES; do
    test -r $OUTDIR/`dirpart $i` || $MKDIRS "$OUTDIR/`dirpart $i`"
    cp "$IMAGEDIR/$i" "$OUTDIR/$i"
done

# run texi2html
# @@@ Find better way to get texi2html home dir
export T2H_HOME="$OLDDIR/$IMAGEDIR/../support"
$TEXI2HTML $FLAGS -subdir="$OUTDIR" $TEXIFILE
