#!/bin/sh
#==============================================================================
# A compiler capability testing script.
#
# Arguments: $1 is the name of the C++ compiler (gcc, c++, etc.)
#            $2 is the symbolic name of the CPU (X86, SPARC, POWERPC, etc.)
#
# The output of this script (a makefile fragment) is configuration information
# needed for building Crystal Space.  It is piped to stdout, and errors are
# piped to stderr.
#==============================================================================
CXX=$1
PROC=$2

#------------------------------------------------------------------------------
# Check if C++ compiler has a built-in 'bool' type.
#------------------------------------------------------------------------------
echo "int func() { bool b = true; return (int)b; }" > comptest.cpp
${CXX} -c comptest.cpp 2>/dev/null || echo "CS_USE_FAKE_BOOL_TYPE = yes"


#------------------------------------------------------------------------------
# Check if C++ compiler understands new-style C++ casting syntax.
# For example: `static_cast<int>(foo)' vs. `(int)foo'
#
# Specifically check for all four new casting operators since some botched
# compilers have been known to implement only a partial set.  (The OpenStep
# Objective-C++ compiler is one such botched implementation.  It fails to
# recognize reinterpret_cast even though it recognizes the others.)
#------------------------------------------------------------------------------
cat << TEST > comptest.cpp
int func1() { long n = 1; return static_cast<int>(n); }
char* func2() { static char const* s = "const"; return const_cast<char*>(s); }
struct A {}; A* func3(A* a) { return dynamic_cast<A*>(a); }
A* func4(void* p) { return reinterpret_cast<A*>(p); }
TEST
${CXX} -c comptest.cpp 2>/dev/null || echo "CS_USE_OLD_STYLE_CASTS = yes"


#------------------------------------------------------------------------------
# Check if C++ compiler understands new C++ `explicit' keyword.
#------------------------------------------------------------------------------
echo "class A { public: explicit A(int); };" > comptest.cpp
${CXX} -c comptest.cpp 2>/dev/null || echo "CS_USE_FAKE_EXPLICIT_KEYWORD = yes"


#------------------------------------------------------------------------------
# If processor type was specified, check if compiler is able to understand
# CS/include/qsqrt.h.  This test should not only catch compilers which do not
# understand the x86 assembly in that file, but should also catch compilers
# which fail with an internal error on this file (such as the RedHat 7 GCC).
# If the processor type was not specified, then no test is made, nor are any
# makefile variables set.  In this case, it is up to the platform-specific
# configuration mechanism to enable the CS_NO_QSQRT flag if necessary.
#------------------------------------------------------------------------------
if [ -n "${PROC}" ]; then
cat << TEST > comptest.cpp
#define PROC_${PROC}
#define COMP_GCC
#include "include/qsqrt.h"
float func() { float n = 1; return qsqrt(n); }
TEST
${CXX} -c comptest.cpp 2>/dev/null || echo "CS_NO_QSQRT = yes"
fi

#------------------------------------------------------------------------------
# The following test tries to detect a compiler bug discovered in gcc 2.96
# (redhat and other unstables...) and gcc 3.0.1
# Note: This fails for crosscompiling because you can't execute the result then
# -----------------------------------------------------------------------------
cat << TEST > comptest.cpp
static inline long double2int(double val)
{
long *l;
val += 68719476736.0;
l = (long*) ((char*)&val + 2);
return *l;
}

int main()
{
if ( double2int(255.99) !=255 )
	return 1;
return 0;
}
TEST
${CXX} -O2 comptest.cpp -o comptest 
./comptest 2>/dev/null || echo "CS_QINT_WORKAROUND = yes"

#------------------------------------------------------------------------------
# Clean up.
#------------------------------------------------------------------------------
rm -f comptest.cpp comptest.o comptest
exit 0
