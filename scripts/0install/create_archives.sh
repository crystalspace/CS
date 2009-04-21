#!/bin/sh

csdir=`dirname $0`/../..
csbindir=$csdir/bin

csver_major=`cat $csdir/include/csver.h | grep "^#define *CS_VERSION_NUM_MAJOR" | sed -e "s/[^0-9]*//"`
csver_minor=`cat $csdir/include/csver.h | grep "^#define *CS_VERSION_NUM_MINOR" | sed -e "s/[^0-9]*//"`
csver_build=`cat $csdir/include/csver.h | grep "^#define *CS_VERSION_NUM_BUILD" | sed -e "s/[^0-9]*//"`
csver_svnrev=`svnversion $csdir | sed -e "s/^\([0-9]*\).*$/\\1/"`
CSVER=$csver_major.$csver_minor.$csver_build.$csver_svnrev

$csbindir/archive-from-lists.sh crystalspace-libs-$CSVER libs-shared@lib
# SDKs: we need to lump everything together
# splitting things over multiple feeds works, but is less robust
# (e.g. cs-config won't find the libs dir when invoked w/o 0launch)
SDK_LISTS="libs-static@lib libs-shared@lib cs-config@bin headers@include headers-platform@include"
$csbindir/archive-from-lists.sh crystalspace-sdk-$CSVER $SDK_LISTS
$csbindir/archive-from-lists.sh crystalspace-sdk-staticplugins-$CSVER $SDK_LISTS libs-staticplugins@lib
