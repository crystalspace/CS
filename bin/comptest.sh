#!/bin/sh
#==============================================================================
# A compiler capability testing script.
#
# Arguments: $1 is the name of the C++ compiler (gcc, c++, etc.)
#
# The output of this script (a makefile fragment) is configuration information
# needed for building Crystal Space.  It is piped to stdout, and errors are
# piped to stderr.
#==============================================================================
CXX=$1

#------------------------------------------------------------------------------
# Check if C++ compiler has a built-in 'bool' type.
#------------------------------------------------------------------------------
echo "int func() { bool b = true; return (int)b; }" > comptest.cpp
${CXX} -c comptest.cpp 2>/dev/null || echo "CS_USE_FAKE_BOOL_TYPE = yes"

#------------------------------------------------------------------------------
# Check if C++ compiler understands new-style C++ casting syntax.
# For example: `static_cast<int>(foo)' vs. `(int)foo'
#------------------------------------------------------------------------------
echo "int func() { long n = 1; return static_cast<int>(n); }" > comptest.cpp
${CXX} -c comptest.cpp 2>/dev/null || echo "CS_USE_OLD_STYLE_CASTS = yes"

#------------------------------------------------------------------------------
# Check if C++ compiler understands new C++ `explicit' keyword.
#------------------------------------------------------------------------------
echo "class A { public: explicit A(int); };" > comptest.cpp
${CXX} -c comptest.cpp 2>/dev/null || echo "CS_USE_FAKE_EXPLICIT_KEYWORD = yes"

#------------------------------------------------------------------------------
# Clean up.
#------------------------------------------------------------------------------
rm -f comptest.cpp comptest.o
exit 0
