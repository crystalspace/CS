#! /bin/sh
#==============================================================================
# Auto-detect the endianess of the host platform.
#
# IMPORTS
#    LINK
#	Shell or environment variable used to link an executable.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    CS_BIG_ENDIAN
#	Makefile variable emitted to the standard output stream if the current
#	host stores numbers in big-endian format.
#    CS_LITTLE_ENDIAN
#	Makefile variable emitted to the standard output stream if the current
#	host stores numbers in little-endian format.
#==============================================================================

precondition '-n "${LINK}"'

msg_checking "byte order"

cat << EOF > endtest.cpp
int main()
{
  long x = 0x12;
  return *(unsigned char*)&x == 0x12;
}
EOF

${CXX} -o endtest endtest.cpp 2>/dev/null
if [ $? -eq 0 ]; then
  if ./endtest; then
    echo "CS_BIG_ENDIAN = 1"
    msg_result "big-endian"
  else
    echo "CS_LITTLE_ENDIAN = 1"
    msg_result "little-endian"
  fi
else
  msg_result "unknown"
fi

rm -f endtest.cpp endtest.o endtest.obj endtest.exe endtest
