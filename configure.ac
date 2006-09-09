# configure.ac                                                 -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003-2006 by Eric Sunshine <sunshine@sunshineco.com>
#
# Autoconf input script for Crystal Space.  Process this file with
# CS/bin/autogen.sh to produce a configure script.
#==============================================================================
AC_PREREQ([2.56])


#------------------------------------------------------------------------------
# cs_version_number
#	Keep this value synchronized with CS/include/csver.h.
#------------------------------------------------------------------------------
m4_define([cs_version_number], [1.1])



#==============================================================================
# INITIALIZATION
#==============================================================================
AC_INIT([crystalspace], [cs_version_number],
        [crystal-main@lists.sourceforge.net])
CS_PACKAGEINFO([Crystal Space],
    [Copyright (C)1998-2006 Jorrit Tyberghein and others],
    [http://www.crystalspace3d.org/])
AC_CONFIG_SRCDIR([libs/Jamfile])
AC_CONFIG_AUX_DIR([mk/autoconf])
AC_CANONICAL_HOST



#------------------------------------------------------------------------------
# Check for compilers and linker.
#------------------------------------------------------------------------------
CS_PROG_CC
CS_PROG_CXX
CS_PROG_LINK

AC_PROG_CXXCPP
CS_EMIT_BUILD_PROPERTY([CMD.C++CPP], [$CXXCPP])

AS_IF([test $ac_compiler_gnu = yes],
    [cs_compiler_name=GCC CS_HEADER_PROPERTY([CS_COMPILER_GCC])],
    [cs_compiler_name="AS_TR_CPP([$CXX])"])
CS_EMIT_BUILD_PROPERTY([COMPILER.TYPE], [$cs_compiler_name])
CS_HEADER_PROPERTY([CS_COMPILER_NAME],
    [AS_ESCAPE(["])$cs_compiler_name[]AS_ESCAPE(["])])



#------------------------------------------------------------------------------
# Check for common tools.
#------------------------------------------------------------------------------
CS_CHECK_COMMON_TOOLS_LINK
CS_CHECK_COMMON_TOOLS_BASIC
CS_CHECK_COMMON_TOOLS_DOC_TEXINFO
CS_CHECK_COMMON_TOOLS_DOC_DOXYGEN

CS_CHECK_PROGS([FLEX], [flex])
CS_EMIT_BUILD_PROPERTY([CMD.FLEX], [$FLEX])

CS_CHECK_PROGS([BISON], [bison])
AS_IF([test -n "$BISON"],
    [CS_CHECK_PROG_VERSION([bison], [$BISON --version], [1.35], [9.9],
	[CS_EMIT_BUILD_PROPERTY([CMD.BISON], [$BISON])])])

CS_CHECK_TOOLS([HHC], [hhc])
CS_EMIT_BUILD_PROPERTY([CMD.HHC], [$HHC])

CS_CHECK_PROGS([SWIG], [swig])
AS_IF([test -n "$SWIG"],
    [CS_CHECK_PROG_VERSION([swig], [$SWIG -version 2>&1], [1.3.21],
	[9.9|.9], [CS_EMIT_BUILD_PROPERTY([CMD.SWIG], [$SWIG])])])



#------------------------------------------------------------------------------
# In order to avoid alienating users by forcing them to install and use Jam,
# config.status is instructed (see far below) to build Jam on the user's behalf
# from local sources, and to place the executable in the root of the build
# directory.  When checking for Jam, in addition to PATH, we also look in root
# of the build directory since we might already have built Jam on a previous
# configure run.
#
# IMPLEMENTATION NOTES
#
# The Crystal Space build system supports Jam versions starting with 2.4.
# Earlier versions are rejected. Examples of rejected versions include Apple's
# highly modified Jam (based upon 2.2.1), and ftjam (based upon
# 2.3.2). BoostJam reports its version number as 3.1.9 (or greater), though it
# is actually based upon Jam 2.4. Since we accept 2.4, and since 3.1.9 passes
# the "jam -v >= 2.4" assertion, this version anomaly is not a problem.
#
# We also check for 'make', since it is used to build Jam locally, when needed.
#------------------------------------------------------------------------------
m4_define([cs_jam_local_exe], [jam$EXEEXT])

CS_PATH_PROGS([JAM], [jam], [], [$PATH$PATH_SEPARATOR.])
AS_IF([test -n "$JAM"],
    [CS_CHECK_PROG_VERSION([jam], ["$JAM" -v 2>&1], [2.4], [9.9|.9],
	[cs_prog_jam_okay=yes], [cs_prog_jam_okay=no])],
    [cs_prog_jam_okay=no])

AS_IF([test $cs_prog_jam_okay != yes],
    [JAM=./cs_jam_local_exe CS_NEED_LOCAL_JAM=yes])
CS_EMIT_BUILD_PROPERTY([JAM], [$JAM])

CS_CHECK_PROGS([MAKE], [gnumake gmake make])



#------------------------------------------------------------------------------
# The cs-win32libs package is a convenience archive made available to Windows
# users of Crystal Space.  It contains common libraries, headers, and tools
# (such as zlib, libjpeg, cal3d, etc.) usable by MSVC, Mingw/MSYS, and Cygwin
# users.  It saves users the bother of having to install these packages
# manually one at a time.
#------------------------------------------------------------------------------
CS_CHECK_CSWIN32LIBS


#------------------------------------------------------------------------------
# Use the CPPFLAGS, CXXFLAGS, CFLAGS, and LDFLAGS passed to configure, as well
# as any additional flags provided by cs-win32libs.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS], [$CPPFLAGS $CFLAGS], [+])
CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS], [$CPPFLAGS $CXXFLAGS], [+])
CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [$LDFLAGS], [+])



#------------------------------------------------------------------------------
# Check for typical required libraries (libm, libmx, libdl, libnsl).
#------------------------------------------------------------------------------
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([cmath], [CS_HEADER_PROPERTY([CS_HAVE_CMATH_H])])
AC_CHECK_HEADERS([math.h], [CS_HEADER_PROPERTY([CS_HAVE_MATH_H])])
AC_LANG_POP([C++])

CS_CHECK_COMMON_LIBS

AS_IF([test $ac_cv_lib_m_pow = yes || test $ac_cv_lib_m_cosf = yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lm], [+])])
AS_IF([test $ac_cv_lib_mx_cosf = yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lmx], [+])])
AS_IF([test $ac_cv_lib_dl_dlopen = yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-ldl], [+])])
AS_IF([test $ac_cv_lib_nsl_gethostbyname = yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lnsl], [+])])

AS_IF([test $ac_cv_lib_m_cosf != no || test $ac_cv_lib_mx_cosf != no],
    [CS_HEADER_PROPERTY([CS_HAVE_MATH_H_FLOAT_FUNCS])])



#------------------------------------------------------------------------------
# Check for pthread.  Also check if pthread implementation supports the
# recursive mutex extension.
#------------------------------------------------------------------------------
CS_CHECK_PTHREAD([cygwin*])
CS_EMIT_BUILD_RESULT([cs_cv_sys_pthread], [PTHREAD])
AS_IF([test $cs_cv_sys_pthread_mutex_recursive != no],
    [CS_HEADER_PROPERTY([CS_PTHREAD_MUTEX_RECURSIVE],
	[$cs_cv_sys_pthread_mutex_recursive])])
CS_CHECK_PTHREAD_ATFORK([cs_cv_sys_pthread_atfork])
CS_EMIT_BUILD_PROPERTY([PTHREAD.ATFORK.AVAILABLE], [$cs_cv_sys_pthread_atfork])



#------------------------------------------------------------------------------
# Determine whether to link base g++ libraries statically
#------------------------------------------------------------------------------
AC_MSG_CHECKING([whether to link base g++ libraries statically])
AC_ARG_ENABLE([static-baselibs], [AC_HELP_STRING([--enable-static-baselibs],
    [Link statically againts libgcc and libstdc++. Use with caution; enabling 
    this option may cause issues when dynamically linking against other C++
    modules. (default NO)])],
    [], [enable_static_baselibs=no])
AC_MSG_RESULT([$enable_static_baselibs])
AS_IF([test $enable_static_baselibs = yes],
    [CS_EMIT_BUILD_FLAGS([for -static-libgcc],
        [cs_cv_prog_link_static_libgcc], [CS_CREATE_TUPLE([-static-libgcc])],
        [C], [COMPILER.LFLAGS], [+])
    libstdcxx=`$CXX -print-file-name=libstdc++.a`
    CS_EMIT_BUILD_FLAGS([for static libstdc++],
        [cs_cv_prog_link_static_libstdcxx],
        CS_CREATE_TUPLE([$libstdcxx]),
        [C++], [COMPILER.LFLAGS], [+])])



#------------------------------------------------------------------------------
# Check if RTLD_NOW dlopen() flag is available; some installations (OpenBSD,
# for example) provide only RTLD_LAZY.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_RTLD],
    [AC_REQUIRE([CS_CHECK_COMMON_LIBS])
    CS_CHECK_BUILD([for RTLD_NOW], [cs_cv_sys_rtld_now],
	[AC_LANG_PROGRAM([[#include <dlfcn.h>]], [dlopen(0, RTLD_NOW)])],
	[], [], [CS_HEADER_PROPERTY([CS_HAVE_RTLD_NOW])], [], [],
	[], [], [$cs_cv_libdl_libs])])

CS_CHECK_RTLD



#------------------------------------------------------------------------------
# Determine host platform.  Recognized families: Unix, Windows, MacOS/X.
#
# CS_CHECK_HOST
#	Sets the shell variables cs_host_target and cs_host_family.  Client
#	code can use these variables to emit appropriate Jam variables and
#	CS_PLATFORM_FOO header define.
#------------------------------------------------------------------------------
CS_CHECK_HOST
AS_IF([test "x$cs_host_macosx" = "xyes"],
    [# We assume that we always have CoreAudio available on MacOS/X.
    CS_EMIT_BUILD_PROPERTY([COREAUDIO.AVAILABLE], [yes])
    CS_EMIT_BUILD_PROPERTY([COREAUDIO.LFLAGS], [-framework CoreAudio -framework AudioToolbox])
    CS_HEADER_PROPERTY([CS_PLATFORM_MACOSX])],
    [case $cs_host_family in
	windows)
	    CS_HEADER_PROPERTY([CS_PLATFORM_WIN32])
	    ;;
	unix)
	    CS_HEADER_PROPERTY([CS_PLATFORM_UNIX])
    esac])
CS_HEADER_PROPERTY([CS_PLATFORM_NAME],
    [AS_ESCAPE(["])$cs_host_os_normalized[]AS_ESCAPE(["])])



#------------------------------------------------------------------------------
# Check if Perl SDK is installed and which compiler and linker flags are
# required to embed Perl.  Also determine if the glue file can be created with
# ExtUtils::Embed or if it must be created manually.
#------------------------------------------------------------------------------
CS_CHECK_PROGS([PERL], [perl5 perl])
CS_EMIT_BUILD_PROPERTY([PERL5], [$PERL])
CS_EMIT_BUILD_PROPERTY([PERL], [$PERL])
CS_EMIT_BUILD_PROPERTY([CMD.PERL5], [$PERL])

# csperl5 fails to build on many platforms, so default is NO for now.
AC_ARG_WITH([perl], [AC_HELP_STRING([--with-perl],
    [use Perl5 scripting interface (default NO)])])
AS_IF([test -z "$with_perl"], [with_perl=no])

AS_IF([test -n "$PERL" && test "$with_perl" != no],
    [AC_CACHE_CHECK([for perl ExtUtils::Embed module], [cs_cv_perl_mod_embed],
	[AS_IF([AC_RUN_LOG([$PERL -MExtUtils::Embed -e 0 1>&2])],
	    [cs_cv_perl_mod_embed=yes], [cs_cv_perl_mod_embed=no])])

    AS_IF([test $cs_cv_perl_mod_embed = yes],
	[CS_EMIT_BUILD_PROPERTY([PERL5.EXTUTILS.EMBED.AVAILABLE], [yes])
	AC_CACHE_CHECK([for perl DynaLoader module],
	    [cs_cv_perl_mod_dynaloader],
	    [AS_IF([AC_RUN_LOG([$PERL -MDynaLoader -e 0 1>&2])],
		[cs_cv_perl_mod_dynaloader=yes],
		[cs_cv_perl_mod_dynaloader=no])])
	AS_IF([test $cs_cv_perl_mod_dynaloader = yes],
	    [CS_EMIT_BUILD_PROPERTY([PERL5.DYNALOADER.AVAILABLE], [yes])])],
	[# ExtUtils::Embed absent.
	AC_MSG_NOTICE([Recommend you install a recent release of Perl 5.])
	AC_MSG_NOTICE([http://www.perl.org/])
	AC_CACHE_CHECK([for perl Config module], [cs_cv_perl_mod_config],
	    [AS_IF([AC_RUN_LOG([$PERL -MConfig -e 0 1>&2])],
		[cs_cv_perl_mod_config=yes], [cs_cv_perl_mod_config=no])])])

    AS_VAR_SET_IF([cs_cv_perl_cflags], [],
	[AS_IF([test $cs_cv_perl_mod_embed = yes],
	    [cs_cv_perl_cflags=`AC_RUN_LOG(
		[$PERL -MExtUtils::Embed -e ccopts])`
	    cs_cv_perl_lflags=`AC_RUN_LOG(
		[$PERL -MExtUtils::Embed -e ldopts])`
	    cs_cv_perl_ext=`AC_RUN_LOG([$PERL -MConfig -e \
		'$e = $Config{"dlext"} || $Config{"so"} || ""; print "$e"'])`],
	    [AS_IF([test $cs_cv_perl_mod_config = yes],
		[cs_perl_core=`AC_RUN_LOG(
		    [$PERL -MConfig -e 'print $Config{archlib}'])`
		cs_perl_core="${cs_perl_core}/CORE"
		cs_cv_perl_cflags=`AC_RUN_LOG(
		    [$PERL -MConfig -e 'print $Config{ccflags}'])`
		cs_cv_perl_cflags="${cs_cv_perl_cflags} -I${cs_perl_core}"
		cs_cv_perl_lflags=`AC_RUN_LOG([$PERL -MConfig -e \
		    'print $Config{ldflags}, " ", $Config{libs}'])`
		cs_cv_perl_lflags="${cs_cv_perl_lflags} -L${cs_perl_core}"
		cs_cv_perl_ext=`AC_RUN_LOG([$PERL -MConfig -e \
		    '$e = $Config{"dlext"}||$Config{"so"}||""; print "$e"'])`],
		[cs_perl_core=`AC_RUN_LOG([$PERL -V:archlib | \
		    $PERL -e '<STDIN> =~ m/\x27(.*)\x27/; print $1'])`
		cs_perl_core="${cs_perl_core}/CORE"
		cs_cv_perl_cflags=`AC_RUN_LOG([$PERL -V:ccflags | \
		    $PERL -e '<STDIN> =~ m/\x27(.*)\x27/; print $1'])`
		cs_cv_perl_cflags="${cs_cv_perl_cflags} -I${cs_perl_core}"
		cs_cv_perl_lflags=`AC_RUN_LOG([$PERL -V:ldflags | \
		    $PERL -e '<STDIN> =~ m/\x27(.*)\x27/; print $1'])`
		cs_perl_lflags_tmp=`AC_RUN_LOG([$PERL -V:libs | \
		    $PERL -e '<STDIN> =~ m/\x27(.*)\x27/; print $1'])`
		cs_cv_perl_lflags="${cs_cv_perl_lflags} ${cs_perl_lflags_tmp}
		    -L${cs_perl_core}"
		cs_cv_perl_ext=`AC_RUN_LOG([$PERL -V:dlext | \
		    $PERL -e '<STDIN> =~ m/\x27(.*)\x27/; print $1'])`])])
	cs_cv_perl_cflags=CS_PATH_NORMALIZE([$cs_cv_perl_cflags])
	cs_cv_perl_lflags=CS_PATH_NORMALIZE([$cs_cv_perl_lflags])])

    AS_IF([test -n "$cs_cv_perl_cflags$cs_cv_perl_lflags"],
	[CS_CHECK_BUILD([if Perl SDK is usable], [cs_cv_perl],
	    [AC_LANG_PROGRAM(
		[[#include <EXTERN.h>
		#include <perl.h>]],
		[perl_run(0);])],
	    [CS_CREATE_TUPLE([$cs_cv_perl_cflags],[],[$cs_cv_perl_lflags])])
	AS_IF([test $cs_cv_perl = yes],
	    [CS_EMIT_BUILD_RESULT([cs_cv_perl], [PERL5])
	    CS_EMIT_BUILD_PROPERTY([PERL5.MODULE_EXT],[.$cs_cv_perl_ext])])])])
	


#------------------------------------------------------------------------------
# Check if Python SDK is installed and which compiler and linker flags are
# required to embed Python.
#------------------------------------------------------------------------------
CS_CHECK_PYTHON([emit], [], [Python scripting interface])



#------------------------------------------------------------------------------
# Check if Java2 SDK is installed and which compiler and linker flags are
# needed.  Also check if Ant build tool, which is commonly used to build Java
# projects, is available.
#------------------------------------------------------------------------------
AC_DEFUN([CS_JAVA_FLAGS],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    AC_REQUIRE([CS_CHECK_HOST])
    AS_IF([test x$cs_host_macosx = xyes && # Not cross-building for Darwin.
	test -r /System/Library/Frameworks/JavaVM.framework/Headers],
	[$1_cflags="-I/System/Library/Frameworks/JavaVM.framework/Headers"
	$1_libs="-framework JavaVM"],
	[AS_IF([test -n "$JAVA_HOME"],
	    [$1_dir="$JAVA_HOME"],
	    [AS_IF([test -n "$2"],
		[$1_dir=`AS_DIRNAME([$2])`
		$1_dir=`AS_DIRNAME([$$1_dir])`],
		[$1_dir=''])])
	AS_IF([test -n "$$1_dir"],
	    [$1_cflags="-I$$1_dir/include"
	    case $host_os in
		mingw*|cygwin*) cs_java_platform=win32 ;;
		*) cs_java_platform=`echo $host_os |
		    sed 's/^\([[^-]]*\).*$/\1/'` ;;
	    esac
	    AS_IF([test -n "$cs_java_platform"],
		[$1_cflags="$$1_cflags
		    $$1_cflags/$cs_java_platform"])
	    $1_cflags=CS_PATH_NORMALIZE([$$1_cflags])])])])

JAVA_HOME=CS_PATH_NORMALIZE([$JAVA_HOME])

AS_IF([test -z "$JAVA" && test -n "$JAVACMD"],
    [JAVA=CS_PATH_NORMALIZE([$JAVACMD])])
CS_PATH_PROGS([JAVA], [java], [],
    [$JAVA_HOME/bin$PATH_SEPARATOR$JAVA_HOME/jre/bin$PATH_SEPARATOR$PATH])
CS_EMIT_BUILD_PROPERTY([JAVA], [$JAVA])

CS_PATH_PROGS([JAVAC], [javac], [],
    [$JAVA_HOME/bin$PATH_SEPARATOR$JAVA_HOME/jre/bin$PATH_SEPARATOR$PATH])
CS_EMIT_BUILD_PROPERTY([JAVAC], [$JAVAC])

AC_ARG_WITH([java], [AC_HELP_STRING([--with-java],
    [use Java scripting interface (default YES)])])
AS_IF([test -z "$with_java"], [with_java=yes])

AS_IF([test "$with_java" != no && test -n "$JAVAC" && test -n "$JAVA"],
    [CS_JAVA_FLAGS([cs_java], [$JAVAC])
    CS_CHECK_BUILD([if Java2 SDK is usable], [cs_cv_java],
	[AC_LANG_PROGRAM(
	    [[
	    #if defined(__GNUC__)
	        typedef long long __int64; /*For gcc on Windows */
	    #endif

	    /* fix for 'jlong' on x86_64 */
	    #if defined(__x86_64)
	    #ifdef _LP64 /* 64-bit Solaris */
	    #undef _LP64
	    #endif
	    #endif

	    #include <jni.h>

	    JNIEXPORT void JNICALL f(JNIEnv *env, jobject obj);]])],
	[CS_CREATE_TUPLE() \
        CS_CREATE_TUPLE([$cs_java_cflags],[$cs_java_lflags],[$cs_java_libs])],
	[], [CS_EMIT_BUILD_RESULT([cs_cv_java], [JAVA])])])

CS_PATH_PROGS([ANT], [ant], [], [$PATH$PATH_SEPARATOR$ANT_HOME/bin])
CS_EMIT_BUILD_PROPERTY([ANT], [$ANT])



#------------------------------------------------------------------------------
# Check if STL is available. Although STL is not used heavily in Crystal Space,
# a few plugin modules take advantage of it, so they need to know if it is
# available.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([for STL], [cs_cv_libstl],
    [AC_LANG_PROGRAM(
	[[#include <map>
	#include <string>]],
	[[std::map<std::string,int> m; m.begin();]])],
    [], [C++], [CS_EMIT_BUILD_RESULT([cs_cv_libstl], [STL])])



#------------------------------------------------------------------------------
# Check how to enable and disable compilation warnings, and how to promote
# diagnostics from warning to error status.
#------------------------------------------------------------------------------
CS_COMPILER_WARNINGS([C++], [cs_cv_prog_cxx_enable_warnings],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS],
	[$cs_cv_prog_cxx_enable_warnings], [+])])

CS_COMPILER_ERRORS([C++], [cs_cv_prog_cxx_enable_errors])

CS_COMPILER_IGNORE_UNUSED([C++], [cs_cv_prog_cxx_ignore_unused],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS.WARNING.NO_UNUSED],
	[$cs_cv_prog_cxx_ignore_unused])])

CS_COMPILER_IGNORE_UNINITIALIZED([C++], [cs_cv_prog_cxx_ignore_uninitialized],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS.WARNING.NO_UNINITIALIZED],
	[$cs_cv_prog_cxx_ignore_uninitialized])])

CS_COMPILER_IGNORE_PRAGMAS([C++], [cs_cv_prog_cxx_ignore_unknown_pragmas],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS],
	[$cs_cv_prog_cxx_ignore_unknown_pragmas], [+])])

# On MacOS/X, use of `long double' in csutil/formatter.h causes complaints.
CS_COMPILER_IGNORE_LONG_DOUBLE([C++], [cs_cv_prog_cxx_ignore_long_double],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS.MANDATORY],
	[$cs_cv_prog_cxx_ignore_long_double], [+])])



#------------------------------------------------------------------------------
# Check for option to disable strict-aliasing (enabled by gcc on higher 
# optimization levels). This is needed for some code we don't have
# control over (e.g. SWIG-generated code) that breaks strict-aliasing rules.
#------------------------------------------------------------------------------
CS_CHECK_BUILD_FLAGS([for flag to disable string-aliasing],
    [cs_cv_prog_cxx_no_strict_aliasing],
    [CS_CREATE_TUPLE([-fno-strict-aliasing])], [C++])

AS_IF([test $cs_cv_prog_cxx_no_strict_aliasing != no],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS.STRICTALIASING.DISABLE],
	[$cs_cv_prog_cxx_no_strict_aliasing], [+])])



#------------------------------------------------------------------------------
# Check if C++ exceptions can be enabled and disabled.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to enable C++ exceptions],
    [cs_cv_prog_cxx_enable_exceptions],
    [CS_CREATE_TUPLE([-fexceptions -fno-omit-frame-pointer]) \
    CS_CREATE_TUPLE([-fexceptions])],
    [C++], [COMPILER.C++FLAGS.EXCEPTIONS.ENABLE])

CS_EMIT_BUILD_FLAGS([how to disable C++ exceptions],
    [cs_cv_prog_cxx_disable_exceptions], [CS_CREATE_TUPLE([-fno-exceptions])],
    [C++], [COMPILER.C++FLAGS.EXCEPTIONS.DISABLE], [],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS],
	[$cs_cv_prog_cxx_disable_exceptions], [+])])



#------------------------------------------------------------------------------
# Check how to make compiler comply strongly with the official C++ language
# specification. Crystal Space itself can not abide 100% by the standard (for
# instance, we employ `long long', which is not part of the present
# specification), however, some external projects do so (by using gcc's -ansi
# and -pedantic options, for instance). Because such projects may utilize
# Crystal Space, we must ensure that our public headers likewise comply with
# the official specification.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to enforce C++ standards conformance],
    [cs_cv_prog_cxx_enable_pedantic], [CS_CREATE_TUPLE([-ansi -pedantic])],
    [C++], [COMPILER.C++FLAGS.PEDANTIC.ENABLE])



#------------------------------------------------------------------------------
# Check if the linker recognizes "-multiply_defined suppress".  This is
# required for MacOS/X 10.1 to avoid warning messages when linking a program
# with "-framework Foundation" if that program does not actually employ any
# Objective-C. Note that we first attempt the uglier equivalent
# "-Wl,-multiply_defined,suppress" in order to make life simpler for external
# projects using cs-config (which reports this linker option in response to
# --libs) in conjunction with GNU libtool. The problem with "-multiply_defined
# suppress" for GNU libtool users is that libtool re-orders linker arguments,
# not realizing that "suppress" must follow immediately after
# "-multiply_defined". The uglier equivalent does not suffer from this problem
# in the hands of libtool since it contains no whitespace.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([if -multiply_defined suppress is needed],
    [cs_cv_prog_link_suppress_multiple],
    [CS_CREATE_TUPLE([], [-Wl,-multiply_defined,suppress]) \
    CS_CREATE_TUPLE([], [-multiply_defined suppress])],
    [], [COMPILER.LFLAGS], [+])



#------------------------------------------------------------------------------
# Check if the linker recognizes "-force_cpusubtype_ALL".  This is required on
# MacOS/X since the Crystal Space's PowerPC implementation of csQsqrt() uses
# the `frsqrte' instruction which is only valid when the -force_cpusubtype_ALL
# flag is present.  This flag is marked "mandatory" because it is required even
# for external projects if they use csQsqrt().
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_FORCE_CPUSUBTYPE_ALL],
    [CS_CHECK_BUILD_FLAGS([if -force_cpusubtype_ALL is needed],
	[cs_cv_prog_cxx_force_cpusubtype_all],
	[CS_CREATE_TUPLE([], [-force_cpusubtype_ALL])])])
    
CS_CHECK_FORCE_CPUSUBTYPE_ALL
CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS.MANDATORY],
    [$cs_cv_prog_cxx_force_cpusubtype_all], [+])



#------------------------------------------------------------------------------
# Check if the MacOS/X compiler recognizes "-fno-common".  This flag is needed
# when building plugin modules (and the libraries which are linked into
# plugins) prior to Apple's introduction of the two-level namespace symbol
# scheme in order to ensure that each plugin module gets its own copy of
# uninitialized global variables.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_NO_COMMON],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
	darwin*)
	    CS_EMIT_BUILD_FLAGS([if -fno-common is needed],
		[cs_cv_prog_cxx_no_common],
		[CS_CREATE_TUPLE([], [-fno-common])], [C++],
		[COMPILER.CFLAGS.MANDATORY], [+])
	    ;;
    esac])

CS_CHECK_NO_COMMON



#------------------------------------------------------------------------------
# Check if Mingw and Cygwin compilers recognize -fvtable-thunks.  As of gcc
# 3.x, this option is no longer supported.  Unfortunately, rather than
# returning an error code (as it does with other unrecognized options), the
# compiler merely prints a warning message when -fvtable-thunks is used, thus
# we use -Werror to turn the warning into an error.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_VTABLE_THUNKS],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
	mingw*|cygwin*)
	    CS_CHECK_BUILD_FLAGS([if -fvtable-thunks is needed],
		[cs_cv_prog_cxx_vtable_thunks],
		[CS_CREATE_TUPLE([-fvtable-thunks])], [C++],
		[CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS.MANDATORY],
		    [$cs_cv_prog_cxx_vtable_thunks], [+])],
		[], [$cs_cv_prog_cxx_enable_errors])
	    ;;
    esac])

CS_CHECK_VTABLE_THUNKS



#------------------------------------------------------------------------------
# Check how to ask the linker to export all dynamic symbols in debug mode to
# allow the backtrace() callstack creation code to obtain symbols in all cases.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to export all dynamic symbols],
    [cs_cv_prog_link_export_dynamic], [CS_CREATE_TUPLE([-Wl,-E])],
    [C++], [COMPILER.LFLAGS.debug], [+])



#------------------------------------------------------------------------------
# Check whether the linker needs -Bsymbolic. This binds "references to global 
# symbols to the definition within the shared library" (from man ld). If this
# flag is not set, it can happen that symbols in plugins get bound against
# symbols from other modules (including the application executable itself),
# causing side-effects such as violating the "memory allocation and freeing in
# the same module" rule.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([for -Bsymbolic linker flag],
    [cs_cv_prog_link_Bsymbolic], [CS_CREATE_TUPLE([-Wl,-Bsymbolic])],
    [C++], [COMPILER.LFLAGS], [+])



#------------------------------------------------------------------------------
# Check if linker recognizes --kill-at which is needed to make csjava.dll
# usable by JNI on Windows.  Without this, JNI is unable to find the exported
# JNI glue functions in csjava.dll.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([if --kill-at is accepted], [cs_cv_prog_link_kill_at], [],
    [CS_CREATE_TUPLE([],[-Wl,--kill-at])], [C++],
    [CS_EMIT_BUILD_PROPERTY([CSJAVA.LFLAGS], [-Wl,--kill-at])])



#------------------------------------------------------------------------------
# Check if linker recognizes "-z defs" which is useful on some platforms that
# allow undefined symbols at link time. This can detect possible build issues
# (e.g. not linking against some library, a missing function) that could cause
# problems on stricter platforms.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([if reporting missing symbols is supported], 
    [cs_cv_prog_link_z_defs], [CS_CREATE_TUPLE([],[-Wl,-z,defs])], 
    [C++], [COMPILER.LFLAGS], [+])
CS_EMIT_BUILD_FLAGS([if missing symbols can be reported as warnings], 
    [cs_cv_prog_link_warn_unresolved_symbols], 
    [CS_CREATE_TUPLE([],[-Wl,--warn-unresolved-symbols])], 
    [C++], [COMPILER.LFLAGS], [+])



#------------------------------------------------------------------------------
# Check if C++ function inlining can be enabled and disabled. Apparently, with
# some versions of GNU build tools on Windows, the Crystal Space Python plugin
# fails to link and emits the error message "Error: 0-bit reloc in dll". In
# particular, this occurs with optimizations (such as -O3) enabled. We can
# avoid the error by disabling function inlining.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to disable function inlining],
    [cs_cv_prog_cxx_disable_inlining],
    [CS_CREATE_TUPLE([-fno-inline-functions])],
    [C++], [COMPILER.C++FLAGS.INLINING.DISABLE])



#------------------------------------------------------------------------------
# Check if "hidden" visibilty for inline functions is supported.
#
# IMPLEMENTATION NOTES
#
# There is an obscure bug in gcc 3.4.x and 4.0.x where
# -fvisibility-inlines-hidden on AMD64 in combination with -fPIC and -shared
# flags causes the linker to fail with a bogus error stating that the target
# library needs to be built with -fPIC even when it has been built using that
# option. On such installations, if Crystal Space is built with
# -fvisibility-inlines-hidden, then clients liking against Crystal Space
# experience this bogus error.  Normally, we would test for this anomaly by
# manipulating a std::basic_string<> in a test program, and avoid
# -fvisibility-inlines-hidden if the test program fails to link, however,
# practical experience has shown that this test is not 100% reliable for all
# such installations. Instead, for simplicity, we disable this flag presently
# for all versions of gcc on AMD64.  References:
#
#     http://dev.gentoo.org/~kugelfang/pappy-visibility.txt
#     http://www.gnu.org/software/gcc/gcc-4.0/changes.html#visibility
#     http://www.nedprod.com/programs/gccvisibility.html
#     http://bugs.gentoo.org/show_bug.cgi?id=78720
#
# Furthermore, there seems to be a bug on MacOS/X with gcc 4.0.x where use of
# -fvisibility-inlines-hidden causes link problems with external clients which
# do not specify this flag.  For this reason, we also disable this flag on
# MacOS/X.  In particular, clients receive errors of this sort:
#
#    /usr/bin/ld: libcrystalspace.a(scf.o) malformed object, illegal
#        reference for -dynamic code (reference to a coalesced section
#        (__TEXT,__textcoal_nt) from section (__TEXT,__text) relocation
#        entry (1))
#------------------------------------------------------------------------------
CS_CHECK_BUILD_FLAGS([for inline visibility flag],
    [cs_cv_prog_cxx_visibility_inlines_hidden],
    [CS_CREATE_TUPLE([-fvisibility-inlines-hidden])],[C++])

AS_IF([test -n "$cs_cv_prog_cxx_visibility_inlines_hidden"],
    [AC_CACHE_CHECK([if $cs_cv_prog_cxx_visibility_inlines_hidden is buggy],
	[cs_cv_prog_cxx_visibility_inlines_hidden_buggy],
	[AS_IF([test $ac_compiler_gnu = yes],
	    [AS_IF([test $host_cpu = x86_64 || test x$cs_host_macosx = xyes],
		[cs_cv_prog_cxx_visibility_inlines_hidden_buggy=yes],
		[cs_cv_prog_cxx_visibility_inlines_hidden_buggy=no])],
	    [cs_cv_prog_cxx_visibility_inlines_hidden_buggy=no])])],
    [cs_cv_prog_cxx_visibility_inlines_hidden_buggy=no])

AS_IF([test $cs_cv_prog_cxx_visibility_inlines_hidden_buggy != yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS],
	[$cs_cv_prog_cxx_visibility_inlines_hidden], [+])])



#------------------------------------------------------------------------------
# Check how to set default symbol visibility, and how to override the setting
# in code.
#------------------------------------------------------------------------------
CS_CHECK_BUILD_FLAGS([for "hidden" symbol visibility flag],
    [cs_cv_prog_cc_visibility_hidden],
    [CS_CREATE_TUPLE([-fvisibility=hidden])])
AS_IF([test -n "$cs_cv_prog_cc_visibility_hidden"],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS], 
	[$cs_cv_prog_cc_visibility_hidden], [+])])
	
CS_CHECK_BUILD_FLAGS([for "default" symbol visibility flag],
    [cs_cv_prog_cc_visibility_default],
    [CS_CREATE_TUPLE([-fvisibility=default])])
AS_IF([test -n "$cs_cv_prog_cc_visibility_default"],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS.VISIBILITY_DEFAULT], 
	[$cs_cv_prog_cc_visibility_default])])

CS_SYMBOL_QUALIFIER([how to declare default visibility],
    [cs_cv_prog_cxx_declare_visible_def],
    [__attribute__((visibility("default")))], [], [C++])
AS_IF([test $cs_cv_prog_cxx_declare_visible_def != no],
    [CS_HEADER_PROPERTY([CS_VISIBILITY_DEFAULT],
	[$cs_cv_prog_cxx_declare_visible_def])])

CS_SYMBOL_QUALIFIER([how to declare hidden visibility],
    [cs_cv_prog_cxx_declare_visible_hidden],
    [__attribute__((visibility("hidden")))], [], [C++])
AS_IF([test $cs_cv_prog_cxx_declare_visible_hidden != no],
    [CS_HEADER_PROPERTY([CS_VISIBILITY_HIDDEN],
	[$cs_cv_prog_cxx_declare_visible_hidden])])



#------------------------------------------------------------------------------
# Check for compiler- and CPU-specific code generation flags.  These flags are
# mutually exclusive.  Exclusivity is enforced by the CS_CHECK_CODE_GEN_FLAG()
# macro.  Note that use of these flags typically means that the resulting
# binary will not be backward compatible.  For instance, if Pentium-specific
# instructions are used, the program will not work with earlier processors,
# such as the Intel 486.  Because these options are potentially dangerous,
# their use it controlled by the user via --enable-cpu-specific-optimizations
# which accepts "no" (same as "disable"), "minimum" (the default), and
# "maximum".  The "minimum" option selects optimizations which are likely to be
# supported by reasonably modern hardware, thus making the resulting
# executables fairly portable.
#
# CS_CHECK_CODE_GEN_FLAG(CPU-PATTERN, FLAGS, [LANGUAGE])
#	If compiler- and CPU-specific optimization flags have not already been
#	determined, and if the canonical host CPU matches CPU-PATTERN, try
#	using the provided compiler FLAGS with the compiler represented by
#	LANGUAGE (typically either "C" or "C++").  If language is omitted, C++
#	is used.  If the compiler accepts the flags, assign them to the shell
#	variable cs_cv_prog_cxx_code_gen.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_CODE_GEN_FLAG],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    AS_IF([test "$cs_cv_prog_cxx_code_gen" = no],
	[case $host_cpu in
	    $1) CS_BUILD_IFELSE([],
		    [CS_CREATE_TUPLE([$2])], m4_default([$3],[C++]),
		    [cs_cv_prog_cxx_code_gen=$cs_build_cflags], [])
		;;
	esac])])

AC_MSG_CHECKING([whether to use CPU-specific optimizations])
AC_ARG_ENABLE([cpu-specific-optimizations],
    [AC_HELP_STRING([--enable-cpu-specific-optimizations=level],
	[enable CPU-specific optimizations; recognized levels are `no',
	`minimum' or `min', `maximum' or `max' (default MINIMUM); the `minimum'
	option selects optimizations which are likely to be supported by
	reasonably modern hardware, thus making the resulting executables
	fairly portable; use the `maximum' option with caution since code for
	one processor will not work with earlier processors (for example, Intel
	686-specific code will not work with a 586)])],
    [case $enable_cpu_specific_optimizations in
	yes|min) enable_cpu_specific_optimizations=minimum ;;
	max) enable_cpu_specific_optimizations=maximum ;;
    esac],
    [enable_cpu_specific_optimizations=minimum])
AC_MSG_RESULT([$enable_cpu_specific_optimizations])

case $enable_cpu_specific_optimizations in
    no|minimum|maximum) ;;
    *) CS_MSG_ERROR([unrecognized option; use `no', `minimum', or `maximum'])
       ;;
esac

AS_IF([test $enable_cpu_specific_optimizations != no],
    [AC_CACHE_CHECK([for code generation flags], [cs_cv_prog_cxx_code_gen],
	[cs_cv_prog_cxx_code_gen=no
	AS_IF([test $enable_cpu_specific_optimizations = maximum],
	    [CS_CHECK_CODE_GEN_FLAG([athlon], [-march=athlon])
	    CS_CHECK_CODE_GEN_FLAG([athlon], [-mcpu=athlon -march=athlon])
	    CS_CHECK_CODE_GEN_FLAG([athlon], [-mcpu=athlon])
	    CS_CHECK_CODE_GEN_FLAG([*686], [-march=i686])
	    CS_CHECK_CODE_GEN_FLAG([*686], [-mcpu=pentiumpro -march=i686])
	    CS_CHECK_CODE_GEN_FLAG([*686], [-mcpu=pentiumpro])
	    CS_CHECK_CODE_GEN_FLAG([*686], [-mpentiumpro -march=i686])
	    CS_CHECK_CODE_GEN_FLAG([*686], [-mpentiumpro])])
	AS_IF([test $host_vendor = apple],
	    [# Work around issue on intel macs, where "-fpmath=sse" is on by
	     # default, but this breaks compilation when combined with 
	     # the "-march=i586" selected below. Furthermore, -march=i686 was
	     # reported to not work, but -march=prescott so, so try to use 
	     # that as the "minimum" optimization on x86 Apples, followed by
	     # -march=i686.
	    CS_CHECK_CODE_GEN_FLAG([[*[3-9]86]], [-march=prescott])
	    CS_CHECK_CODE_GEN_FLAG([[*[3-9]86]], [-march=i686])])
	CS_CHECK_CODE_GEN_FLAG([x86_64], [-march=k8])
	CS_CHECK_CODE_GEN_FLAG([x86_64], [-mtune=k8])
	CS_CHECK_CODE_GEN_FLAG([[*[5-6]86]], [-march=i586])
	CS_CHECK_CODE_GEN_FLAG([[*[5-6]86]], [-mcpu=pentium -march=i586])
	CS_CHECK_CODE_GEN_FLAG([[*[5-6]86]], [-mcpu=pentium])
	CS_CHECK_CODE_GEN_FLAG([[*[5-6]86]], [-mpentium -march=i586])
	CS_CHECK_CODE_GEN_FLAG([[*[5-6]86]], [-mpentium])
	CS_CHECK_CODE_GEN_FLAG([[*[3-9]86]], [-march=i486])
	CS_CHECK_CODE_GEN_FLAG([[*[3-9]86]], [-mcpu=i486 -march=i486])
	CS_CHECK_CODE_GEN_FLAG([[*[3-9]86]], [-mcpu=i486])
	CS_CHECK_CODE_GEN_FLAG([[*[3-9]86]], [-m486])
	CS_CHECK_CODE_GEN_FLAG([alpha], [-mieee])])

    AS_IF([test "$cs_cv_prog_cxx_code_gen" != no],
	[CS_EMIT_BUILD_PROPERTY(
	    [COMPILER.CFLAGS], [$cs_cv_prog_cxx_code_gen], [+])])])



#------------------------------------------------------------------------------
# Check if byteswap.h is available, which contains optimized byte swapping
# functiosn (used for endian conversion).
#------------------------------------------------------------------------------
AC_CHECK_HEADERS([byteswap.h],
    [CS_HEADER_PROPERTY([CS_HAVE_BYTESWAP_H])])



#------------------------------------------------------------------------------
# Check for compiler optimization flags.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_OPTIMIZE_FLAGS],
    [AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([AC_PROG_CXX])

    cs_optimize_flags=''

    CS_EMIT_BUILD_FLAGS([how to enable optimizations],
	[cs_cv_prog_cxx_optimize],
	[CS_CREATE_TUPLE([-O3]) CS_CREATE_TUPLE([-O2]) CS_CREATE_TUPLE([-O])],
	[C++], [COMPILER.CFLAGS.optimize], [+],
	[cs_optimize_flags="$cs_optimize_flags $cs_cv_prog_cxx_optimize"])

    CS_EMIT_BUILD_FLAGS([if -fomit-frame-pointer is accepted],
	[cs_cv_prog_cxx_omit_frame_pointer],
	[CS_CREATE_TUPLE([-fomit-frame-pointer])],
	[C++], [COMPILER.CFLAGS.optimize], [+],
	[cs_optimize_flags="$cs_optimize_flags \
	    $cs_cv_prog_cxx_omit_frame_pointer"])

    CS_EMIT_BUILD_FLAGS([if -ffast-math is accepted],
	[cs_cv_prog_cxx_fast_math],
	[CS_CREATE_TUPLE([-ffast-math])], [C++],
	[COMPILER.CFLAGS.optimize], [+],
	[cs_optimize_flags="$cs_optimize_flags $cs_cv_prog_cxx_fast_math"])])

CS_CHECK_OPTIMIZE_FLAGS



#------------------------------------------------------------------------------
# Check for compiler debug flags.
#------------------------------------------------------------------------------
CS_CHECK_BINUTILS_2_17

AC_MSG_CHECKING([whether to split debug information])
AC_ARG_ENABLE([separate-debug-info], 
    [AC_HELP_STRING([--enable-separate-debug-info],
        [split debug information into separate files. (default YES if 
        binutils >= 2.17 detected and not on Cygwin, otherwise default NO)])])
AS_IF([test -z "$enable_separate_debug_info"], 
    AS_IF([test -z `uname | grep -i CYGWIN`],
        [enable_separate_debug_info=$cs_cv_binutils_2_17],
        [enable_separate_debug_info=no]))
AC_MSG_RESULT([$enable_separate_debug_info])

CS_EMIT_BUILD_PROPERTY([LINK.DEBUG.INFO.SEPARATE], [$enable_separate_debug_info])

CS_EMIT_BUILD_FLAGS([how to enable debug mode debugging symbols],
    [cs_cv_prog_cxx_debug_symbols_extra],
    [CS_CREATE_TUPLE([-g3]) CS_CREATE_TUPLE([-g2]) CS_CREATE_TUPLE([-g])],
    [C++], [COMPILER.CFLAGS.debug], [+],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS.debug],
	[$cs_cv_prog_cxx_debug_symbols_extra], [+])])
	
AC_MSG_CHECKING([whether to enable debug information in optimize mode])
AC_ARG_ENABLE([optimize-mode-debug-info], 
    [AC_HELP_STRING([--enable-optimize-mode-debug-info],
        [whether to enable debug information even in optimize builds. 
        (default YES if separate debug information is enabled, 
        otherwise default NO)])],
    [enable_optimize_mode_debug_info=$enableval])
AS_IF([test -z "$enable_optimize_mode_debug_info"], 
    [enable_optimize_mode_debug_info=$enable_separate_debug_info])
AC_MSG_RESULT([$enable_optimize_mode_debug_info])
AS_IF([test "$enable_optimize_mode_debug_info" != "no"],
    [CS_EMIT_BUILD_FLAGS([how to enable optimize mode debugging symbols],
	[cs_cv_prog_cxx_debug_symbols_normal],
	[CS_CREATE_TUPLE([-g2]) CS_CREATE_TUPLE([-g])],
	[C++], [COMPILER.CFLAGS.optimize], [+],
	[CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS.optimize],
	    [$cs_cv_prog_cxx_debug_symbols_normal], [+])])])



#------------------------------------------------------------------------------
# Check if compiler complains about adding /usr/local/include to the header
# search path.  If not, add it to the header search path since not all
# compilers search there by default.  Some compiler, on the other hand,
# complain if it is already in the built-in search path, and we would like to
# avoid the complaint, if possible.
#
# Furthermore, add a -L entry to LDFLAGS for /usr/local/lib if present.
# Finally, add /usr/local/lib to LD_LIBRARY_PATH if present since not all
# platforms do so by default. This will allow some tests to succeed which would
# otherwise fail. An example is the sizeof(wchar_t) check on Solaris, which
# attempts to run a test program. Without augmenting LD_LIBRARY_PATH, the test
# program would fail to run, citing inability to locate libstdc++.so.
#------------------------------------------------------------------------------
AS_IF([test -d /usr/local/include],
    [CS_CHECK_BUILD_FLAGS([if -I/usr/local/include is usable],
	[cs_cv_prog_cpp_local_include],
	[CS_CREATE_TUPLE([-I/usr/local/include])], [C++], [], [],
	[$cs_cv_prog_cxx_enable_warnings $cs_cv_prog_cxx_enable_errors])],
    [cs_cv_prog_cpp_local_include=''])

AS_IF([test -n "$cs_cv_prog_cpp_local_include"],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS],
	[$cs_cv_prog_cpp_local_include], [+])
    CPPFLAGS="$CPPFLAGS $cs_cv_prog_cpp_local_include"])

AS_IF([test -d /usr/local/lib],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-L/usr/local/lib], [+])
    LDFLAGS="$LDFLAGS -L/usr/local/lib"
    LD_LIBRARY_PATH="$LD_LIBRARY_PATH$PATH_SEPARATOR/usr/local/lib"
    export LD_LIBRARY_PATH])



#------------------------------------------------------------------------------
# Check if va_copy() or __va_copy() is available. 
#------------------------------------------------------------------------------
# CS_CHECK_VA_COPY(FUNCTION)
AC_DEFUN([CS_CHECK_VA_COPY],
    [CS_CHECK_BUILD([for $1], [cs_cv_func_$1],
	[AC_LANG_PROGRAM(
	    [[#include <stdio.h>
	    #include <stdarg.h>]],
	    [va_list ap1; va_list ap2; $1(ap1,ap2)])],
	[], [], [CS_HEADER_PROPERTY(AS_TR_CPP([CS_HAVE_$1]))], [], [],
	[$cs_cv_prog_cxx_enable_pedantic])])

CS_CHECK_VA_COPY([va_copy])
CS_CHECK_VA_COPY([__va_copy])



#------------------------------------------------------------------------------
# Check for X-Windows.  If the X-Windows implementation is XFree86, then also
# enable the `VideoMode' extension.
#------------------------------------------------------------------------------
AC_DEFUN([CS_X_CHECK_PREPARE],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    AC_REQUIRE([CS_CHECK_HOST])

    # Explicitly disable X checking on Windows to avoid discovering XFree86 in
    # case Cygwin is installed.  Doing so allows us to avoid Mesa's OpenGL
    # headers during the OpenGL check, and prefer Microsoft's OpenGL headers.
    # The Mesa OpenGL headers cause link errors because they do not employ
    # __stdcall.
    case $host_os in
	mingw*|cygwin*) with_x=no ;;
    esac

    # Explicitly disable X checking on MacOS/X in case the optional XFree86 is
    # installed since we want to avoid an undesirable dependency upon XFree86
    # in the CS executables and plugins.  If the user is actually
    # cross-building for Darwin on MacOS/X, then the --with-x option will
    # override this test (by setting cs_host_macosx to "no").
    AS_IF([test x$cs_host_macosx = xyes], [with_x=no])])

CS_X_CHECK_PREPARE
AC_PATH_X
AC_PATH_XTRA

AS_IF([test "$no_x" != yes],
    [# We must be especially careful to instruct CS_EMIT_BUILD_PROPERTY() to
    # emit the following properties even if their values are empty.  This is
    # necessary because of the way that Jam performs "product-wise" variable
    # expansion.  For example, if X is "x" and Y is undefined, Jam will expand
    # "$(X) $(Y)" to "", whereas every other tool in the world will expand it
    # to "x ".  Since the emitted X11.CFLAGS, X11.LFLAGS, XFREE86VM.CFLAGS, and
    # XFREE86VM.LFLAGS properties reference the other variables by name, we
    # must ensure that those other variables are defined (even if only with an
    # empty string).
    CS_EMIT_BUILD_PROPERTY([X11.AVAILABLE], [yes])
    CS_EMIT_BUILD_PROPERTY([X_CFLAGS], [$X_CFLAGS], [], [Y])
    CS_EMIT_BUILD_PROPERTY([X_LIBS], [$X_LIBS], [], [Y])
    CS_EMIT_BUILD_PROPERTY([X_PRE_LIBS], [$X_PRE_LIBS], [], [Y])
    CS_EMIT_BUILD_PROPERTY([X_EXTRA_LIBS], [$X_EXTRA_LIBS], [], [Y])
    CS_EMIT_BUILD_PROPERTY([X11.CFLAGS], [AS_ESCAPE([$(X_CFLAGS)])])
    CS_EMIT_BUILD_PROPERTY([X11.LFLAGS],
      [AS_ESCAPE([$(X_PRE_LIBS) $(X_LIBS) -lXext -lX11 $(X_EXTRA_LIBS)])])

    CS_CHECK_LIB_WITH([Xxf86vm],
	[AC_LANG_PROGRAM(
	    [[#define XK_MISCELLANY 1
	    #include <X11/Xlib.h>
	    #include <X11/Xutil.h>
	    #include <X11/Xatom.h>
	    #include <X11/extensions/xf86vmode.h>]],
	    [XF86VidModeLockModeSwitch(0, 0, 0)])], [], [],
	[CS_EMIT_BUILD_PROPERTY([XFREE86VM.AVAILABLE], [yes])
	CS_EMIT_BUILD_PROPERTY([XFREE86VM.CFLAGS], [AS_ESCAPE([$(X_CFLAGS)])])
	CS_EMIT_BUILD_PROPERTY([XFREE86VM.LFLAGS],
	    [AS_ESCAPE([$(X_PRE_LIBS) $(X_LIBS) -lXxf86vm -lXext -lX11
	    $(X_EXTRA_LIBS)])])], [],
	[$X_CFLAGS], [], [$X_PRE_LIBS $X_LIBS -lX11 -lXext $X_EXTRA_LIBS])

    CS_CHECK_LIB_WITH([Xaw],
	[AC_LANG_PROGRAM(
	    [[#include <X11/Shell.h>
	    #include <X11/Xaw/Form.h>]],
	    [XtVaGetValues(0,0,0,0)])],
	[], [],	[CS_EMIT_BUILD_RESULT([cs_cv_libXaw], [XAW])], [],
	[$X_CFLAGS], [], [$X_PRE_LIBS $X_LIBS -lXt -lX11 $X_EXTRA_LIBS])])



#------------------------------------------------------------------------------
# Check for OpenGL and companions.  Also check if GLX extensions are supported
# and define the (backward) header property CSGL_EXT_STATIC_ASSERTION if they
# are _not_ supported. Finally, check if the some GL types are exported by
# the GL headers. Some are starting to show up in very modern headers, but are
# absent from most.
#
# IMPLEMENTATION NOTES
#
# GLU is not required or used by CrystalSpace itself, but certain 3rd party
# libraries may depend on it (notably Cg).
#------------------------------------------------------------------------------
CS_CHECK_OPENGL
AC_DEFUN([CS_CHECK_OPENGL_TYPE],
    [CS_CHECK_BUILD([for $1], [cs_cv_libgl_$1],
	[AC_LANG_PROGRAM(
	    [CS_GL_INCLUDE([CS_OPENGL_PATH],[GL],[gl.h])],
		[$1 x; (void)x;])],
	[], [], [$2], [], [],
	[$cs_cv_libgl_cflags], [$cs_cv_libgl_lflags], [$cs_cv_libgl_libs])])
AS_IF([test $cs_cv_libgl = yes],
    [CS_CHECK_OPENGL_TYPE([GLhandleARB], [CS_HEADER_PROPERTY([CS_HAVE_GLHANDLEARB_T])])
    CS_CHECK_OPENGL_TYPE([GLcharARB], [CS_HEADER_PROPERTY([CS_HAVE_GLCHARARB_T])]) 
    CS_CHECK_OPENGL_TYPE([GLchar], [CS_HEADER_PROPERTY([CS_HAVE_GLCHAR_T])])])
CS_CHECK_GLU
CS_CHECK_GLX
# Check for GLX extensions.  Define CSGL_EXT_STATIC_ASSERTION if _not_
# present.
CS_CHECK_GLXEXT([], [CS_HEADER_PROPERTY([CSGL_EXT_STATIC_ASSERTION])])


#------------------------------------------------------------------------------
# Check for DirectDraw.  At least one user (Jorrit) has a strange installation
# in which inclusion of <windows.h> fails if an int32 is not present, thus we
# must take this into account.  With Cygwin, we must check for the DirectX
# headers in /usr/include/directx.
#------------------------------------------------------------------------------
# CS_CHECK_DIRECTX([DSOUND-VERSION])
AC_DEFUN([CS_CHECK_DIRECTX],
    [CS_CHECK_BUILD([for DirectX$1], [cs_cv_libdirectx$1],
	[AC_LANG_PROGRAM(
	    [[#if !HAVE_TYPE_INT32
	    typedef long int32;
	    #endif
	    #define DIRECTINPUT_VERSION 0x0500
	    #include <windows.h>
	    #include <ddraw.h>
	    #include <dsound.h>
	    #include <dinput.h>]],
	    [DirectDrawCreate(0,0,0); 
	     DirectSoundCreate$1(0,0,0); 
	     DirectInputCreate(0,0,0,0)])],
	[CS_CREATE_TUPLE([],[],[-lddraw -ldsound -ldinput]) \
	CS_CREATE_TUPLE([-I/usr/include/directx],[],
	    [-lddraw -ldsound -ldinput])], 
	[C++], [CS_EMIT_BUILD_RESULT([cs_cv_libdirectx$1], [DIRECTX$1])])])

AS_IF([test $cs_host_family = windows],
    [AC_CHECK_TYPE([int32], [AC_DEFINE([HAVE_TYPE_INT32])], [])
    CS_CHECK_DIRECTX([])
    CS_CHECK_DIRECTX([8])])



#------------------------------------------------------------------------------
# Check for the Windows Multimedia library, used by the 'sndwaveout' plugin.
#------------------------------------------------------------------------------
AS_IF([test $cs_host_family = windows],
    [CS_CHECK_BUILD([for waveout], [cs_cv_libwaveout],
	[AC_LANG_PROGRAM(
	    [[#include <windows.h>
	    #include <mmsystem.h>]],
	    [waveOutOpen(0, 0, 0, 0, 0, 0)])],
	[CS_CREATE_TUPLE([],[],[-lwinmm])], 
	[], [CS_EMIT_BUILD_RESULT([cs_cv_libwaveout], [WAVEOUT])])])



#------------------------------------------------------------------------------
# Check for zlib.  In addition to the standard installed locations, we also
# search CS/libs/zlib*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([z],
    [AC_LANG_PROGRAM([[#include <zlib.h>]], [zlibVersion()])],
    [$srcdir/libs/zlib*], [], [CS_EMIT_BUILD_RESULT([cs_cv_libz], [ZLIB])],
    [AC_MSG_NOTICE([*** zlib not found; VFS will be unusable])], 
    [], [], [], [zlib])



#------------------------------------------------------------------------------
# Check for libpng.  In addition to the standard installed locations, we also
# search CS/libs/libpng*.  Some platforms, such as RedHat 8.x require libm.a to
# be linked with libpng, so use libm.a if available.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LIBPNG_PREPARE],
    [AC_REQUIRE([CS_CHECK_COMMON_LIBS])])

AS_IF([test "$cs_cv_libz" = yes],
    [CS_CHECK_LIBPNG_PREPARE
    CS_CHECK_LIB_WITH([png],
	[AC_LANG_PROGRAM([[#include <png.h>]],
	    [png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)])],
	[$srcdir/libs/libpng*], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libpng], [PNG])], [],
	[$cs_cv_libz_cflags $cs_cv_libm_cflags],
	[$cs_cv_libz_lflags $cs_cv_libm_lflags],
	[$cs_cv_libz_libs $cs_cv_libm_libs], [libpng])])



#------------------------------------------------------------------------------
# Check for libjpeg.  In addition to the standard installed locations, we also
# search CS/libs/*jpeg*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([jpeg],
    [AC_LANG_PROGRAM(
	[[#include <stddef.h>
	#include <stdio.h>
	#include <jpeglib.h>]],
	[jpeg_create_compress(0)])],
    [$srcdir/libs/*jpeg*], [], [CS_EMIT_BUILD_RESULT([cs_cv_libjpeg], [JPEG])],
    [], [], [], [], [libjpeg])



#------------------------------------------------------------------------------
# Check for libmng.  In addition to the standard installed locations, we also
# search CS/libs/libmng*. Note that libmng on some OpenBSD installations has a
# dependency on lcms (http://www.littlecms.com/), so we must check for this
# library, and utilize it explicitly if present.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([lcms],
    [AC_LANG_PROGRAM([[#include <lcms.h>]],
	[LPLUT p = cmsAllocLUT(); cmsFreeLUT(p);])],
	[/usr/include/lcms| /usr/local/include/lcms|])

CS_CHECK_LIB_WITH([mng],
    [AC_LANG_PROGRAM([[#include <libmng.h>]], [mng_version_release()])],
    [$srcdir/libs/libmng*], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libmng], [MNG])], [],
    [$cs_cv_liblcms_cflags $cs_cv_libjpeg_cflags $cs_cv_libz_cflags],
    [$cs_cv_liblcms_lflags $cs_cv_libjpeg_lflags $cs_cv_libz_lflags],
    [$cs_cv_liblcms_libs $cs_cv_libjpeg_libs $cs_cv_libz_libs],
    [libmng])



#------------------------------------------------------------------------------
# Check for MikMod.  In addition to the standard installed locations, we also
# search CS/libs/libmikmod*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([mikmod],
    [AC_LANG_PROGRAM(
	[[#include <mikmod.h>]],
	[MikMod_Init("mikmodtest"); MikMod_Exit();])],
    [$srcdir/libs/libmikmod*], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libmikmod], [MIKMOD])], [],
    [$cs_cv_sys_pthread_cflags],
    [$cs_cv_sys_pthread_lflags],
    [$cs_cv_sys_pthread_libs])



#------------------------------------------------------------------------------
# Check for Ogg/Vorbis.  In addition to the standard installed locations, we
# also search CS/libs/libogg*, CS/libs/libvorbis.*, CS/libs/libvorbisfile*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([ogg],
    [AC_LANG_PROGRAM(
	[[#include <ogg/ogg.h>]],
	[ogg_sync_state state; ogg_sync_init(&state);])],
    [$srcdir/libs/libogg*], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libogg], [OGG])])

AS_IF([test "$cs_cv_libogg" = yes],
    [CS_CHECK_LIB_WITH([vorbis],
	[AC_LANG_PROGRAM([[#include <vorbis/codec.h>]],
	  [vorbis_info info; vorbis_info_init(&info);])],
	[$srcdir/libs/libvorbis.*], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libvorbis], [VORBIS])], [],
	[$cs_cv_libogg_cflags],
	[$cs_cv_libogg_lflags],
	[$cs_cv_libogg_libs])])

AS_IF([test "$cs_cv_libvorbis" = yes],
    [CS_CHECK_LIB_WITH([vorbisfile],
	[AC_LANG_PROGRAM(
	    [[#include <vorbis/vorbisfile.h>
	    #include <stdio.h>]],
	    [OggVorbis_File file; ov_test(stdin, &file, "", 0);])],
	[$srcdir/libs/libvorbisfile*], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libvorbisfile], [VORBISFILE])], [],
	[$cs_cv_libogg_cflags $cs_cv_libvorbis_cflags],
	[$cs_cv_libogg_lflags $cs_cv_libvorbis_lflags],
	[$cs_cv_libogg_libs $cs_cv_libvorbis_libs])])



#------------------------------------------------------------------------------
# Check for lib3ds.  In addition to the standard installed locations, we also
# search CS/libs/lib3ds*.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LIB3DS_PREPARE],
    [AC_REQUIRE([CS_CHECK_COMMON_LIBS])])

CS_CHECK_LIB3DS_PREPARE

CS_CHECK_LIB_WITH([3ds],
    [AC_LANG_PROGRAM([[#include <lib3ds/file.h>]],[lib3ds_file_new();])],
    [$srcdir/libs/lib3ds*], [], [], [], [], [], [$cs_cv_libm_libs],
    [lib3ds, lib3ds-120])

AS_IF([test $cs_cv_lib3ds = yes],
    [CS_CHECK_BUILD([if lib3ds is sufficiently recent], [cs_cv_lib3ds_recent],
	[AC_LANG_PROGRAM([[#include <lib3ds/io.h>]], [lib3ds_io_free(0);])],
	[], [], [CS_EMIT_BUILD_RESULT([cs_cv_lib3ds], [3DS])], [], [],
	[$cs_cv_lib3ds_cflags], [$cs_cv_lib3ds_lflags], [$cs_cv_lib3ds_libs])])



#------------------------------------------------------------------------------
# Check for ODE.  In addition to the standard installed locations, we also
# search CS/libs/libode*.  Also check if it is a sufficiently recent version.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([ode],
    [AC_LANG_PROGRAM([[#include <ode/ode.h>]], [dWorldCreate()])],
    [$srcdir/libs/libode*], [C++])

AS_IF([test $cs_cv_libode = yes],
    [CS_CHECK_BUILD([if libode is built with OPCODE support],
	[cs_cv_libode_opcode],
	[AC_LANG_PROGRAM(
	    [[#include <ode/ode.h>]],
	    [dGeomTriMeshDataCreate();])],
	[], [C++], [], [], [],
	[$cs_cv_libode_cflags], [$cs_cv_libode_lflags], [$cs_cv_libode_libs])],
	[cs_cv_libode_opcode=no])

AS_IF([test $cs_cv_libode_opcode = yes],
    [CS_CHECK_BUILD([if libode is sufficiently recent], [cs_cv_libode_recent],
	[AC_LANG_PROGRAM(
	    [[#include <ode/ode.h>]],
	    [dGeomTriMeshGetTriangleCount(0);])],
	[], [C++],
	[CS_EMIT_BUILD_RESULT([cs_cv_libode], [ODE])], [], [],
	[$cs_cv_libode_cflags], [$cs_cv_libode_lflags], [$cs_cv_libode_libs])],
	[cs_cv_libode_recent=no])



#------------------------------------------------------------------------------
# Check for bullet physics library.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([bullet],
    [AC_LANG_PROGRAM([[
    #include <BroadphaseCollision/SimpleBroadphase.h>
    #include <CcdPhysics/CcdPhysicsController.h>
    #include <CcdPhysics/CcdPhysicsEnvironment.h>
    #include <CollisionDispatch/CollisionDispatcher.h>
    ]],
    [CollisionDispatcher dispatcher; SimpleBroadphase broadphase;
    new CcdPhysicsEnvironment (&dispatcher, &broadphase)])],
    [], [C++], [CS_EMIT_BUILD_RESULT([cs_cv_libbullet], [BULLET])], [],
    [], [], 
    [-lbulletccdphysics -lbulletphysicsinterfacecommon -lbulletdynamics -lbullet])



#------------------------------------------------------------------------------
# Check for OpenAL.
#------------------------------------------------------------------------------
# CS_AL_INCLUDE(CPP-MACRO,FALLBACK,HEADER)
AC_DEFUN([CS_AL_INCLUDE],
    [[#ifndef CS_HEADER_GLOBAL
    #define CS_HEADER_GLOBAL(X,Y) CS_HEADER_GLOBAL_COMPOSE(X,Y)
    #define CS_HEADER_GLOBAL_COMPOSE(X,Y) <X/Y>
    #endif
    #ifdef $1
    #include CS_HEADER_GLOBAL($1,$3)
    #else
    #include <$2/$3>
    #endif]])

AC_DEFUN([CS_CHECK_LIBOPENAL],
    [AC_REQUIRE([CS_CHECK_COMMON_LIBS])
    CS_CHECK_BUILD([for OpenAL], [cs_cv_libopenal],
	[AC_LANG_PROGRAM(
	    [CS_AL_INCLUDE([CS_OPENAL_PATH],[AL],[alut.h])],
	    [alutInit(0,0);])],
	[CS_CREATE_TUPLE([-DCS_OPENAL_PATH=OpenAL],[],[-framework OpenAL]) \
	CS_CREATE_TUPLE([],[],[-lopenal]) \
	CS_CREATE_TUPLE([],[],[-lopenal32]) \
	CS_CREATE_TUPLE([],[],[-lalut -lopenal]) \
	CS_CREATE_TUPLE([],[],[-lalut -lopenal32]) \
	CS_CREATE_TUPLE([],[],[-lALut -lOpenAL32]) \
	CS_CREATE_TUPLE([],[],[-lopenal -ldsound]) \
	CS_CREATE_TUPLE([],[],[-lopenal32 -ldsound]) \
	CS_CREATE_TUPLE([],[],[-lalut -lopenal -ldsound]) \
	CS_CREATE_TUPLE([],[],[-lalut -lopenal32 -ldsound])], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libopenal], [OPENAL])], [], [],
	[$cs_cv_sys_pthread_cflags],
	[$cs_cv_sys_pthread_lflags],
	[$cs_cv_sys_pthread_libs $cs_cv_libdl_libs $cs_cv_libm_libs])])

CS_CHECK_LIBOPENAL



#------------------------------------------------------------------------------
# Check for FreeType2. Note that on MacOS/X, Fink strangely installs
# `freetype-config' in /sw/lib/freetype2/bin, thus we augment PATH so that
# CS_CHECK_LIB_WITH() will take this unusual location into account.
#------------------------------------------------------------------------------
PATH=$PATH$PATH_SEPARATOR/sw/lib/freetype2/bin
export PATH

CS_CHECK_LIB_WITH([freetype2],
    [AC_LANG_PROGRAM(
	[[#include <ft2build.h>
	#include FT_FREETYPE_H]],
	[FT_Library p; FT_Init_FreeType(&p); FT_Done_FreeType(p);])],
	[], [], [], [], [], [], [], [freetype])

AS_IF([test $cs_cv_libfreetype2 = yes],
    [CS_CHECK_BUILD([if freetype2 is sufficiently recent],
	[cs_cv_freetype2_recent],
	[AC_LANG_PROGRAM(
	    [[#include <ft2build.h>
	    #include FT_FREETYPE_H
	    #include FT_GLYPH_H
	    #include FT_MODULE_H]], [return
	    FT_LOAD_TARGET_MONO|FT_RENDER_MODE_NORMAL|FT_ENCODING_UNICODE])],
	[], [], [CS_EMIT_BUILD_RESULT([cs_cv_libfreetype2], [FT2])], [], [],
	[$cs_cv_libfreetype2_cflags],
	[$cs_cv_libfreetype2_lflags],
	[$cs_cv_libfreetype2_libs])])



#------------------------------------------------------------------------------
# For Mingw, test if we need to link explicitly with libmingwex.a.  Older
# versions of Mingw did not have this library, whereas newer interim versions
# supply it but do not link automatically with it.  The very newest versions
# link with libmingwex.a automatically.  To see if libmingwex.a is required, we
# try calling opendir(), which exists in libming32.a for older releases, and in
# libmingwex.a for newer releases.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LIBMINGWX],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
	mingw*)
	    AC_CACHE_CHECK([for libmingwex], [cs_cv_libmingwex],
		[CS_BUILD_IFELSE(
		    [AC_LANG_PROGRAM([[#include <dirent.h>]], [opendir("");])],
		    [CS_CREATE_TUPLE() CS_CREATE_TUPLE([],[],[-lmingwex])], [],
		    [AS_IF([test -n "$cs_build_libs"],
			[cs_cv_libmingwex=yes], [cs_cv_libmingwex=no])],
		    [cs_cv_libmingwex=no])])
	    ;;
	*) cs_cv_libmingwex=no ;;
    esac])

CS_CHECK_LIBMINGWX
AS_IF([test $cs_cv_libmingwex = yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lmingwex], [+])])



#------------------------------------------------------------------------------
# Check for Cal3D.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([cal3d],
    [AC_LANG_PROGRAM([[#include <cal3d/cal3d.h>]],
	[CalModel* m = 0; m->getMixer();])], [], [C++])

AS_IF([test $cs_cv_libcal3d = yes],
    [CS_CHECK_BUILD([if cal3d is sufficiently recent], [cs_cv_cal3d_recent],
	[AC_LANG_PROGRAM(
	    [[#include <cal3d/cal3d.h>]],
	    [int x = LOADER_FLIP_WINDING;])],
	[], [C++],
	[CS_EMIT_BUILD_RESULT([cs_cv_libcal3d], [CAL3D])], [], [],
	[$cs_cv_libcal3d_cflags],
	[$cs_cv_libcal3d_lflags],
	[$cs_cv_libcal3d_libs])])



#------------------------------------------------------------------------------
# Check for SDL.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([sdl],
    [AC_LANG_PROGRAM(
	[[#include <SDL.h>
	#undef main]],
	[SDL_Init(0); SDL_Quit();])])

AS_IF([test $cs_cv_libsdl = yes],
    [CS_CHECK_BUILD([if SDL is sufficiently recent], [cs_cv_sdl_recent],
	[AC_LANG_PROGRAM(
	    [[#include <SDL.h>
	    #undef main]],
	    [SDL_EnableKeyRepeat(0,0)])],
	[], [], [CS_EMIT_BUILD_RESULT([cs_cv_libsdl], [SDL])], [], [],
	[$cs_cv_libsdl_cflags], [$cs_cv_libsdl_lflags], [$cs_cv_libsdl_libs])])


#------------------------------------------------------------------------------
# Check for wxWidgets. Also check if it was built with OpenGL support since it
# is required by the Crystal Space wxWidgets canvas.
#
# IMPLEMENTATION NOTE
#
# On Windows, wx-config's --gl-libs options incorrectly emits a reference to a
# non-existent library (wxmsw_glxxx.lib). To handle this broken case, we first
# try the result of --libs and --gl-libs together; if that fails, then we try
# only --libs augmented with the results of our own OpenGL checks. On more 
# recent WX versions it seems that on Win32 both the libraries from --gl-libs 
# and the OpenGL checks are needed - the last case.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([wx],
    [AC_LANG_PROGRAM(
	[[#include "wx/wxprec.h"
	#ifdef __BORLANDC__
	#pragma hdrstop
	#endif
	#ifndef WX_PRECOMP
	#include "wx/wx.h"
	#endif
	]],
	[[wxWindow* p = 0; int w,h; p->GetClientSize(&w,&h);]])],
    [], [C++])
    
AS_IF([test $cs_cv_libwx = yes],
    [AS_IF([test -z "$cs_cv_libwx_opengl"],
	[AS_IF([test -n "$CONFIG_WX"],
	    [cs_cv_libwx_gllibs=CS_RUN_PATH_NORMALIZE([$CONFIG_WX --gl-libs])],
	    [cs_cv_libwx_gllibs=''])])
    CS_CHECK_BUILD([if wxWidgets is built with OpenGL support],
	[cs_cv_libwx_opengl],
	[AC_LANG_PROGRAM(
	    [[#include "wx/wxprec.h"
	    #ifdef __BORLANDC__
	    #pragma hdrstop
	    #endif
	    #ifndef WX_PRECOMP
	    #include "wx/wx.h"
	    #endif
	    #include "wx/glcanvas.h"
	    ]],
	    [wxGLCanvas* f = new wxGLCanvas(0, wxID_ANY); glVertex3f(0,0,0)])],
	[CS_CREATE_TUPLE(
	    [$cs_cv_libwx_cflags],
	    [$cs_cv_libwx_lflags],
	    [$cs_cv_libwx_libs   $cs_cv_libwx_gllibs]) \
	CS_CREATE_TUPLE(
	    [$cs_cv_libwx_cflags $cs_cv_libglu_cflags],
	    [$cs_cv_libwx_lflags $cs_cv_libglu_lflags],
	    [$cs_cv_libwx_libs   $cs_cv_libglu_libs])  \
	CS_CREATE_TUPLE(
	    [$cs_cv_libwx_cflags $cs_cv_libglu_cflags],
	    [$cs_cv_libwx_lflags $cs_cv_libglu_lflags],
	    [$cs_cv_libwx_libs   $cs_cv_libwx_gllibs $cs_cv_libglu_libs])],
	[C++])],
    [cs_cv_libwx_opengl=no])
    
AS_IF([test $cs_cv_libwx_opengl = yes],
    [CS_EMIT_BUILD_RESULT([cs_cv_libwx_opengl], [WX])])



#----------------------------------------------------------------------------
# Check for CEGUI.
#----------------------------------------------------------------------------
CS_CHECK_LIB_WITH([CEGUI],
    [AC_LANG_PROGRAM([[#include <CEGUI.h>]],
    [CEGUI::WindowManager::getSingleton();])], [], [C++],
    [], [], [], [], [], [CEGUIBase])

AS_IF([test $cs_cv_libCEGUI = yes],
    [CS_CHECK_BUILD([if CEGUI is sufficiently recent], [cs_cv_CEGUI_recent],
	[AC_LANG_PROGRAM(
	    [[#include <falagard/CEGUIFalWidgetLookManager.h>]],
	    [CEGUI::WidgetLookManager::getSingleton();])],
	[], [C++],
	[CS_EMIT_BUILD_RESULT([cs_cv_libCEGUI], [CEGUI])], [], [],
	[$cs_cv_libCEGUI_cflags],
	[$cs_cv_libCEGUI_lflags],
	[$cs_cv_libCEGUI_libs])])



#------------------------------------------------------------------------------
# Check if GNU/Linux joystick API is present.
#------------------------------------------------------------------------------
AS_IF([test $cs_host_family != windows],
    [CS_CHECK_BUILD([for GNU/Linux joystick API], [cs_cv_sys_linux_joystick],
	[AC_LANG_PROGRAM(
	    [[#include <fcntl.h>
	    #include <sys/ioctl.h>
	    #include <linux/joystick.h>]],
	    [unsigned char axes;
	    unsigned char buttons;
	    int version;
	    char name[[[128]]];
	    int fd;
	    struct js_event js;
	    (void)js;
	    ioctl(fd, JSIOCGVERSION, &version);
	    ioctl(fd, JSIOCGAXES, &axes);
	    ioctl(fd, JSIOCGBUTTONS, &buttons);
	    ioctl(fd, JSIOCGNAME(128), name);])],
	[], [],
	[CS_EMIT_BUILD_PROPERTY([LINUXJOYSTICK.AVAILABLE], [yes])])])



#------------------------------------------------------------------------------
# Check for Cg shader package.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LIBCG],
    [AC_REQUIRE([CS_CHECK_COMMON_LIBS])
    AC_REQUIRE([CS_CHECK_PTHREAD])
    CS_CHECK_LIB_WITH([Cg],
	[AC_LANG_PROGRAM(
	    [[#include <Cg/cg.h>]],
	    [CGcontext c = cgCreateContext()])],
	[], [], [], [],
	[$cs_cv_libm_cflags $cs_cv_sys_pthread_cflags], 
	[$cs_cv_libm_lflags $cs_cv_sys_pthread_lflags], 
	[$cs_cv_libm_libs   $cs_cv_sys_pthread_libs],
	[cg, CG])

    AS_IF([test $cs_cv_libCg = yes],
	[CS_CHECK_BUILD([if Cg is sufficiently recent], [cs_cv_libCg_recent],
	    [AC_LANG_PROGRAM(
	    	[[#include <Cg/cg.h>]], [cgSetErrorHandler(0,0);])],
	    [], [], [], [], [],
	    [$cs_cv_libCg_cflags],
	    [$cs_cv_libCg_lflags],
	    [$cs_cv_libCg_libs])],
	[cs_cv_libCg_recent=no])

    AS_IF([test $cs_cv_libCg_recent = yes],
	[CS_CHECK_LIB_WITH([CgGL],
	    [AC_LANG_PROGRAM(
		[[#include <Cg/cg.h>
		#include <Cg/cgGL.h>]],
		[CGprogram p; cgGLBindProgram(p)])],
	    [], [C++],
	    [CS_EMIT_BUILD_RESULT([cs_cv_libCgGL], [CG])], [],
	    [$cs_cv_libCg_cflags $cs_cv_libgl_cflags $cs_cv_libglu_cflags],
	    [$cs_cv_libCg_lflags $cs_cv_libgl_lflags $cs_cv_libglu_lflags],
	    [$cs_cv_libCg_libs   $cs_cv_libgl_libs   $cs_cv_libglu_libs],
	    [cggl, cgGL, CGGL, Cg, cg, CG])])])

CS_CHECK_LIBCG



#------------------------------------------------------------------------------
# Check for OpenSoundSystem (OSS).
#------------------------------------------------------------------------------
AS_IF([test $cs_host_family != windows],
    [AC_CHECK_HEADER([soundcard.h],
	[CS_HEADER_PROPERTY([CS_HAVE_SOUNDCARD_H])
	cs_cv_header_soundcard_h=yes],
	[AC_CHECK_HEADER([sys/soundcard.h],
	    [CS_HEADER_PROPERTY([CS_HAVE_SYS_SOUNDCARD_H])
	    cs_cv_header_soundcard_h=yes],
	    [AC_CHECK_HEADER([machine/soundcard.h],
		  [CS_HEADER_PROPERTY([CS_HAVE_MACHINE_SOUNDCARD_H])
		  cs_cv_header_soundcard_h=yes],
		  [cs_cv_header_soundcard_h=no])])])]
    
    [AS_IF([test $cs_cv_header_soundcard_h = yes],
	[CS_EMIT_BUILD_PROPERTY([OSS.AVAILABLE], [yes])])])

#------------------------------------------------------------------------------
# Check for JACK daemon jackasyn library (JACKASYN)
#------------------------------------------------------------------------------
AS_IF([test $cs_host_family != windows],
    [CS_CHECK_LIB_WITH([jackasyn],
	[AC_LANG_PROGRAM(
	    [[#include <stdio.h>
	      #include <libjackoss.h>]],
	    [jackoss_open("/dev/dsp",1);])],
	[], [], [CS_EMIT_BUILD_RESULT([cs_cv_libjackasyn], [JACKASYN])])])

#------------------------------------------------------------------------------
# Check for Advanced GNU/Linux Sound Architecture (ALSA)
#------------------------------------------------------------------------------
AS_IF([test $cs_host_family != windows],
    [CS_CHECK_LIB_WITH([asound],
	[AC_LANG_PROGRAM(
	    [[#include <alsa/asoundlib.h>]],
	    [snd_pcm_hw_params_t* p; snd_pcm_hw_params_alloca(&p);])],
	[], [], [CS_EMIT_BUILD_RESULT([cs_cv_libasound], [ALSA])])])



#------------------------------------------------------------------------------
# Check for Colour Ascii Art library (libcaca).
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([caca],
    [AC_LANG_PROGRAM([[#include <caca.h>]], [caca_init(); caca_end();])],
    [], [], [CS_EMIT_BUILD_RESULT([cs_cv_libcaca], [LIBCACA])], [], [], [], [],
    [libcaca])



#------------------------------------------------------------------------------
# Check for SpiderMonkey JavaScript library.
#------------------------------------------------------------------------------
case $cs_host_family in
    windows)
	cs_spidermonkey_platform='XP_WIN'
	;;
    *)
	cs_spidermonkey_platform='XP_UNIX'
	;;
esac
CS_CHECK_LIB_WITH([js],
    [AC_LANG_PROGRAM([[#include <js/jsapi.h>]], 
    [JS_NewRuntime(0)])],
    [], [], [CS_EMIT_BUILD_RESULT([cs_cv_libjs], [JS])], [], 
    [-D$cs_spidermonkey_platform])



#------------------------------------------------------------------------------
# Check if regular expressions are available.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([for regexp support], [cs_cv_regexp],
    [AC_LANG_PROGRAM(
	[[#ifdef HAVE_SYS_TYPES_H
	#include <sys/types.h>
	#endif
	#include <regex.h>]],
	[regex_t re; regcomp (&re, 0, 0); regexec (&re, 0, 0, 0, 0);])],
    [], [],
    [CS_HEADER_PROPERTY([CS_HAVE_REGEX])
    CS_EMIT_BUILD_PROPERTY([REGEX.AVAILABLE], [yes])])



#------------------------------------------------------------------------------
# Check if backtrace() is available.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([for backtrace], [cs_cv_backtrace],
    [AC_LANG_PROGRAM(
	[[#ifdef HAVE_SYS_TYPES_H
	#include <sys/types.h>
	#endif
	#include <execinfo.h>]],
	[void* foo; int x; x = backtrace (&foo, 0); 
	backtrace_symbols (&foo, x);])],
    [], [],
    [CS_HEADER_PROPERTY([CS_HAVE_BACKTRACE])
    CS_EMIT_BUILD_PROPERTY([BACKTRACE.AVAILABLE], [yes])])



#------------------------------------------------------------------------------
# Check if setenv() is available.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([for setenv], [cs_cv_setenv],
    [AC_LANG_PROGRAM(
	[[#ifdef HAVE_SYS_TYPES_H
	#include <sys/types.h>
	#endif
	#include <stdlib.h>]],
	[setenv(0, 0, 0);])],
    [], [],
    [CS_HEADER_PROPERTY([CS_HAVE_SETENV])])



#------------------------------------------------------------------------------
# Check for a couple of memory-related functions.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([for _aligned_malloc], [cs_cv__aligned_malloc],
    [AC_LANG_PROGRAM(
	[[#ifdef HAVE_SYS_TYPES_H
	#include <sys/types.h>
	#endif
	#include <malloc.h>]],
	[_aligned_malloc(0, 0);])],
    [], [],
    [CS_HEADER_PROPERTY([CS_HAVE__ALIGNED_MALLOC])])



#------------------------------------------------------------------------------
# Check whether malloc.h can be included without a warning.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([if <malloc.h> can be included], [cs_cv_malloc_h_includeable],
    [AC_LANG_PROGRAM(
        [[#include <malloc.h>]])],
    [], [C++], 
    [CS_HEADER_PROPERTY([CS_HAVE_MALLOC_H])], [],
    [$cs_cv_prog_cxx_enable_warnings $cs_cv_prog_cxx_enable_errors])



#------------------------------------------------------------------------------
# Check if cxxabi.h is available and if the abi::__cxa_demangle() function is 
# available.
#------------------------------------------------------------------------------
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([cxxabi.h],
    [CS_CHECK_BUILD([for abi::__cxa_demangle], [cs_cv_abi_cxa_demangle],
        [AC_LANG_PROGRAM(
            [[#include <cxxabi.h>]],
            [abi::__cxa_demangle(0, 0, 0, 0)])],
        [], [C++], [CS_HEADER_PROPERTY([CS_HAVE_ABI_CXA_DEMANGLE])])])
AC_LANG_POP([C++])



#------------------------------------------------------------------------------
# Check if TemplateToolkit2 is installed (needed for MSVC project generation).
#------------------------------------------------------------------------------
CS_CHECK_TEMPLATE_TOOLKIT2([emit])



#------------------------------------------------------------------------------
# Check if CppUnit, the unit-testing framework is available. If so,
# unit-testing will be enabled automatically for any application, plugin, and
# library which has a subdirectory named "t/". The subdirectory should contain
# CppUnit test fragments with file extension ".t".
#------------------------------------------------------------------------------
CS_CHECK_CPPUNIT([emit])



#------------------------------------------------------------------------------
# Determine if plugin modules must implement main().  This is needed on some
# platforms to ensure that global constructors and functions which initialize
# global variables in the plugin module get invoked.
#
# IMPLEMENTATION NOTES
#
# Currently, we answer this question by making the (potentially incorrect)
# assumption that plugins on platforms other than Windows, MacOS/X, and Darwin
# require main().  In the future, we should use a proper test to make this
# determination.
#------------------------------------------------------------------------------
AC_DEFUN([CS_SYS_PLUGIN_MAIN],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    AC_CACHE_CHECK([if plugins require main()], [cs_cv_sys_plugin_main],
	[case $host_os in
	    mingw*|cygwin*) cs_cv_sys_plugin_main=no ;;
	    darwin*) cs_cv_sys_plugin_main=no ;;
	    # Everything else is assumed to be Unix or Unix-like.
	    *) cs_cv_sys_plugin_main=yes ;;
	esac])
    AS_IF([test $cs_cv_sys_plugin_main = yes],
	[CS_HEADER_PROPERTY([CS_UNIX_PLUGIN_REQUIRES_MAIN])])])

CS_SYS_PLUGIN_MAIN



#------------------------------------------------------------------------------
# Check for sizeof(long) to determine whether the platform is 32 or 64 bit.
# Also needed by the PRIx99 macros provided by CS.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LONG_BITS],
    [AC_CHECK_SIZEOF([long])
    AS_IF([test $ac_cv_sizeof_long = 8],
	[cs_long_bits=64], [cs_long_bits=32])])

CS_CHECK_LONG_BITS
CS_HEADER_PROPERTY([CS_PROCESSOR_SIZE], [$cs_long_bits])
CS_EMIT_BUILD_PROPERTY([TARGET.PROCESSORSIZE], [$cs_long_bits])
CS_HEADER_PROPERTY([CS_LONG_SIZE], [$ac_cv_sizeof_long])



#------------------------------------------------------------------------------
# Determine host CPU.
#
# CS_CHECK_HOST_CPU
#	Set the shell variable cs_host_cpu to a normalized form of the CPU name
#	returned by config.guess/config.sub.  Also takes the normalized name,
#	uppercases and appends it to the string "CS_PROCESSOR_" to form a name
#	suitable for the C preprocessor.  Assigns this value to the shell
#	variable cs_host_cpu_cpp_define.  Typically, Crystal Space's conception
#	of CPU name is the same as that returned by config.guess/config.sub,
#	but there may be exceptions as seen in the `case' statement.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_HOST_CPU],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_cpu in
	[[Ii][3-9]86*|[Xx]86*]) cs_host_cpu=x86 ;;
	*) cs_host_cpu=$host_cpu ;;
    esac
    cs_host_cpu_normalized="AS_TR_CPP([$cs_host_cpu])"])

CS_CHECK_HOST_CPU
CS_EMIT_BUILD_PROPERTY([TARGET.PROCESSOR], [$cs_host_cpu_normalized])
CS_HEADER_PROPERTY([CS_PROCESSOR_$cs_host_cpu_normalized])
CS_HEADER_PROPERTY([CS_PROCESSOR_NAME],
    [AS_ESCAPE(["])$cs_host_cpu[]AS_ESCAPE(["])])

AS_IF([test $cs_host_cpu = x86], 
  [CS_HEADER_PROPERTY([CS_HAVE_MMX])])



#------------------------------------------------------------------------------
# Check host endianess.
#------------------------------------------------------------------------------
AC_C_BIGENDIAN(
    [CS_HEADER_PROPERTY([CS_BIG_ENDIAN])],
    [CS_HEADER_PROPERTY([CS_LITTLE_ENDIAN])])



#------------------------------------------------------------------------------
# Check if stdint.h is available and if the INT64_C() macro is available.
#------------------------------------------------------------------------------
AC_CHECK_HEADERS([stdint.h],
    [CS_HEADER_PROPERTY([CS_HAVE_STDINT_H])
    CS_CHECK_BUILD([for INT64_C], [cs_cv_c_int64],
        [AC_LANG_PROGRAM(
            [[#define __STDC_CONSTANT_MACROS
            #define __STDC_LIMIT_MACROS
            #include <stdint.h>]],
            [(void)INT64_C(0x100000000)])],
        [], [C++], [CS_HEADER_PROPERTY([CS_HAVE_INT64_C])])])



#------------------------------------------------------------------------------
# Check if intptr_t, uintptr_t, intmax_t and uintmax_t are available.
#------------------------------------------------------------------------------
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([inttypes.h], [CS_HEADER_PROPERTY([CS_HAVE_INTTYPES_H])])
AC_CHECK_TYPE([intptr_t], [CS_HEADER_PROPERTY([CS_HAVE_INTPTR_T])], [],
    [[#include <stddef.h>
    #ifdef HAVE_INTTYPES_H
    #include <inttypes.h>
    #endif
    #ifdef HAVE_STDINT_H
    #include <stdint.h>
    #endif
    #ifdef HAVE_SYS_TYPES_H
    #include <sys/types.h>
    #endif]])
AC_CHECK_TYPE([intmax_t], [CS_HEADER_PROPERTY([CS_HAVE_INTMAX_T])], [],
    [[#include <stddef.h>
    #ifdef HAVE_INTTYPES_H
    #include <inttypes.h>
    #endif
    #ifdef HAVE_STDINT_H
    #include <stdint.h>
    #endif
    #ifdef HAVE_SYS_TYPES_H
    #include <sys/types.h>
    #endif]])



#------------------------------------------------------------------------------
# Check if the PRIx99 printf()-formatting adjunct macros are available.
#------------------------------------------------------------------------------
AC_CHECK_HEADERS([inttypes.h])
CS_CHECK_BUILD([for PRIx99 macros], [cs_cv_c_format_macros],
    [AC_LANG_PROGRAM(
        [[#include <stddef.h>
	#define __STDC_FORMAT_MACROS
	#if HAVE_INTTYPES_H
        #include <inttypes.h>
	#endif
	]],
        [char const* s = "widget %" PRIu32 " toggled";])],
    [], [], [CS_HEADER_PROPERTY([CS_HAVE_C_FORMAT_MACROS])])



#------------------------------------------------------------------------------
# Check if the types wchar_t and wint_t are available; also determine the size
# of wchar_t.  These types might be built-in, or they might come from the
# wchar.h or wctype.h header files.  Also determine availability of wcslen(),
# fputws(), fwide() and wcsrtombs().
#------------------------------------------------------------------------------
m4_define([cs_wc_include],
    [[#if HAVE_WCHAR_H
    #include <wchar.h>
    #endif
    #if HAVE_WCTYPE_H
    #include <wctype.h>
    #endif
    ]])

AC_LANG_PUSH([C++])

AC_CHECK_HEADERS([wchar.h], [CS_HEADER_PROPERTY([CS_HAVE_WCHAR_H])])
AC_CHECK_HEADERS([wctype.h], [CS_HEADER_PROPERTY([CS_HAVE_WCTYPE_H])])

AC_CHECK_TYPE([wint_t], [CS_HEADER_PROPERTY([CS_HAVE_WINT_T])], [],
    [cs_wc_include])

AC_CHECK_TYPE([wchar_t], [cs_type_wchar_t=yes], [cs_type_wchar_t=no],
    [cs_wc_include])

AS_IF([test $cs_type_wchar_t = yes],
    [CS_HEADER_PROPERTY([CS_HAVE_WCHAR_T])
    AC_CHECK_FUNC([wcslen], [CS_HEADER_PROPERTY([CS_HAVE_WCSLEN])])
    AC_CHECK_FUNC([fputws], [CS_HEADER_PROPERTY([CS_HAVE_FPUTWS])])
    AC_CHECK_FUNC([fwide], [CS_HEADER_PROPERTY([CS_HAVE_FWIDE])])
    AC_CHECK_FUNC([wcsrtombs], [CS_HEADER_PROPERTY([CS_HAVE_WCSRTOMBS])])
    AC_CHECK_SIZEOF([wchar_t], []
	[[#if HAVE_WCHAR_H
	#include <wchar.h>
	#endif]])
    CS_HEADER_PROPERTY([CS_WCHAR_T_SIZE], [$ac_cv_sizeof_wchar_t])])

AC_LANG_POP([C++])



#------------------------------------------------------------------------------
# Check if MAXPATHLEN is defined by the system headers.
#------------------------------------------------------------------------------
AC_CHECK_HEADERS([sys/param.h], [CS_HEADER_PROPERTY([CS_HAVE_SYS_PARAM_H])])
CS_CHECK_BUILD([for MAXPATHLEN], [cs_cv_c_macro_maxpathlen],
    [AC_LANG_PROGRAM(
	[[#include <stdio.h>
	#if HAVE_SYS_PARAM_H
	#include <sys/param.h>
	#endif]],
	[char s[[[MAXPATHLEN]]]; (void)s;])],
    [], [], [CS_HEADER_PROPERTY([CS_MAXPATHLEN], [MAXPATHLEN])], [], [],
    [$cs_cv_prog_cxx_enable_pedantic])



#------------------------------------------------------------------------------
# Check for BSD-style socket support.  Also test for presence of type
# socklen_t.  Note that we must not include both <winsock.h> and the Unix
# socket-related headers at the same time on Cygwin even though they both exist
# since the inclusion of both results in errors about structures and functions
# defined differently between <winsock.h> and the other headers.  Therefore, if
# <winsock.h> is present, we include only it, otherwise we include the Unix
# socket-related headers.  A related problem also occurs with the check for
# presence of <winsock.h> on Cygwin, thus this check is handled specially.  For
# this check, the "default-includes" normally used by AC_CHECK_HEADER() are
# suppressed.  This prevents complaints about structures and functions in
# <winsock.h> being defined differently by <sys/types.h> and <unistd.h>
#------------------------------------------------------------------------------
m4_define([cs_socket_flags],
    [CS_CREATE_TUPLE() \
    CS_CREATE_TUPLE([], [], [-lsocket]) \
    CS_CREATE_TUPLE([], [], [-lsocket -lnsl]) \
    CS_CREATE_TUPLE([], [], [-lwsock32])])

AC_CHECK_HEADERS([winsock.h], [], [], [/* no default includes */])

AC_CHECK_HEADERS(
    [unistd.h \
    sys/types.h \
    sys/socket.h \
    sys/ioctl.h \
    netinet/in.h \
    arpa/inet.h \
    netdb.h])

CS_CHECK_BUILD([for socket support], [cs_cv_sys_socket],
    [AC_LANG_PROGRAM(
	[[#if HAVE_WINSOCK_H
	#include <winsock.h>
	#else
	#if HAVE_SYS_TYPES_H
	#include <sys/types.h>
	#endif
	#if HAVE_NETINET_IN_H
	#include <netinet/in.h>
	#endif
	#if HAVE_ARPA_INET_H
	#include <arpa/inet.h>
	#endif
	#if HAVE_SYS_SOCKET_H
	#include <sys/socket.h>
	#endif
	#endif]],
	[struct in_addr a; inet_ntoa(a); socket(0,0,0);])],
    [cs_socket_flags], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_sys_socket], [SOCKET])])

AS_IF([test $cs_cv_sys_socket = yes],
    [AC_CHECK_TYPE([socklen_t],
	[CS_HEADER_PROPERTY([CS_HAVE_SOCKLEN_T])],
	[],
	[[#if HAVE_UNISTD_H
	#include <unistd.h>
	#endif
	#if HAVE_SYS_TYPES_H
	#include <sys/types.h>
	#endif
	#if HAVE_SYS_SOCKET_H
	#include <sys/socket.h>
	#endif
	#if HAVE_SYS_IOCTL_H
	#include <sys/ioctl.h>
	#endif
	#if HAVE_NETINET_IN_H
	#include <netinet/in.h>
	#endif
	#if HAVE_NETDB_H
	#include <netdb.h>
	#endif
	]])])



#------------------------------------------------------------------------------
# Check how to declare functions and types deprecated.
#------------------------------------------------------------------------------
CS_SYMBOL_QUALIFIER([how to declare deprecations],
    [cs_cv_prog_cxx_declare_deprecated],
    [__attribute__((deprecated))], [], [C++],
    [CS_HEADER_PROPERTY([CS_DEPRECATED_METHOD],
	[$cs_cv_prog_cxx_declare_deprecated])
    CS_HEADER_PROPERTY([CS_DEPRECATED_TYPE],
	[$cs_cv_prog_cxx_declare_deprecated])])



#------------------------------------------------------------------------------
# Check how to declare functions and types perfectly constant (not affected by
# any global state and always returning the same value for the same input).
#------------------------------------------------------------------------------
CS_SYMBOL_QUALIFIER([how to declare const functions],
    [cs_cv_prog_cxx_declare_const],
    [__attribute__((const))], [], [C++],
    [CS_HEADER_PROPERTY([CS_CONST_METHOD],
	[$cs_cv_prog_cxx_declare_const])])



#------------------------------------------------------------------------------
# Check how to declare functions and types pure (not affected by any global
# state).
#------------------------------------------------------------------------------
CS_SYMBOL_QUALIFIER([how to declare pure functions],
    [cs_cv_prog_cxx_declare_pure],
    [__attribute__((pure))], [], [C++],
    [CS_HEADER_PROPERTY([CS_PURE_METHOD],
	[$cs_cv_prog_cxx_declare_pure])])



#------------------------------------------------------------------------------
# Check if C++ compiler understands new C++ `explicit' and 'typename' keywords.
#------------------------------------------------------------------------------
AC_CACHE_CHECK([for C++ `explicit' keyword], [cs_cv_cxx_explicit],
    [CS_BUILD_IFELSE(
	[AC_LANG_PROGRAM([class A { public: explicit A(int); };], [])],
	[], [C++], [cs_cv_cxx_explicit=yes], [cs_cv_cxx_explicit=no])])
AS_IF([test $cs_cv_cxx_explicit != no],
    [CS_HEADER_PROPERTY([CS_HAVE_CXX_KEYWORD_EXPLICIT])])

AC_CACHE_CHECK([for C++ `typename' keyword], [cs_cv_cxx_typename],
    [CS_BUILD_IFELSE(
	[AC_LANG_PROGRAM(
	    [template <typename T> class A { public: T a; };], [])],
	[], [C++], [cs_cv_cxx_typename=yes], [cs_cv_cxx_typename=no])])
AS_IF([test $cs_cv_cxx_typename != no],
    [CS_HEADER_PROPERTY([CS_HAVE_CXX_KEYWORD_TYPENAME])])



#------------------------------------------------------------------------------
# Check if C++ compiler understands new explicit template specialization
# syntax `template<> class Foo<Bar>'.  Older compiler allows specializations
# without the `template<>' prefix.
#------------------------------------------------------------------------------
AC_CACHE_CHECK([for C++ explicit template<> specialization],
    [cs_cv_cxx_explicit_template_specialization],
    [CS_BUILD_IFELSE(
	[AC_LANG_PROGRAM(
	    [template <class T> class A {}; template<> class A<int> {};], [])],
	[], [C++],
	[cs_cv_cxx_explicit_template_specialization=yes],
	[cs_cv_cxx_explicit_template_specialization=no])])

AS_IF([test $cs_cv_cxx_explicit_template_specialization = no],
    [CS_HEADER_PROPERTY([CS_USE_OLD_TEMPLATE_SPECIALIZATION])])



#------------------------------------------------------------------------------
# CS_CHECK_QSQRT
#	Check if compiler is able to understand the assembly code in csqsqrt.h.
#	The MacOS/X PowerPC version of csQsqrt() requires the
#	-force_cpusubtype_ALL flag, so we use that flag when available.  Sets
#	the shell variable cs_cv_cxx_qsqrt_ok to "yes" if csqsqrt.h is usable,
#	otherwise "no".
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_QSQRT],
    [AC_REQUIRE([CS_CHECK_HOST_CPU])
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([CS_CHECK_FORCE_CPUSUBTYPE_ALL])
    AC_CACHE_CHECK([if assembly-coded csQsqrt() compiles],
	[cs_cv_cxx_qsqrt_ok],
	[AS_IF([test $ac_compiler_gnu = yes],
	    [CS_BUILD_IFELSE(
		[AC_LANG_PROGRAM(
		    [[#define CS_PROCESSOR_$cs_host_cpu_normalized
		    #define CS_COMPILER_GCC
		    #define __CS_CSSYSDEF_H__
		    #include <csqsqrt.h>]],
		    [float n = csQsqrt(1.0f); (void)n;])],
		[CS_CREATE_TUPLE([-I$srcdir/include])], [C++],
		[cs_cv_cxx_qsqrt_ok=yes], [cs_cv_cxx_qsqrt_ok=no],
		[$cs_cv_prog_cxx_force_cpusubtype_all])],
	    [cs_cv_cxx_qsqrt_ok=no])])])

CS_CHECK_QSQRT
AS_IF([test $cs_cv_cxx_qsqrt_ok = no],
    [CS_HEADER_PROPERTY([CS_NO_QSQRT])])



#------------------------------------------------------------------------------
# Check how to determine whether a floating point value is finite.
#------------------------------------------------------------------------------
# CS_CHECK_FINITE(FUNCTION)
AC_DEFUN([CS_CHECK_FINITE],
    [CS_CHECK_BUILD([for $1], [cs_cv_func_$1],
	[AC_LANG_PROGRAM(
	    [[#if HAVE_CMATH
	    #include <cmath>
	    #endif
	    #if HAVE_MATH_H
	    #include <math.h>
	    #endif]],
	    [float f; $1(f)])],
	[], [C++], [CS_HEADER_PROPERTY(AS_TR_CPP([CS_HAVE_$1]))], [], [],
	[$cs_cv_prog_cxx_enable_pedantic])])

CS_CHECK_BUILD([for std::isfinite], [cs_cv_func_std__isfinite],
    [AC_LANG_PROGRAM(
	[[#if HAVE_CMATH
	#include <cmath>
	#endif
	#if HAVE_MATH_H
	#include <math.h>
	#endif]],
	[float f; std::isfinite(f)])],
    [], [C++], [CS_HEADER_PROPERTY(AS_TR_CPP([CS_HAVE_STD__ISFINITE]))], [], [],
    [$cs_cv_prog_cxx_enable_pedantic])
CS_CHECK_FINITE([isfinite])
CS_CHECK_FINITE([finitef])
CS_CHECK_FINITE([finite])
CS_CHECK_FINITE([_finite])



#------------------------------------------------------------------------------
# Determine if plugin meta-information should be embedded directly into plugin
# modules or if it should exist in stand-alone .csplugin files.
#------------------------------------------------------------------------------
CS_META_INFO_EMBED([emit])
AS_IF([test $enable_meta_info_embedding = yes],
    [CS_HEADER_PROPERTY([CS_EMBED_PLUGIN_META])
    CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS],
	[AS_ESCAPE([$(EMBED_META.CFLAGS)])], [+])
    CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS],
	[AS_ESCAPE([$(EMBED_META.LFLAGS)])], [+])
    AC_CHECK_HEADERS([elf.h], [CS_HEADER_PROPERTY([CS_HAVE_ELF_H])])
    AS_IF([test "$cs_cv_libbfd_ok" = yes],
	[CS_HEADER_PROPERTY([CS_HAVE_LIBBFD])])])



#------------------------------------------------------------------------------
# Determine build mode; either "optimize", "debug", or "profile".
#------------------------------------------------------------------------------
AC_ARG_ENABLE([optimize], [AC_HELP_STRING([--enable-optimize],
    [build with optimizations enabled (default YES);
	same as --enable-mode=optimize])],
    [test "$enableval" = "yes" && cs_build_mode=optimize])

AC_ARG_ENABLE([debug], [AC_HELP_STRING([--enable-debug],
    [build with debugging information (default NO);
	same as --enable-mode=debug])],
    [test "$enableval" = "yes" && cs_build_mode=debug])

AC_ARG_ENABLE([profile], [AC_HELP_STRING([--enable-profile],
    [build with profiling information (default NO);
	same as --enable-mode=profile])],
    [test "$enableval" = "yes" && cs_build_mode=profile])

AC_ARG_ENABLE([mode], [AC_HELP_STRING([--enable-mode=mode],
    [set build mode; recognized modes are `optimize', `debug', `profile'
	(default OPTIMIZE)])],
    [case $enableval in
	optimize|debug|profile) cs_build_mode=$enableval ;;
	*) CS_MSG_ERROR([m4_text_wrap(
	    [unrecognized mode --enable-mode=$enableval;
	    use `optimize', `debug', or `profile'], [    ], [[]], [60])]) ;;
    esac])

AC_MSG_CHECKING([build mode])
AS_VAR_SET_IF([cs_build_mode], [], [cs_build_mode=optimize])
AC_MSG_RESULT([$cs_build_mode])
CS_EMIT_BUILD_PROPERTY([MODE], [$cs_build_mode])



#------------------------------------------------------------------------------
# Check for libbfd in debug mode on Windows so that CS_ASSERT() can dump a
# meaningful traceback in addition to emitting the failed condition. (On Linux,
# backtrace() is used instead.)
#
# ***WARNING***
#
# libbfd caries a GPL license which is incompatible with Crystal Space's LGPL
# license. We only enable this feature in debug mode as a diagnostic aid during
# the development cycle. When a project is ready for release, it should be
# built in optimize mode.
#------------------------------------------------------------------------------
AS_IF([test $cs_build_mode = debug],
    [case $host_os in
	mingw*|cygwin*)
	    CS_CHECK_LIBBFD([emit],
		[CS_HEADER_PROPERTY([CS_HAVE_LIBBFD])
		cs_debug_libbfd=yes],
		[cs_debug_libbfd=no])
	    ;;
	*) cs_debug_libbfd=no ;;
    esac],
    [cs_debug_libbfd=no])



#------------------------------------------------------------------------------
# Determine whether ptmalloc should be disabled.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([whether to enable ptmalloc])
AC_ARG_ENABLE([ptmalloc],
    [AC_HELP_STRING([--enable-ptmalloc],
	[enable the built-in ptmalloc which provides fast memory allocations,
	but may cause problems in rare cases. (default YES)])],
    [], [enable_ptmalloc=yes])
AC_MSG_RESULT([$enable_ptmalloc])
AS_IF([test $enable_ptmalloc != yes],
    [CS_HEADER_PROPERTY([CS_NO_PTMALLOC])])



#------------------------------------------------------------------------------
# Determine if extensive memory debugger should be enabled.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([whether to use extensive memory debugger])
AC_ARG_ENABLE([extensive-memory-debugger],
    [AC_HELP_STRING([--enable-extensive-memory-debugger],
	[enable the extensive memory debugger which catches common memory
	management mistakes (default NO)])],
    [], [enable_extensive_memory_debugger=no])
AC_MSG_RESULT([$enable_extensive_memory_debugger])
CS_EMIT_BUILD_PROPERTY([EXTENSIVE_MEMDEBUG],
    [$enable_extensive_memory_debugger])
AS_IF([test $enable_extensive_memory_debugger = yes],
    [CS_HEADER_PROPERTY([CS_EXTENSIVE_MEMDEBUG])])



#------------------------------------------------------------------------------
# Determine if memory tracker should be enabled.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([whether to use memory tracker])
AC_ARG_ENABLE([memory-tracker],
    [AC_HELP_STRING([--enable-memory-tracker],
	[enable the memory tracker which checks memory usage per
	source file (default NO)])],
    [], [enable_memory_tracker=no])
AC_MSG_RESULT([$enable_memory_tracker])
CS_EMIT_BUILD_PROPERTY([MEMORY_TRACKER],
    [$enable_memory_tracker])
AS_IF([test $enable_memory_tracker = yes],
    [CS_HEADER_PROPERTY([CS_MEMORY_TRACKER])])



#------------------------------------------------------------------------------
# Determine if reference tracker should be enabled.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([whether to use reference tracker])
AC_ARG_ENABLE([ref-tracker],
    [AC_HELP_STRING([--enable-ref-tracker],
        [enable the reference tracker which dumps information 
        about leaked references (default NO)])],
    [], [enable_ref_tracker=no])
AC_MSG_RESULT([$enable_ref_tracker])
CS_EMIT_BUILD_PROPERTY([REF_TRACKER],
    [$enable_ref_tracker])
AS_IF([test $enable_ref_tracker = yes],
    [CS_HEADER_PROPERTY([CS_REF_TRACKER])])



#------------------------------------------------------------------------------
# Determine if leak guard should be enabled.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([whether to use leak guard])
AC_ARG_ENABLE([leak-guard],
    [AC_HELP_STRING([--enable-leak-guard],
	[enable the leak guard protection which warns about objects not freed
	at program exit (default NO)])],
    [], [enable_leak_guard=no])
AC_MSG_RESULT([$enable_leak_guard])
AS_IF([test $enable_leak_guard = yes],
    [CS_HEADER_PROPERTY([CS_USE_LEAK_GUARD])])



#------------------------------------------------------------------------------
# Determine whether link libraries should be static or shared.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([whether to build shared libraries])
AC_ARG_ENABLE([shared], [AC_HELP_STRING([--enable-shared],
    [enable creation of shared libraries (_EXPERIMENTAL_!) (default NO)])],
    [], [enable_shared=no])
AC_MSG_RESULT([$enable_shared])
CS_EMIT_BUILD_PROPERTY([BUILD_SHARED_LIBS], [$enable_shared])
AS_IF([test $enable_shared = yes], 
  [CS_HEADER_PROPERTY([CS_BUILD_SHARED_LIBS])])



#------------------------------------------------------------------------------
# Determine if the "make" emulation layer should be synthesized.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([whether to emulate makefiles])
AC_ARG_ENABLE([make-emulation],
    [AC_HELP_STRING([--enable-make-emulation],
	[enable emulation layer allowing project to be built with `make', in
	addition to `jam' (default YES)])],
    [], [enable_make_emulation=yes])
AC_MSG_RESULT([$enable_make_emulation])



#------------------------------------------------------------------------------
# Boilerplate: csconfig.h
#------------------------------------------------------------------------------
CS_HEADER_PREPEND(
[// File generated automatically.
#ifndef __CS_CSCONFIG_H__
#define __CS_CSCONFIG_H__
])

CS_HEADER_PROPERTY([CS_PACKAGE_NAME],
    [AS_ESCAPE(["])$PACKAGE_NAME[]AS_ESCAPE(["])])
CS_HEADER_PROPERTY([CS_PACKAGE_VERSION],
    [AS_ESCAPE(["])$PACKAGE_VERSION[]AS_ESCAPE(["])])
CS_HEADER_APPEND(
[#endif // __CS_CSCONFIG_H__
])



#------------------------------------------------------------------------------
# Boilerplate: Jamconfig
#------------------------------------------------------------------------------
CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS],
    [AS_ESCAPE([$(COMPILER.CFLAGS.MANDATORY)])], [+])
CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS],
    [AS_ESCAPE([$(COMPILER.C++FLAGS.MANDATORY)])], [+])
CS_EMIT_PACKAGEINFO
CS_OUTPUT_INSTALLDIRS



#------------------------------------------------------------------------------
# Output generated files.
#------------------------------------------------------------------------------
CS_HEADER_OUTPUT([include/csconfig.h])
CS_JAMCONFIG_OUTPUT([Jamconfig])
AC_CONFIG_FILES([Jamfile])



#------------------------------------------------------------------------------
# In order to avoid alienating users by forcing them to install and use Jam, we
# perform the following actions:
#
# (1) If the user does not have Jam installed, we build it on his behalf from a
#     copy of the Jam sources residing in $srcdir/apps/tools/jam, and place the
#     executable in the root of the build directory.
#
# (2) Synthesize a makefile which implements all of top-level user-visible
#     targets provided by the Jam system.  The makefile simply forwards these
#     target invocations over to Jam.
#------------------------------------------------------------------------------
# (1)
AC_DEFUN([CS_BUILD_LOCAL_JAM],
    [rm -rf $2
    AS_MKDIR_P([$2])
    for jamsrc in $1/*; do
	AS_IF([test -f $jamsrc], [cp $jamsrc $2])
    done
    # Yacc might not be installed, so ensure that Jam does not try to use it.
    YACC=
    export YACC
    rm -f $3
    AS_IF([AC_RUN_LOG([cd $2 &&
	    $MAKE CC="$CC" CFLAGS="$JAM_CFLAGS" EXENAME=$JAM_EXE0NAME 1>&2])],
	[cs_built_jam=`echo $2/bin.*/jam$EXEEXT`
	AS_IF([test -f $cs_built_jam],
	    [cp $cs_built_jam $3],
	    [AC_MSG_ERROR([failed to build jam; please install manually])])],
	[AC_MSG_ERROR([failed to build jam; please install manually])])
    rm -rf $2])

AC_DEFUN([CS_JAM_CFLAGS],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
	mingw*) $1="$$1 -DMSYS" ;;
	cygwin*) $1="$$1 -D__cygwin__" ;;
    esac
    $1=CS_TRIM([$$1])])

AS_IF([test "$CS_NEED_LOCAL_JAM" = yes && test $enable_make_emulation = yes],
    [CS_JAM_CFLAGS([JAM_CFLAGS])
    AC_CONFIG_COMMANDS([cs_jam_local_exe],
	[CS_BUILD_LOCAL_JAM([$srcdir/apps/tools/jam], [jambuild],
	    [cs_jam_local_exe])],
	[MAKE=$MAKE
	EXEEXT=$EXEEXT
	CC="$CC"
	JAM_CFLAGS="$JAM_CFLAGS"
	JAM_EXE0NAME=./jam0$EXEEXT])])

# (2)
AC_DEFUN([CS_SYNTHESIZE_MAKEFILE],
    [AS_IF([AC_RUN_LOG(
	    ["$JAM" -sDUMPTARGETS_FILE=$tmp/jcache dumptargets 1>&2])],
	[jam_targets=all
	while read jam_line; do
	    jam_targets="$jam_targets $jam_line"
	done < $tmp/jcache
	cat << 'EOF' >$1
# Auto-generated; do not modify.
TO_NULL = >/dev/null 2>&1
RUN_JAM = \
  silent='-d0'; \
  loud='-dx'; \
  $(JAM) -h | grep '(x)commands' $(TO_NULL) || loud='-d+2'; \
  makeflags=`echo $$MAKEFLAGS | \
    sed 's/--[[^ ]][[^ ]]*//g;s/[[^ ]][[^ ]]*=.*//g'`; \
  jamflags=''; \
  echo "x$$makeflags" | grep k $(TO_NULL) || jamflags="-q $$jamflags"; \
  echo "x$$makeflags" | grep s $(TO_NULL) || jamflags="$$loud $$jamflags"; \
  echo "x$$makeflags" | grep s $(TO_NULL) && jamflags="$$silent $$jamflags"; \
  $(JAM) $$jamflags
EOF
	echo "JAM = $JAM" >>$1
	for jam_target in $jam_targets; do
	    echo ".PHONY: $jam_target" >>$1
	    echo "$jam_target:" >>$1
	    case $jam_target in
	        help) echo '	@$(JAM)'" $jam_target" >>$1 ;;
	        *) echo '	@$(RUN_JAM)'" $jam_target" >>$1 ;;
	    esac
	done],
	[AC_MSG_WARN([failed to create makefile emulation layer])])
    rm -f $tmp/jcache])

AS_IF([test $enable_make_emulation = yes],
    [AC_CONFIG_COMMANDS([Makefile], [CS_SYNTHESIZE_MAKEFILE([Makefile])],
	[JAM=$JAM])])



#------------------------------------------------------------------------------
# Finalization.
#------------------------------------------------------------------------------
AC_OUTPUT



#------------------------------------------------------------------------------
# In case libbfd is used for debugging, emit a message notifying developers
# that libbfd is GPL, along with its inherent consequences.
#------------------------------------------------------------------------------
AS_IF([test $cs_debug_libbfd = yes],
    [AC_MSG_NOTICE([
***
*** This debug build is linked against libbfd, a GPL-licensed library.
***
libbfd is used for non-critical development and debugging aids and is thus, by
default, enabled in debug, but not optimize or release builds.  Be aware of
licensing issues from linking against a GPL library: usually, for development
work this is not problematic, however, you should not ship this build or any
binary statically linked against a Crystal Space library if you are not using a
GPL-compatible license.  If you really must ship a debug build of Crystal Space
(as opposed to a release build), you can avoid licensing issues by disabling
use of libbfd with the --without-bfd configure option.
])])
