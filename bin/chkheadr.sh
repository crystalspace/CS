#!/bin/sh

# test for socklen_t
cat << EOF > comptest.cpp
#include <unistd.h>"
#include <sys/types.h>
#include <sys/socket.h>
#define BSD_COMP 1
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
int main() { socklen_t x = 0; return (int)x; }
EOF
${CXX} -c comptest.cpp 2>/dev/null || echo "CS_USE_FAKE_SOCKLEN_TYPE = yes"

# clean up
rm -f comptest.cpp comtest.o

