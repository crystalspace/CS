#------------------------------------------------------------------------------
# Orginial Macros Copyright 2003 Eric Sunshine <sunshine@sunshineco.com>
# Determine host platform.  Recognized families: Unix, Windows, MacOS/X.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Determine host CPU.
#
# CS_CHECK_HOST_CPU
#       Set the shell variable cs_host_cpu to a normalized form of the CPU name
#       returned by config.guess/config.sub. Additionally sets the
#	TARGET.PROCESSOR Jamconfig property. Also takes the normalized name,
#       uppercases and appends it to the string "PROC_" to form a name suitable
#       for the C preprocessor.  Assigns this value to the shell variable
#       cs_host_cpu_cpp_define.  Typically, Crystal Space's conception of CPU
#       name is the same as that returned by config.guess/config.sub, but there
#       may be exceptions as seen in the `case' statement.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_HOST_CPU],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_cpu in
        [[Ii][3-9]86*|[Xx]86*]) cs_host_cpu=x86 ;;
        *) cs_host_cpu=$host_cpu ;;
    esac
    cs_host_cpu_normalized="AS_TR_CPP([$cs_host_cpu])"
    CS_JAMCONFIG_PROPERTY([TARGET.PROCESSOR], [$cs_host_cpu_normalized])
    ])

#------------------------------------------------------------------------------
# CS_CHECK_HOST
#       Sets the shell variables cs_host_target, cs_host_makefile, and
#       cs_host_family.  Client code can use these variables to emit
#       appropriate TARGET and TARGET_MAKEFILE makefile variables, and OS_FOO
#       header define.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_HOST],
    [
    CS_CHECK_HOST_CPU
    cs_host_os_normalized=''
    AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
        mingw*|cygwin*)
            cs_host_target=win32gcc
            cs_host_family=windows
            AC_DEFINE([OS_WIN32],,[define when compiling for win32 operating system])
	    AS_IF([test -z "$cs_host_os_normalized"],
		    [cs_host_os_normalized='Win32'])
            ;;
        darwin*)
            _CS_CHECK_HOST_DARWIN
            AC_DEFINE([OS_MACOSX],,[define when compiling for macosx operating system])
            AS_IF([test -z "$cs_host_os_normalized"],
		[cs_host_os_normalized='MacOS/X'])
            ;;
        *)
            # Everything else is assumed to be Unix or Unix-like.
            cs_host_target=unix
            cs_host_family=unix
            AC_DEFINE([OS_UNIX],,[define when compiling for unix operating system])
            AS_IF([test -z "$cs_host_os_normalized"], [cs_host_os_normalized='Unix'])
	    ;;
    esac

    cs_host_os_normalized_uc="AS_TR_CPP([$cs_host_os_normalized])"
    CS_JAMCONFIG_PROPERTY([TARGET.OS], [$cs_host_os_normalized_uc])
])

AC_DEFUN([_CS_CHECK_HOST_DARWIN],
    [AC_REQUIRE([AC_PATH_X])
    AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([AC_PROG_CXX])
    # If user explicitly requested --with-x, then assume Darwin+XFree86; else
    # assume MacOS/X.
    AC_MSG_CHECKING([for --with-x])
    if test "$with_x" = "yes"; then
        AC_MSG_RESULT([yes (assume Darwin)])
        cs_host_target=unix
        cs_host_family=unix
    else
        AC_MSG_RESULT([no (assume MacOS/X)])
        cs_host_target=macosx
        cs_host_family=unix
        AC_CACHE_CHECK([for Objective-C compiler], [cs_cv_prog_objc],
            [cs_cv_prog_objc="$CC -c"])
        CS_JAMCONFIG_PROPERTY([CMD.OBJC], [$cs_cv_prog_objc])
        AC_CACHE_CHECK([for Objective-C++ compiler], [cs_cv_prog_objcxx],
            [cs_cv_prog_objcxx="$CXX -c"])
        CS_JAMCONFIG_PROPERTY([CMD.OBJC++], [$cs_cv_prog_objcxx])
    fi])

