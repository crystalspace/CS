#!/bin/sh

MAKEINFO=$1 ; shift
MKDIRS=$1 ; shift
OUTDIR=$1 ; shift
TEXIFILE=$1 ; shift
FORMAT=$1 ; shift
FLAGS=$@

# Poor man's (portable) 'dirname' command.
dirpart() {
    expr "$1" : '\(..*\)[/\\].*'
}

# copy images
OLDDIR=`pwd`
IMAGEDIR=`dirpart "$TEXIFILE"`
cd "$IMAGEDIR"
case $FORMAT in
    html)
	IMAGEFILES=`find . \( -name \*.jpg -o \*.png \) -print`
	FLAGS="--html $FLAGS"
	;;
    xml)
	IMAGEFILES=`find . \( -name \*.png -o -name \*.jpg \) -print`
	FLAGS="--xml $FLAGS"
	;;
    docbook)
	IMAGEFILES=`find . \( -name \*.png -o -name \*.jpg \) -print`
	FLAGS="--docbook $FLAGS"
	;;
    info)
	IMAGEFILES=`find . -name \*.txt -print`
	;;
    *)
	echo "Unknown Format: $FORMAT"
	exit;;
esac
cd "$OLDDIR"
for i in $IMAGEFILES; do
    test -r "$OUTDIR/`dirpart $i`" || $MKDIRS "$OUTDIR/`dirpart $i`"
    cp "$IMAGEDIR/$i" "$OUTDIR/$i"
done

TEXIDIR=`dirpart "$TEXIFILE"`
TEXIDIR=`cd "$TEXIDIR" ; pwd`
cd "$OUTDIR"
$MAKEINFO -I"$TEXIDIR" $TEXIDIR/`basename "$TEXIFILE"` $FLAGS
cd "$OLDDIR"
