#! /bin/sh
# This generates the cs-config script

if test -x cs-config; then
	rm cs-config
fi
cat $5/cs-config.temppre	> cs-config
echo "#	WARNING: This script is automatic generated! " >> cs-config
echo				>> cs-config
echo "prefix=$1" 		>> cs-config
echo "#tweak if CRYSTAL var is set" >> cs-config
echo "if test -n \"\$CRYSTAL\"; then" >> cs-config
echo "	prefix=\"\$CRYSTAL\""	>> cs-config
echo "fi"			>> cs-config
echo "exec_prefix=\${prefix}"	>> cs-config
echo "version=0.19"		>> cs-config
echo "includedir=\${prefix}/include"	>> cs-config
echo "libdir=\${prefix}/lib"	>> cs-config
echo "syslibs=\"$4\""		>> cs-config
echo "common_cflags=\"$2\""	>> cs-config
echo "common_cxxflags=\"$3\""	>> cs-config
echo				>> cs-config
cat $5/cs-config.temppost	>> cs-config

chmod +x cs-config
