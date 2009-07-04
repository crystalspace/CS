#!/bin/sh

OLD_PWD=$PWD
MY_PATH=$PWD/$0

ARCHIVER="tar --use-compress-program=lzma -cvf"
ARCHIVE_EXT=.tar.lzma

ARCHIVE_NAME=$1
shift

TMPDIR=`mktemp -d`
mkdir -p $TMPDIR/$ARCHIVE_NAME

COPYSCRIPT=$TMPDIR/copy_files
echo "" > $COPYSCRIPT

fullpath()
{
  pushd `dirname $1` > /dev/null
  expanded_path=`pwd`
  popd > /dev/null
  echo "$expanded_path/`basename $1`"
}

addcopyentries()
{
  listfilename=$1
  listsubdir=${2:-.}
  MY_IFS=$IFS
  IFS=$'\n'
  for entry in `cat $listfilename`
  do
    srcfile=`echo "$entry" | sed -e "s/\(.*\):\(.*\)/\\1/"`
    dstfile=$listsubdir/`echo "$entry" | sed -e "s/\(.*\):\(.*\)/\\2/"`
    echo mkdir -p \"`dirname "$dstfile"`\"
    echo cp -pR \"`fullpath "$srcfile"`\" \"$dstfile\"
  done
  IFS=$MY_IFS
}

addtouchentries()
{
  echo "" > touch.tmp
  listfilename=$1
  listsubdir=${2:-.}
  MY_IFS=$IFS
  IFS=$'\n'
  for entry in `cat $listfilename`
  do
    srcfile=`echo "$entry" | sed -e "s/\(.*\):\(.*\)/\\1/"`
    dstfile=$listsubdir/`echo "$entry" | sed -e "s/\(.*\):\(.*\)/\\2/"`
    echo touch -r \"`dirname "$(fullpath "$srcfile")"`\" \"`dirname "$dstfile"`\" >> touch.tmp
  done
  cat touch.tmp | uniq
  rm touch.tmp
  IFS=$MY_IFS
}

for list_arg in "$@"
do
    listname=`echo "$list_arg" | sed -e "s/\([^@]*\)@\?\(.*\)/\\1/g"`
    listsubdir=`echo "$list_arg" | sed -e "s/\([^@]*\)@\?\(.*\)/\\2/g"`
    
    listfile="out/lists/$listname.txt"
    if [ -r "$listfile" ]
    then
      addcopyentries "$listfile" $listsubdir >> $COPYSCRIPT
      addtouchentries "$listfile" $listsubdir >> $COPYSCRIPT
    fi
done

cd $TMPDIR/$ARCHIVE_NAME
source $COPYSCRIPT
cd ..
touch -r $MY_PATH $TMPDIR/$ARCHIVE_NAME

$ARCHIVER $OLD_PWD/$ARCHIVE_NAME$ARCHIVE_EXT $ARCHIVE_NAME

cd $OLD_PWD
rm -rf $TMPDIR
