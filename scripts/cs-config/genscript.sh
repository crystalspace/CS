#! /bin/sh
# This generates the cs-config script

verfile=include/csver.h
vmajor=`sed -e '/#define[ 	][ 	]*CS_VERSION_MAJOR/!d' -e '/#define[ 	][ 	]*CS_VERSION_MAJOR/s/\(#define[ 	][ 	]*CS_VERSION_MAJOR[ 	][ 	]*"\)\(.*\)"\(.*\)/\2/' < ${verfile}`
vminor=`sed -e '/#define[ 	][ 	]*CS_VERSION_MINOR/!d' -e '/#define[ 	][ 	]*CS_VERSION_MINOR/s/\(#define[ 	][ 	]*CS_VERSION_MINOR[ 	][ 	]*"\)\(.*\)"\(.*\)/\2/' < ${verfile}`
rdate=`sed -e '/#define[ 	][ 	]*CS_RELEASE_DATE/!d' -e '/#define[ 	][ 	]*CS_RELEASE_DATE/s/\(#define[ 	][ 	]*CS_RELEASE_DATE[ 	][ 	]*"\)\(.*\)"\(.*\)/\2/' < ${verfile}`

if test -x cs-config; then
	rm cs-config
fi
cat $5/cs-config.temppre	> cs-config
echo "#	WARNING: This script is generated automatically! " >> cs-config
echo				>> cs-config
echo "prefix=\${CRYSTAL-$1}"	>> cs-config
echo "exec_prefix=\${prefix}"	>> cs-config
echo "version='${vmajor}.${vminor} (${rdate})'"		>> cs-config
echo "includedir=\${prefix}/include"	>> cs-config
echo "libdir=\${prefix}/lib"	>> cs-config
echo "syslibs=\"$4\""		>> cs-config
echo "common_cflags=\"$2\""	>> cs-config
echo "common_cxxflags=\"$3\""	>> cs-config
echo				>> cs-config
cat $5/cs-config.temppost	>> cs-config

chmod +x cs-config
