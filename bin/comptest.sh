#! /bin/sh
#==============================================================================
# Script which locates the C and C++ compilers and then tests their
# characteristics and checks for known bugs.
#
# IMPORTS
#    CPU
#	Optional shell or environment variable giving the host processor type.
#    MACHINE
#	Optional shell or environment variable giving the host machine
#	(hardware) type.
#    checkprog()
#	Shell function which checks if the program mentioned as its sole
#	argument can be found in the PATH.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    CC
#	Shell variable and makefile variable emitted to the standard output
#	stream.  Value is the command used to compile a C program into an
#	object file.
#    CXX
#	Shell variable and makefile variable emitted to the standard output
#	stream.  Value is the command used to compile a C++ program into an
#	object file.
#    LINK
#	Shell variable and makefile variable emitted to the standard output
#	stream.  Value is the command used to link a C++ executable.
#    CFLAGS.SYSTEM
#	Makefile variable emitted to the standard output stream.  If any
#	compiler- or platform-specific optiomizations flags are discovered,
#	they are appended to this variable.
#    CS_USE_FAKE_BOOL_TYPE
#	Makefile variable emitted to the standard otuput stream.  Value is
#	"yes" if the C++ compiler does not have a built-in `bool' type,
#	otherwise the variable is not set.
#    CS_USE_OLD_STYLE_CASTS
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if the C++ compiler does not understand new-style casts, such as
#	"static_cast<Foo>(bar)", otherwise the variable is not set.
#    CS_USE_FAKE_EXPLICIT_KEYWORD
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if the C++ compiler does not recognize the `explicit' keyword ,
#	otherwise the variable is not set.
#    CS_NO_QSQRT
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if the C++ compiler is incapable of compiling the qsqrt()
#	function, otherwise the variable is not set.
#    CS_QINT_WORKAROUND
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if the C++ compiler contains a bug which causes qint() to produce
#	incorrect results, otherwise the variable is not set.
#==============================================================================

#------------------------------------------------------------------------------
# Try to find compiler.
#------------------------------------------------------------------------------

#Find a c++ compiler
msg_checking "for C++ compiler"
if [ -z "${CXX}" ]; then
  [ -z "${CXX}" ] && CXX=`checkprog g++`
  [ -z "${CXX}" ] && CXX=`checkprog gcc`
  [ -z "${CXX}" ] && CXX=`checkprog egcs`
  [ -z "${CXX}" ] && CXX=`checkprog c++`
  [ -z "${CXX}" ] && CXX=`checkprog cc`
  if [ -z "${CXX}" ]; then
    msg_result "no"
    echo "$0: Cannot find an installed C++ compiler!" >&2
    exit 1
  fi
  CXX=`basename ${CXX}`
fi
msg_result "${CXX}"

#Find a C compiler
msg_checking "for C compiler"
if [ -z "${CC}" ]; then
  [ -z "${CC}" ] && CC=`checkprog gcc`
  [ -z "${CC}" ] && CC=`checkprog egcs`
  [ -z "${CC}" ] && CC=`checkprog cc`
  if [ -z "${CC}" ]; then
    msg_result "no"
    echo "$0: Cannot find an installed C compiler!" >&2
    exit 1
  fi
  CC=`basename ${CC}`
fi
msg_result "${CC}"

msg_checking "for linker"
LINK="${CXX}"
msg_result "${LINK}"

echo "CC = ${CC} -c"
echo "CXX = ${CXX} -c"
echo "LINK = ${CXX}"


#-----------------------------------------------------------------------------
# Check for optimization flags.
#-----------------------------------------------------------------------------

echo "int main() { return 0; }" > comptest.cpp

msg_checking "for optimization flags"

if test -n "$ARCH"; then
  ARCH="-march=$ARCH";
fi

# Check for machine-specific C compiler flags (these are mutually exclusive).
(echo "$CPU" | grep -s 686 >/dev/null && ${CXX} -c -mcpu=pentiumpro ${ARCH-"-march=i686"} comptest.cpp >/dev/null 2>&1 && echo "-mcpu=pentiumpro ${ARCH-"-march=i686"}") >comptest.log || \
(echo "$CPU" | grep -s 686 >/dev/null && ${CXX} -c -mpentiumpro ${ARCH-"-march=i686"} comptest.cpp >/dev/null 2>&1 && echo "-mpentiumpro ${ARCH-"-march=i686"}") >comptest.log || \
(echo "$CPU" | grep -s [5-6]86 >/dev/null && ${CXX} -c -mcpu=pentium ${ARCH-"-march=i586"} comptest.cpp >/dev/null 2>&1 && echo "-mcpu=pentium ${ARCH-"-march=i586"}") >comptest.log || \
(echo "$CPU" | grep -s [5-6]86 >/dev/null && ${CXX} -c -mpentium ${ARCH-"-march=i586"} comptest.cpp >/dev/null 2>&1 && echo "-mpentium ${ARCH-"-march=i586"}") >comptest.log || \
(echo "$CPU" | grep -s [3-9]86 >/dev/null && ${CXX} -c -mcpu=i486 comptest.cpp >/dev/null 2>&1 && echo "-mcpu=i486") >comptest.log || \
(echo "$CPU" | grep -s [3-9]86 >/dev/null && ${CXX} -c -m486 comptest.cpp >/dev/null 2>&1 && echo "-m486") >comptest.log || \
(echo "$MACHINE" | grep -s alpha >/dev/null && ${CXX} -c -mieee comptest.cpp >/dev/null 2>&1 && echo "-mieee") >comptest.log

OPTFLAGS=`cat comptest.log`
if [ -n "${OPTFLAGS}" ]; then
  echo "CFLAGS.SYSTEM += ${OPTFLAGS}"
  msg_result "${OPTFLAGS}"
else
  msg_result "no"
fi


#------------------------------------------------------------------------------
# Check if exceptions can be disabled.
#------------------------------------------------------------------------------
msg_checking "how to disable C++ exceptions"
${CXX} -c -fno-exceptions comptest.cpp 2>/dev/null
if [ $? -eq 0 ]; then 
  echo "CFLAGS.SYSTEM += -fno-exceptions"
  msg_result "-fno-exceptions"
else
  msg_result "no"
fi


#------------------------------------------------------------------------------
# Check how to enable compilation warnings.
# Note: On some platforms, it is more appropriate to use -Wmost rather than
# -Wall even if the compiler understands both.
#------------------------------------------------------------------------------
msg_checking "how to enable compilation warnings"
${CXX} -c -Wmost comptest.cpp 2>/dev/null
if [ $? -eq 0 ]; then
  echo "CFLAGS.SYSTEM += -Wmost"
  msg_result "-Wmost"
else
  ${CXX} -c -Wall comptest.cpp 2>/dev/null
  if [ $? -eq 0 ]; then
    echo "CFLAGS.SYSTEM += -Wall"
    msg_result "-Wall"
  else
    msg_result "no"
  fi
fi


#------------------------------------------------------------------------------
# Check if warnings about unknown pragmas can be disabled.  (MSVC and Borland
# use a number of pragmas not understood by GCC, for instance.)
#------------------------------------------------------------------------------
msg_checking "how to disable unknown #pragma warnings"
${CXX} -c -Wno-unknown-pragmas comptest.cpp 2>/dev/null
if [ $? -eq 0 ]; then
  echo "CFLAGS.SYSTEM += -Wno-unknown-pragmas"
  msg_result "-Wno-unknown-pragmas"
else
  msg_result "no"
fi


#------------------------------------------------------------------------------
# Check if C++ compiler has a built-in 'bool' type.
#------------------------------------------------------------------------------
msg_checking "for C++ bool type"
echo "int func() { bool b = true; return (int)b; }" > comptest.cpp
${CXX} -c comptest.cpp 2>/dev/null
if [ $? -eq 0 ]; then
  msg_result "yes"
else
  echo "CS_USE_FAKE_BOOL_TYPE = yes"
  msg_result "no"
fi


#------------------------------------------------------------------------------
# Check if C++ compiler understands new-style C++ casting syntax.
# For example: `static_cast<int>(foo)' vs. `(int)foo'
#
# Specifically check for all four new casting operators since some botched
# compilers have been known to implement only a partial set.  (The OpenStep
# Objective-C++ compiler is one such botched implementation.  It fails to
# recognize reinterpret_cast even though it recognizes the others.)
#------------------------------------------------------------------------------
msg_checking "for C++ new-style casts"
cat << TEST > comptest.cpp
int func1() { long n = 1; return static_cast<int>(n); }
char* func2() { static char const* s = "const"; return const_cast<char*>(s); }
struct A {}; A* func3(A* a) { return dynamic_cast<A*>(a); }
A* func4(void* p) { return reinterpret_cast<A*>(p); }
TEST
${CXX} -c comptest.cpp 2>/dev/null
if [ $? -eq 0 ]; then
  msg_result "yes"
else
  echo "CS_USE_OLD_STYLE_CASTS = yes"
  msg_result "no"
fi


#------------------------------------------------------------------------------
# Check if C++ compiler understands new C++ `explicit' keyword.
#------------------------------------------------------------------------------
msg_checking "for C++ 'explicit' keyword"
echo "class A { public: explicit A(int); };" > comptest.cpp
${CXX} -c comptest.cpp 2>/dev/null
if [ $? -eq 0 ]; then
  msg_result "yes"
else
  echo "CS_USE_FAKE_EXPLICIT_KEYWORD = yes"
  msg_result "no"
fi


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
  msg_checking "if qsqrt() compiles"
  cat << TEST > comptest.cpp
  #define PROC_${PROC}
  #define COMP_GCC
  #include "include/qsqrt.h"
  float func() { float n = 1; return qsqrt(n); }
TEST
  ${CXX} -c comptest.cpp 2>/dev/null
  if [ $? -eq 0 ]; then
    msg_result "yes"
  else
    echo "CS_NO_QSQRT = yes"
    msg_result "no"
  fi
fi


#------------------------------------------------------------------------------
# Attempt to detect a compiler bug discovered in gcc 2.96 (RedHat and other
# unstable versions) and gcc 3.0.1.  This is somewhat more complicated than it
# should be because, when executed, the test program crashes on some platforms
# and prints an uncatchable "Bus error" message to the screen.  Consequently,
# we run the test program in a subshell and capture its output rather than its
# return value in order to test for success.  When run in a subshell in this
# fashion, we are able to catch the otherwise uncatchable termination message.
# Note: This fails for cross-compiling because you can't execute the result.
# -----------------------------------------------------------------------------
msg_checking "if qint() functions correctly"
cat << TEST > comptest.cpp
#include <stdio.h>
static inline long double2int(double val)
{
  long* l;
  val += 68719476736.0;
  l = (long*)((char*)&val + 2);
  return *l;
}
int main()
{
  int const rc = (double2int(255.99) != 255 ? 1 : 0);
  if (rc == 0)
  {
    fputs("qint-okay", stdout);
    fflush(stdout);
  }
  return rc;
}
TEST
${CXX} -O2 comptest.cpp -o comptest 
if [ $? -eq 0 ]; then
  qintout=`./comptest`
  if [ "${qintout}" = "qint-okay" ]; then
    msg_result "yes"
  else
    echo "CS_QINT_WORKAROUND = yes"
    msg_result "no"
  fi
else
  echo "CS_QINT_WORKAROUND = yes"
  msg_result "no"
fi


#------------------------------------------------------------------------------
# Clean up.
#------------------------------------------------------------------------------
rm -f comptest.cpp comptest.o comptest.obj comptest.exe comptest comptest.log

postcondition '-n "${CC}"'
postcondition '-n "${CXX}"'
postcondition '-n "${LINK}"'
