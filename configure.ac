#                                               -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003-2005 by Eric Sunshine <sunshine@sunshineco.com>
#
# Autoconf input script for Crystal Space.  Process this file with
# CS/bin/autogen.sh to produce a configure script.
#==============================================================================
AC_PREREQ([2.56])


#------------------------------------------------------------------------------
# cs_version_number
#	Keep this value synchronized with CS/include/csver.h.
#------------------------------------------------------------------------------
m4_define([cs_version_number], [0.99])



#==============================================================================
# INITIALIZATION
#==============================================================================
AC_INIT([crystalspace], [cs_version_number],
        [crystal-main@lists.sourceforge.net])
CS_PACKAGEINFO([Crystal Space],
    [Copyright (C)1998-2004 Jorrit Tyberghein and others],
    [http://www.crystalspace3d.org/])
AC_CONFIG_SRCDIR([libs/csutil/Jamfile])
AC_CONFIG_AUX_DIR([mk/autoconf])
AC_CANONICAL_HOST



#------------------------------------------------------------------------------
# Check for compilers and linker.
#
# IMPLEMENTATION NOTES
#
# Note that AC_PROG_CC and AC_PROC_CXX insist upon adding both optimization
# (-O2) and debug (-g) flags to CFLAGS and CXXFLAGS if these variables have not
# been set explicitly. Unfortunately, this is not suitable for Crystal Space's
# build system which makes a distinction between `optimize' and `debug' builds.
# To work around this problem, we ensure that both of these variables are set
# (either with empty values or with a user-supplied values) in order to
# override Autoconf's undesirable behavior.
#------------------------------------------------------------------------------

CFLAGS="$CFLAGS" # See above note.
AC_PROG_CC
CS_EMIT_BUILD_PROPERTY([CMD.CC], [$CC])

CXXFLAGS="$CXXFLAGS" # See above note.
AC_PROG_CXX
CS_EMIT_BUILD_PROPERTY([CMD.C++], [$CXX])

# C++ compiler is used as linker.
CS_EMIT_BUILD_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.C++)])])

AS_IF([test $ac_compiler_gnu = yes],
    [cs_compiler_name=GCC CS_HEADER_PROPERTY([CS_COMPILER_GCC])],
    [cs_compiler_name="AS_TR_CPP([$CXX])"])
CS_EMIT_BUILD_PROPERTY([COMPILER.TYPE], [$cs_compiler_name])
CS_HEADER_PROPERTY([CS_COMPILER_NAME],
    [AS_ESCAPE(["])$cs_compiler_name[]AS_ESCAPE(["])])



#------------------------------------------------------------------------------
# Check for common tools.
#------------------------------------------------------------------------------

# The default RANLIB in Jambase is wrong on some platforms, and is also
# unsuitable during cross-compilation, so we set the value unconditionally
# (sixth argument of CS_EMIT_BUILD_PROPERTY).
AC_PROG_RANLIB
CS_EMIT_BUILD_PROPERTY([RANLIB], [$RANLIB], [], [], [], [Y])

CS_CHECK_TOOLS([DLLTOOL], [dlltool])
CS_EMIT_BUILD_PROPERTY([CMD.DLLTOOL], [$DLLTOOL])

CS_CHECK_TOOLS([DLLWRAP], [dllwrap])
CS_EMIT_BUILD_PROPERTY([CMD.DLLWRAP], [$DLLWRAP])

CS_CHECK_TOOLS([WINDRES], [windres])
CS_EMIT_BUILD_PROPERTY([CMD.WINDRES], [$WINDRES])

CS_CHECK_TOOLS([OBJCOPY], [objcopy])
AS_IF([test -n "$OBJCOPY"],
    [CS_EMIT_BUILD_PROPERTY([OBJCOPY.AVAILABLE], [yes])
    CS_EMIT_BUILD_PROPERTY([CMD.OBJCOPY], [$OBJCOPY])])

CS_CHECK_PROGS([FLEX], [flex])
CS_EMIT_BUILD_PROPERTY([CMD.FLEX], [$FLEX])

CS_CHECK_PROGS([BISON], [bison])
AS_IF([test -n "$BISON"],
    [CS_CHECK_PROG_VERSION([bison], [$BISON --version], [1.35], [9.9],
	[CS_EMIT_BUILD_PROPERTY([CMD.BISON], [$BISON])])])

CS_CHECK_PROGS([DOXYGEN], [doxygen])
CS_EMIT_BUILD_PROPERTY([CMD.DOXYGEN], [$DOXYGEN])

CS_CHECK_PROGS([TEXI2DVI], [texi2dvi])
CS_EMIT_BUILD_PROPERTY([CMD.TEXI2DVI], [$TEXI2DVI])

CS_CHECK_PROGS([DVIPS], [dvips])
CS_EMIT_BUILD_PROPERTY([CMD.DVIPS], [$DVIPS])

CS_CHECK_PROGS([DVIPDF], [dvipdf])
CS_EMIT_BUILD_PROPERTY([CMD.DVIPDF], [$DVIPDF])

CS_CHECK_PROGS([MAKEINFO], [makeinfo])
CS_EMIT_BUILD_PROPERTY([CMD.MAKEINFO], [$MAKEINFO])

CS_CHECK_PROGS([INSTALL], [install])
CS_EMIT_BUILD_PROPERTY([INSTALL], [$INSTALL])

CS_CHECK_PROGS([SWIG], [swig])
AS_IF([test -n "$SWIG"],
    [CS_CHECK_PROG_VERSION([swig], [$SWIG -version 2>&1], [1.3.21],
	[9.9|.9], [CS_EMIT_BUILD_PROPERTY([CMD.SWIG], [$SWIG])])])

CS_CHECK_LIBTOOL
CS_EMIT_BUILD_PROPERTY([LIBTOOL], [$LIBTOOL])
CS_EMIT_BUILD_PROPERTY([APPLE_LIBTOOL], [$APPLE_LIBTOOL])

CS_CHECK_MKDIR
CS_EMIT_BUILD_PROPERTY([CMD.MKDIR], [$MKDIR])
CS_EMIT_BUILD_PROPERTY([CMD.MKDIRS], [$MKDIRS])



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
CS_CHECK_TOOLS([CSLIBS_CONFIG], [cslibs-config])
AS_IF([test $ac_compiler_gnu = yes],
    [cs_cv_cslibs_compiler="--compiler gcc-`$CXX -dumpversion`"])
AC_CACHE_CHECK([for cslibs package], [cs_cv_cslibs],
    [AS_IF([test -n "$CSLIBS_CONFIG"],
	[cs_cv_cslibs=yes
	cs_cv_cslibs_cflags=CS_RUN_PATH_NORMALIZE(
	    [$CSLIBS_CONFIG --cflags $cs_cv_cslibs_compiler])
	cs_cv_cslibs_lflags=CS_RUN_PATH_NORMALIZE(
	    [$CSLIBS_CONFIG --lflags $cs_cv_cslibs_compiler])
	cs_cv_cslibs_binpath=CS_RUN_PATH_NORMALIZE(
	    [$CSLIBS_CONFIG --binpath $cs_cv_cslibs_compiler])],
	[cs_cv_cslibs=no])])
AS_IF([test $cs_cv_cslibs = yes],
    [CFLAGS="$CFLAGS $cs_cv_cslibs_cflags"
    CPPFLAGS="$CPPFLAGS $cs_cv_cslibs_cflags"
    LDFLAGS="$LDFLAGS $cs_cv_cslibs_lflags"
    PATH="$cs_cv_cslibs_binpath$PATH_SEPARATOR$PATH"])



#------------------------------------------------------------------------------
# Use the CPPFLAGS, CXXFLAGS, CFLAGS, and LDFLAGS passed to configure, as well
# as any additional flags provided by cs-win32libs.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS], [$CPPFLAGS $CFLAGS], [+])
CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS], [$CPPFLAGS $CXXFLAGS], [+])
CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [$LDFLAGS], [+])



#------------------------------------------------------------------------------
# Check for NASM and extensions (such as %xdefine) which we require.
#------------------------------------------------------------------------------
CS_CHECK_TOOLS([NASM], [nasmw nasm])
AS_IF([test -n "$NASM"],
    [AC_CACHE_CHECK([if nasm is sufficiently recent], [cs_cv_prog_nasm_ok],
	[echo "%xdefine TEST" > conftest.asm
	AS_IF([AC_RUN_LOG(
	    [$NASM conftest.asm -o conftest.$OBJEXT 1>&2])],
	    [cs_cv_prog_nasm_ok=yes], [cs_cv_prog_nasm_ok=no])
	rm -f conftest.*])
    AS_IF([test $cs_cv_prog_nasm_ok = yes],
	[CS_EMIT_BUILD_PROPERTY([NASM.AVAILABLE], [yes])
	CS_EMIT_BUILD_PROPERTY([CMD.NASM], [$NASM])
	CS_HEADER_PROPERTY([CS_USE_NASM])])])



#------------------------------------------------------------------------------
# Check for typical required libraries (libm, libmx, libdl, libnsl).
#------------------------------------------------------------------------------
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([cmath], [CS_HEADER_PROPERTY([CS_HAS_CMATH_H])])
AC_LANG_POP([C++])

AC_DEFUN([CS_CHECK_COMMON_LIBS],
    [AC_LANG_PUSH([C])
    AC_CHECK_LIB([m], [pow], [cs_cv_libm_libs=-lm], [cs_cv_libm_libs=])
    AC_CHECK_LIB([m], [cosf], [cs_cv_libm_libs=-lm])
    AC_CHECK_LIB([mx], [cosf])
    AC_CHECK_LIB([dl], [dlopen], [cs_cv_libdl_libs=-ldl], [cs_cv_libdl_libs=])
    AC_CHECK_LIB([nsl], [gethostbyname])
    AC_LANG_POP([C])])

CS_CHECK_COMMON_LIBS

AS_IF([test $ac_cv_lib_m_pow = yes || test $ac_cv_lib_m_cosf = yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lm], [+])])
AS_IF([test $ac_cv_lib_mx_cosf = yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lmx], [+])])
AS_IF([test $ac_cv_lib_dl_dlopen = yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-ldl], [+])])
AS_IF([test $ac_cv_lib_nsl_gethostbyname = yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lnsl], [+])])

AS_IF([test $ac_cv_lib_m_cosf = no && test $ac_cv_lib_mx_cosf = no],
    [CS_HEADER_PROPERTY([CS_USE_FAKE_MATH_H_FLOAT_FUNCS])])



#------------------------------------------------------------------------------
# Check for pthread.  Also check if pthread implementation supports the
# recursive mutex extension.
#------------------------------------------------------------------------------
CS_CHECK_PTHREAD([cygwin*])
CS_EMIT_BUILD_RESULT([cs_cv_sys_pthread], [PTHREAD])
AS_IF([test $cs_cv_sys_pthread_mutex_recursive != no],
    [CS_HEADER_PROPERTY([CS_PTHREAD_MUTEX_RECURSIVE],
	[$cs_cv_sys_pthread_mutex_recursive])])
AS_IF([test $cs_cv_sys_pthread_mutex_timed != no],
    [CS_HEADER_PROPERTY([CS_PTHREAD_MUTEX_TIMED],
	[$cs_cv_sys_pthread_mutex_timed])])



#------------------------------------------------------------------------------
# Check if RTLD_NOW dlopen() flag is available; some installations (OpenBSD,
# for example) provide only RTLD_LAZY.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_RTLD],
    [AC_REQUIRE([CS_CHECK_COMMON_LIBS])
    CS_CHECK_BUILD([for RTLD_NOW], [cs_cv_sys_rtld_now],
	[AC_LANG_PROGRAM([[#include <dlfcn.h>]], [dlopen(0, RTLD_NOW)])],
	[], [], [CS_HEADER_PROPERTY([CS_RTLD_NOW_AVAILABLE])], [], [],
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
AC_DEFUN([CS_CHECK_HOST],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    cs_host_os_normalized=''
    case $host_os in
	mingw*|cygwin*)
	    cs_host_target=win32gcc
	    cs_host_family=windows
	    ;;
	darwin*)
	    _CS_CHECK_HOST_DARWIN
	    ;;
	*)
	    # Everything else is assumed to be Unix or Unix-like.
	    cs_host_target=unix
	    cs_host_family=unix
	    ;;
    esac])

AC_DEFUN([_CS_CHECK_HOST_DARWIN],
    [AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([AC_PROG_CXX])

    # Both MacOS/X and Darwin are identified via $host_os as "darwin".  We need
    # a way to distinguish between the two.  If Carbon.h is present, then
    # assume MacOX/S; if not, assume Darwin.  If --with-x=yes was invoked, and
    # Carbon.h is present, then assume that user wants to cross-build for
    # Darwin even though build host is MacOS/X.  Implementation note: At least
    # one MacOS/X user switches between gcc 2.95 and gcc 3.3 with a script
    # which toggles the values of CC, CXX, and CPP.  Unfortunately, CPP was
    # being set to run the preprocessor directly ("cpp", for instance) rather
    # than running it via the compiler ("gcc -E", for instance).  The problem
    # with running the preprocessor directly is that __APPLE__ and __GNUC__ are
    # not defined, which causes the Carbon.h check to fail.  We avoid this
    # problem by supplying a non-empty fourth argument to AC_CHECK_HEADER(),
    # which causes it to test compile the header only (which is a more robust
    # test), rather than also testing it via the preprocessor.

    AC_CHECK_HEADER([Carbon/Carbon.h],
	[cs_host_macosx=yes], [cs_host_macosx=no], [/* force compile */])

    AS_IF([test $cs_host_macosx = yes],
	[AC_MSG_CHECKING([for --with-x])
	AS_IF([test "${with_x+set}" = set && test "$with_x" = "yes"],
	    [AC_MSG_RESULT([yes (assume Darwin)])
	    cs_host_macosx=no],
	    [AC_MSG_RESULT([no])])])

    AS_IF([test $cs_host_macosx = yes],
	[cs_host_target=macosx
	cs_host_family=unix
	cs_host_os_normalized='MacOS/X'

	CS_HEADER_PROPERTY([CS_PLATFORM_MACOSX])
	CS_HEADER_PROPERTY([CS_24BIT_PIXEL_LAYOUT], [CS_24BIT_PIXEL_ARGB])

	# We assume that we always have CoreAudio available on MacOS/X.
	CS_EMIT_BUILD_PROPERTY([COREAUDIO.AVAILABLE], [yes])
	CS_EMIT_BUILD_PROPERTY([COREAUDIO.LFLAGS], [-framework CoreAudio])

	AC_CACHE_CHECK([for Objective-C compiler], [cs_cv_prog_objc],
	    [cs_cv_prog_objc="$CC"])
	CS_EMIT_BUILD_PROPERTY([CMD.OBJC], [$cs_cv_prog_objc])
	AC_CACHE_CHECK([for Objective-C++ compiler], [cs_cv_prog_objcxx],
	    [cs_cv_prog_objcxx="$CXX"])
	CS_EMIT_BUILD_PROPERTY([CMD.OBJC++], [$cs_cv_prog_objcxx])],

	[cs_host_target=unix
	cs_host_family=unix])])

CS_CHECK_HOST

case $cs_host_family in
    windows)
	CS_HEADER_PROPERTY([CS_PLATFORM_WIN32])
	AS_IF([test -z "$cs_host_os_normalized"],
	    [cs_host_os_normalized='Win32'])
	;;
    unix)
	CS_HEADER_PROPERTY([CS_PLATFORM_UNIX])
	AS_IF([test -z "$cs_host_os_normalized"],
	    [cs_host_os_normalized='Unix'])
	;;
esac

cs_host_os_normalized_uc="AS_TR_CPP([$cs_host_os_normalized])"
CS_EMIT_BUILD_PROPERTY([TARGET.OS], [$cs_host_os_normalized_uc])
CS_EMIT_BUILD_PROPERTY([TARGET.OS.NORMALIZED], [$cs_host_os_normalized])
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
	    [[#include <jni.h>
	    JNIEXPORT void JNICALL f(JNIEnv *env, jobject obj);]])],
	[CS_CREATE_TUPLE() \
        CS_CREATE_TUPLE([$cs_java_cflags],[$cs_java_lflags],[$cs_java_libs])],
	[], [CS_EMIT_BUILD_RESULT([cs_cv_java], [JAVA])])])

CS_PATH_PROGS([ANT], [ant], [], [$PATH$PATH_SEPARATOR$ANT_HOME/bin])
CS_EMIT_BUILD_PROPERTY([ANT], [$ANT])



#------------------------------------------------------------------------------
# Check how to enable compilation warnings.  Note that on some platforms, it is
# more appropriate to use -Wmost rather than -Wall even if the compiler
# understands both, thus we attempt -Wmost before -Wall.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to enable compilation warnings],
    [cs_cv_prog_cxx_enable_warnings],
    [CS_CREATE_TUPLE([-Wmost]) CS_CREATE_TUPLE([-Wall])], [C++],
    [COMPILER.CFLAGS], [+],
    [cs_prog_cxx_enable_warnings="$cs_cv_prog_cxx_enable_warnings"],
    [cs_prog_cxx_enable_warnings=''])



#------------------------------------------------------------------------------
# Check how to make compiler treat warnings as errors.
#------------------------------------------------------------------------------
CS_CHECK_BUILD_FLAGS([how to treat warnings as errors],
    [cs_cv_prog_cxx_enable_errors], [CS_CREATE_TUPLE([-Werror])], [C++],
    [cs_prog_cxx_enable_errors="$cs_cv_prog_cxx_enable_errors"],
    [cs_prog_cxx_enable_errors=''])



#------------------------------------------------------------------------------
# Check how to make compiler ignore unused variables and arguments.  This is
# needed for code generated by tools, such as Swig, Bison, and Flex, over which
# we have no control.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to ignore unused variables],
    [cs_cv_prog_cxx_ignore_unused_variables],
    [CS_CREATE_TUPLE([-Wno-unused])], [C++],
	[COMPILER.C++FLAGS.WARNING.NO_UNUSED])



#------------------------------------------------------------------------------
# Check how to make compiler ignore uninitialized variables and arguments.
# This is needed for code generated by tools, such as Swig, Bison, and Flex,
# over which we have no control.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to ignore uninitialized variables],
    [cs_cv_prog_cxx_ignore_uninitialized_variables],
    [CS_CREATE_TUPLE([-Wno-uninitialized])], [C++],
    [COMPILER.C++FLAGS.WARNING.NO_UNINITIALIZED])



#------------------------------------------------------------------------------
# Check if warnings about unknown #pragmas can be disabled.  (MSVC and Borland
# use a number of pragmas not understood by GCC, for instance.)
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([[how to ignore unknown #pragma directives]],
    [cs_cv_prog_cxx_ignore_unknown_pragmas],
    [CS_CREATE_TUPLE([-Wno-unknown-pragmas])], [C++], [COMPILER.CFLAGS], [+])



#------------------------------------------------------------------------------
# Check if C++ exceptions can be enabled and disabled.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to enable C++ exceptions],
    [cs_cv_prog_cxx_enable_exceptions], [CS_CREATE_TUPLE([-fexceptions])],
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
    [cs_cv_prog_cxx_enable_pedantic],
    [CS_CREATE_TUPLE([-ansi -pedantic])], [C++],
	[COMPILER.C++FLAGS.PEDANTIC.ENABLE])



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
AS_IF([test $cs_cv_prog_cxx_force_cpusubtype_all != no],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS.MANDATORY],
	[$cs_cv_prog_cxx_force_cpusubtype_all], [+])])



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
		[], [$cs_prog_cxx_enable_errors])
	    ;;
    esac])

CS_CHECK_VTABLE_THUNKS



#------------------------------------------------------------------------------
# Check if compiler recognizes -pipe directive.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([if -pipe is accepted], [cs_cv_prog_cxx_pipe],
   [CS_CREATE_TUPLE([-pipe])], [C++], [COMPILER.CFLAGS], [+])



#------------------------------------------------------------------------------
# Check if compiler/linker recognizes -shared directive which is needed for
# linking plugin modules.  Unfortunately, the Apple compiler (and possibly
# others) require extra effort.  Even though the compiler does not recognize
# the -shared option, it nevertheless returns a "success" result after
# complaining that the option is unrecognized.  Worse, even -Werror fails to
# coerce the compiler into returning a "failure" result, so we must perform the
# additional task of checking the compiler's output for an appropriate error
# emission, such as "unrecognized option `-shared'".
#------------------------------------------------------------------------------
CS_CHECK_BUILD_FLAGS([if -shared is accepted], [cs_cv_prog_link_shared],
    [CS_CREATE_TUPLE([-shared])], [C++],
    [CS_EMIT_BUILD_PROPERTY([PLUGIN.LFLAGS], [-shared], [+])], [],
    [$cs_prog_cxx_enable_errors], [], [], [nrecognize])



#------------------------------------------------------------------------------
# Check if linker recognizes -soname which is used to assign a name internally
# to plugin modules.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([if -soname is accepted], [cs_cv_prog_link_soname], [],
    [CS_CREATE_TUPLE([-Wl,-soname,foobar])], [C++],
    [CS_EMIT_BUILD_PROPERTY([PLUGIN.LFLAGS.USE_SONAME], [yes])])



#------------------------------------------------------------------------------
# Check how to ask the linker to export all dynamic symbols in debug mode to
# allow the backtrace() callstack creation code to obtain symbols in all cases.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to export all dynamic symbols],
    [cs_cv_prog_link_export_dynamic], [CS_CREATE_TUPLE([-Wl,-E])],
    [C++], [COMPILER.LFLAGS.debug], [+])



#------------------------------------------------------------------------------
# Check if linker recognizes --kill-at which is needed to make csjava.dll
# usable by JNI on Windows.  Without this, JNI is unable to find the exported
# JNI glue functions in csjava.dll.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([if --kill-at is accepted], [cs_cv_prog_link_kill_at], [],
    [CS_CREATE_TUPLE([-Wl,--kill-at])], [C++],
    [CS_EMIT_BUILD_PROPERTY([CSJAVA.LFLAGS], [-Wl,--kill-at])])



#------------------------------------------------------------------------------
# Check if compiler can be instructed to produce position-independent-code
# (PIC).  This feature is required by some platforms when building plugin
# modules and shared libraries.
#------------------------------------------------------------------------------
CS_CHECK_COMPILER_PIC([C++], [cs_cv_prog_cxx_pic],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS.PIC], [$cs_cv_prog_cxx_pic])])



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

    CS_EMIT_BUILD_FLAGS([if $CXX recognizes -fomit-frame-pointer],
	[cs_cv_prog_cxx_omit_frame_pointer],
	[CS_CREATE_TUPLE([-fomit-frame-pointer])],
	[C++], [COMPILER.CFLAGS.optimize], [+],
	[cs_optimize_flags="$cs_optimize_flags \
	    $cs_cv_prog_cxx_omit_frame_pointer"])

    CS_EMIT_BUILD_FLAGS([if $CXX recognizes -ffast-math],
	[cs_cv_prog_cxx_fast_math],
	[CS_CREATE_TUPLE([-ffast-math])], [C++],
	[COMPILER.CFLAGS.optimize], [+],
	[cs_optimize_flags="$cs_optimize_flags $cs_cv_prog_cxx_fast_math"])])

CS_CHECK_OPTIMIZE_FLAGS



#------------------------------------------------------------------------------
# Check for compiler debug flags.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to enable compiler debug output],
    [cs_cv_prog_cxx_debug],
    [CS_CREATE_TUPLE([-g3]) CS_CREATE_TUPLE([-g2]) CS_CREATE_TUPLE([-g])],
    [C++], [COMPILER.CFLAGS.debug], [+])
AS_IF([test $cs_cv_prog_cxx_debug != no],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS.debug], [$cs_cv_prog_cxx_debug],
	[+])])



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
	[cs_cv_prog_cxx_local_include],
	[CS_CREATE_TUPLE([-I/usr/local/include])], [C++],
	[cs_cv_prog_cxx_local_include=yes],
	[cs_cv_prog_cxx_local_include=no],
	[$cs_prog_cxx_enable_warnings $cs_prog_cxx_enable_errors])],
    [cs_cv_prog_cxx_local_include=no])

AS_IF([test $cs_cv_prog_cxx_local_include = yes],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS], [-I/usr/local/include], [+])
    CPPFLAGS="$CPPFLAGS -I/usr/local/include"])

AS_IF([test -d /usr/local/lib],
    [CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-L/usr/local/lib], [+])
    LDFLAGS="$LDFLAGS -L/usr/local/lib"
    LD_LIBRARY_PATH="$LD_LIBRARY_PATH$PATH_SEPARATOR/usr/local/lib"
    export LD_LIBRARY_PATH])



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
		[XtVaGetValues(0, 0, 0, 0)])], 
		[], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libXaw], [XAW])],
	[],
	[$X_CFLAGS], [], [$X_PRE_LIBS $X_LIBS -lX11 $X_EXTRA_LIBS])
    ])




#------------------------------------------------------------------------------
# Check for OpenGL and companions.  Also check if GLX extensions are supported
# and define the (backward) header property CSGL_EXT_STATIC_ASSERTION if they
# are _not_ supported. Finally, check if the type GLhandleARB is exported by
# the GL headers. It is starting to show up in very modern headers, but is
# absent from most.
#
# IMPLEMENTATION NOTES
#
# Some Mesa installations require pthread, so pthread flags are employed if
# available.
#
# The check for opengl32 needs to precede other checks because Cygwin users
# often have Mesa installed, and Mesa's OpenGL library is compiled without the
# __stdcall flags which results in link errors, whereas Microsoft's native
# opengl32 works fine.  Conversely, some Unix implementations have Wine
# installed (Windows emulation layer) which includes an opengl32.so library.
# We need to avoid detection of this library on Unix since it would cause an
# undesirable dependence upon Wine.
#
# Many OpenGL libraries on Unix already contain GLX, so there is no separate
# GLX library, thus we first check for GLX using the discovered OpenGL library
# before attempting to locate a separate GLX-specific library.  The same logic
# is also applied to GLU and GLUT checks.
#
# On MacOS/X, some users have XFree86 installed which creates a link from
# /usr/include/GL to /usr/X11R6/include/GL.  We want to ignore this directory
# and instead check for Apple's OpenGL.framework, if we are not cross-building
# for Darwin.  We accomplish this by placing the OpenGL.framework test ahead of
# the other tests.
#
# At least one user (Jorrit) has a strange installation in which inclusion of
# <windows.h> fails if an int32 is not present, thus we must take this into
# account.
#------------------------------------------------------------------------------
m4_define([cs_define_int32],
    [[#if !HAVE_TYPE_INT32
    typedef long int32;
    #endif
    ]])

AC_DEFUN([CS_CHECK_GL_PREPARE],
    [AC_REQUIRE([CS_CHECK_HOST])
    AC_REQUIRE([CS_CHECK_COMMON_LIBS])
    AC_REQUIRE([CS_CHECK_PTHREAD])
    AC_REQUIRE([AC_PATH_X])
    AC_REQUIRE([AC_PATH_XTRA])
    AC_CHECK_TYPE([int32], [AC_DEFINE([HAVE_TYPE_INT32])], [])
    AC_CHECK_HEADERS([windows.h], [], [], [cs_define_int32])])

# CS_GL_INCLUDE(CPP-MACRO,FALLBACK,HEADER)
AC_DEFUN([CS_GL_INCLUDE],
    [AC_REQUIRE([CS_CHECK_GL_PREPARE])
    [#if HAVE_WINDOWS_H
    #if !HAVE_TYPE_INT32
    typedef long int32;
    #endif
    #include <windows.h>
    #endif
    #ifndef CS_HEADER_GLOBAL
    #define CS_HEADER_GLOBAL(X,Y) CS_HEADER_GLOBAL_COMPOSE(X,Y)
    #define CS_HEADER_GLOBAL_COMPOSE(X,Y) <X/Y>
    #endif
    #ifdef $1
    #include CS_HEADER_GLOBAL($1,$3)
    #else
    #include <$2/$3>
    #endif]])

CS_CHECK_GL_PREPARE

# Apply plaform-specific flags if necessary.
cs_gl_plat_cflags=''
cs_gl_plat_lflags=''
cs_gl_plat_libs=''
AS_IF([test -n "$cs_cv_libm_cflags$cs_cv_libm_lflags$cs_cv_libm_libs"],
    [cs_gl_plat_cflags="$cs_cv_libm_cflags $cs_gl_plat_cflags"
    cs_gl_plat_lflags="$cs_cv_libm_lflags $cs_gl_plat_lflags"
    cs_gl_plat_libs="$cs_cv_libm_libs $cs_gl_plat_libs"])
AS_IF([test $cs_cv_sys_pthread = yes],
    [cs_gl_plat_cflags="$cs_cv_sys_pthread_cflags $cs_gl_plat_cflags"
    cs_gl_plat_lflags="$cs_cv_sys_pthread_lflags $cs_gl_plat_lflags"
    cs_gl_plat_libs="$cs_cv_sys_pthread_libs $cs_gl_plat_libs"])
AS_IF([test "$no_x" != yes],
    [cs_gl_plat_cflags="$X_CFLAGS $cs_gl_plat_cflags"
    cs_gl_plat_lflags="$cs_gl_plat_lflags"
    cs_gl_plat_libs="
	$X_PRE_LIBS $X_LIBS -lX11 -lXext $X_EXTRA_LIBS $cs_gl_plat_libs"])

# Mesa requested?
AC_ARG_WITH([mesa], [AC_HELP_STRING([--with-mesa],
	[use Mesa OpenGL library if available (default YES)])],
	[], [with_mesa=yes])

AS_IF([test $with_mesa != no],
    [cs_mesa_gl=CS_CREATE_TUPLE([],[],[-lMesaGL])
    cs_mesa_glu=CS_CREATE_TUPLE([],[],[-lMesaGLU])
    cs_mesa_glx=CS_CREATE_TUPLE([],[],[-lMesaGLX])])

# MacOS/X or Darwin?
AS_IF([test "x$cs_host_macosx" = "xyes"],
    [cs_osx_gl=CS_CREATE_TUPLE([-DCS_OPENGL_PATH=OpenGL],[],
	[-framework OpenGL])
    cs_osx_glu=CS_CREATE_TUPLE([-DCS_GLU_PATH=OpenGL],[],[-framework OpenGL])
    cs_osx_glut=CS_CREATE_TUPLE([-DCS_GLUT_PATH=GLUT],[],[-framework GLUT])])

# Windows?
case $host_os in
    mingw*|cygwin*)
	cs_win32_gl=CS_CREATE_TUPLE([],[],[-lopengl32])
	cs_win32_glu=CS_CREATE_TUPLE([],[],[-lglu32])
	cs_win32_glut=CS_CREATE_TUPLE([],[],[-lglut32])
    ;;
esac

# Check for OpenGL.
CS_CHECK_BUILD([for OpenGL], [cs_cv_libgl],
    [AC_LANG_PROGRAM([CS_GL_INCLUDE([CS_OPENGL_PATH],[GL],[gl.h])],[glEnd()])],
    [$cs_win32_gl \
    $cs_osx_gl \
    CS_CREATE_TUPLE([],[],[-lGL]) \
    CS_CREATE_TUPLE([],[],[-lgl]) \
    $cs_mesa_gl], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libgl], [GL])], [], [],
    [$cs_gl_plat_cflags], [$cs_gl_plat_lflags], [$cs_gl_plat_libs])

AS_IF([test $cs_cv_libgl = yes],
    [# Check for GLhandleARB.
    CS_CHECK_BUILD([for GLhandleARB], [cs_cv_libgl_glhandlearb],
	[AC_LANG_PROGRAM(
	    [CS_GL_INCLUDE([CS_OPENGL_PATH],[GL],[gl.h])],
	    [GLhandleARB x; (void)x;])],
	[], [],	[CS_HEADER_PROPERTY([CS_HAS_GLHANDLEARB_T])], [], [],
	[$cs_cv_libgl_cflags], [$cs_cv_libgl_lflags], [$cs_cv_libgl_libs])

    # Check for GLU.
    CS_CHECK_BUILD([for GLU], [cs_cv_libglu],
	[AC_LANG_PROGRAM(
	    [CS_GL_INCLUDE([CS_GLU_PATH],[GL],[glu.h])], [gluNewQuadric()])],
	[$cs_osx_glu \
	CS_CREATE_TUPLE() \
	$cs_win32_glu \
	CS_CREATE_TUPLE([],[],[-lGLU]) \
	CS_CREATE_TUPLE([],[],[-lglu]) \
	$cs_mesa_glu], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libglu], [GLU])], [], [],
	[$cs_cv_libgl_cflags], [$cs_cv_libgl_lflags], [$cs_cv_libgl_libs])

    # Check for GLUT.
    CS_CHECK_BUILD([for GLUT], [cs_cv_libglut],
	[AC_LANG_PROGRAM(
	    [CS_GL_INCLUDE([CS_GLUT_PATH],[GL],[glut.h])], [glutSwapBuffers])],
	[$cs_osx_glut \
	CS_CREATE_TUPLE() \
	$cs_win32_glut \
	CS_CREATE_TUPLE([],[],[-lGLUT]) \
	CS_CREATE_TUPLE([],[],[-lglut])], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libglut], [GLUT])], [], [],
	[$cs_cv_libgl_cflags], [$cs_cv_libgl_lflags], [$cs_cv_libgl_libs])

    # Check for GLX.
    CS_CHECK_BUILD([for GLX], [cs_cv_libglx],
	[AC_LANG_PROGRAM([[#include <GL/glx.h>]], [glXWaitGL()])],
	[CS_CREATE_TUPLE() \
	CS_CREATE_TUPLE([],[],[-lGLX]) \
	CS_CREATE_TUPLE([],[],[-lglx]) \
	$cs_mesa_glx], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libglx], [GLX])], [], [],
	[$cs_cv_libgl_cflags], [$cs_cv_libgl_lflags], [$cs_cv_libgl_libs])

    # Check for GLX extensions.  Define CSGL_EXT_STATIC_ASSERTION if _not_
    # present.
    AS_IF([test $cs_cv_libglx = yes],
	[CS_CHECK_BUILD([for GLX extensions], [cs_cv_libglx_extensions],
	    [AC_LANG_PROGRAM(
		[[#define GLX_GLXEXT_PROTOTYPES
		#include <GL/glx.h>]],
		[glXGetProcAddressARB(0)])],
	    [CS_CREATE_TUPLE(
		[$cs_cv_libglx_cflags],
		[$cs_cv_libglx_lflags],
		[$cs_cv_libglx_libs])],
	    [], [], [CS_HEADER_PROPERTY([CSGL_EXT_STATIC_ASSERTION])])])])



#------------------------------------------------------------------------------
# Check for DirectDraw.  At least one user (Jorrit) has a strange installation
# in which inclusion of <windows.h> fails if an int32 is not present, thus we
# must take this into account.  With Cygwin, we must check for the DirectX
# headers in /usr/include/directx.
#------------------------------------------------------------------------------
AC_CHECK_TYPE([int32], [AC_DEFINE([HAVE_TYPE_INT32])], [])
CS_CHECK_BUILD([for DirectX], [cs_cv_libdirectx],
    [AC_LANG_PROGRAM(
	[[#if !HAVE_TYPE_INT32
	typedef long int32;
	#endif
	#include <windows.h>
	#include <ddraw.h>
	#include <dsound.h>
	#include <dinput.h>]],
	[DirectDrawCreate(0,0,0); 
	 DirectSoundCreate(0,0,0); 
	 DirectInputCreate(0,0,0,0)])],
    [CS_CREATE_TUPLE([],[],[-lddraw -ldsound -ldinput]) \
    CS_CREATE_TUPLE([-I/usr/include/directx],[],[-lddraw -ldsound -ldinput])], 
    [], [CS_EMIT_BUILD_RESULT([cs_cv_libdirectx], [DIRECTX])])



#------------------------------------------------------------------------------
# Check for the Windows Multimedia library, used by the 'sndwaveout' plugin.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([for waveout], [cs_cv_libwaveout],
    [AC_LANG_PROGRAM(
	[[#include <windows.h>
	#include <mmsystem.h>]],
	[waveOutOpen(0, 0, 0, 0, 0, 0)])],
    [CS_CREATE_TUPLE([],[],[-lwinmm])], 
    [], [CS_EMIT_BUILD_RESULT([cs_cv_libwaveout], [WAVEOUT])])



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
    [lib3ds-120])

AS_IF([test $cs_cv_lib3ds = yes],
    [CS_CHECK_BUILD([if lib3ds is sufficiently recent], [cs_cv_lib3ds_recent],
	[AC_LANG_PROGRAM([[#include <lib3ds/io.h>]], [lib3ds_io_free(0);])],
	[], [], [CS_EMIT_BUILD_RESULT([cs_cv_lib3ds], [3DS])], [], [],
	[$cs_cv_lib3ds_cflags], [$cs_cv_lib3ds_lflags], [$cs_cv_lib3ds_libs])])



#------------------------------------------------------------------------------
# Check for ODE.  In addition to the standard installed locations, we also
# search CS/libs/libode*.  Also check if it is a sufficiently recent version.
# Finally, the cs-win32libs package uses a slightly augmented libode in which
# functions are exported as `extern "C"' to avoid name mangling problems which
# would otherwise prevent the package from being used with different compilers.
# Therefore, we need to check if `extern "C"' is needed.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([ode],
    [AC_LANG_PROGRAM([[#include <ode/ode.h>]], [dWorldCreate()])],
    [$srcdir/libs/libode*], [C++])

AS_IF([test $cs_cv_libode = yes],
    [CS_CHECK_BUILD([if libode is sufficiently recent], [cs_cv_libode_recent],
	[AC_LANG_PROGRAM(
	    [[#include <ode/ode.h>]],
	    [dWorldID w = dWorldCreate();
	    dWorldQuickStep(w,0.0f);])],
	[], [C++], [], [], [],
	[$cs_cv_libode_cflags], [$cs_cv_libode_lflags], [$cs_cv_libode_libs])],
	[cs_cv_libode_recent=no])

AS_IF([test $cs_cv_libode_recent = yes],
    [CS_CHECK_BUILD([if libode is built with OPCODE support],
	[cs_cv_libode_opcode],
	[AC_LANG_PROGRAM(
	    [[#include <ode/ode.h>]],
	    [dGeomTriMeshDataCreate();])],
	[], [C++],
	[CS_EMIT_BUILD_RESULT([cs_cv_libode], [ODE])], [], [],
	[$cs_cv_libode_cflags], [$cs_cv_libode_lflags], [$cs_cv_libode_libs])],
	[cs_cv_libode_opcode=no])

AS_IF([test $cs_cv_libode_recent = yes],
    [CS_CHECK_BUILD([if libode requires extern "C"],
	[cs_cv_libode_extern_c],
	[AC_LANG_PROGRAM(
	    [[#include <ode/ode.h>
	    extern "C"
	    int dCollideBoxPlane(dxGeom*,dxGeom*,int,dContactGeom*,int);]],
	    [dCollideBoxPlane(0,0,0,0,0);])],
	[], [C++], [CS_EMIT_BUILD_PROPERTY([ODE.EXTERN_C], [yes])], [], [],
	[$cs_cv_libode_cflags], [$cs_cv_libode_lflags], [$cs_cv_libode_libs])])



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
	    #include FT_INTERNAL_OBJECTS_H
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
	    [[#include <cal3d/cal3d.h>
	    #include <cal3d/animcallback.h>]],
	    [CalAnimationCallback* c = 0;
	    c->AnimationComplete((CalModel*)0, (void*)0);])],
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
# Check for VOS.
#------------------------------------------------------------------------------
AC_ARG_WITH([vos], [AC_HELP_STRING([--with-vos=dir],
    [specify location of VOS pkg-config files])])
AS_IF([test -z "$with_vos"], [with_vos=yes])

AS_IF([test "$with_vos" != no && test "$with_vos" != yes],
    [AS_IF([test -n "$PKG_CONFIG_PATH"],
 	[PKG_CONFIG_PATH="$PKG_CONFIG_PATH$PATH_SEPARATOR$with_vos"],
	[PKG_CONFIG_PATH="$with_vos"])
    export PKG_CONFIG_PATH])
CS_CHECK_PKG_CONFIG

m4_define([cs_vos_pkgs], [libmetaobject_a3dl libmetaobject_misc])
AS_IF([test "$with_vos" != no],
    [AC_CACHE_CHECK([for VOS], [cs_cv_libvos],
	[AS_IF([test -n "$PKG_CONFIG"],
	    [AS_IF([AC_RUN_LOG([$PKG_CONFIG --exists libmetaobject_a3dl])],
		[cs_cv_libvos=yes
		cs_cv_libvos_cflags=CS_RUN_PATH_NORMALIZE(
		    [$PKG_CONFIG --cflags cs_vos_pkgs])
		cs_cv_libvos_lflags=''
		cs_cv_libvos_libs=CS_RUN_PATH_NORMALIZE(
		    [$PKG_CONFIG --libs cs_vos_pkgs])],
		[cs_cv_libvos=no])],
	    [cs_cv_libvos=no])])
    AS_IF([test $cs_cv_libvos = yes],
	[CS_EMIT_BUILD_RESULT([cs_cv_libvos], [VOS])])])



#------------------------------------------------------------------------------
# Check for wxWidgets. Also check if it was built with OpenGL support since it
# is required by the Crystal Space wxWidgets canvas.
#
# IMPLEMENTATION NOTE
#
# On Windows, wx-config's --gl-libs options incorrectly emits a reference to a
# non-existent library (wxmsw_glxxx.lib). To handle this broken case, we first
# try the result of --libs and --gl-libs together; if that fails, then we try
# only --libs augmented with the results of our own OpenGL checks.
#------------------------------------------------------------------------------
CS_CHECK_TOOLS([WXCONFIG], [wx-config])

AC_CACHE_CHECK([for wxWidgets], [cs_cv_wxwidgets],
    [AS_IF([test -n "$WXCONFIG"],
    	[cs_cv_wxwidgets=yes
    	cs_cv_wxwidgets_cflags=CS_RUN_PATH_NORMALIZE([$WXCONFIG --cxxflags])
    	cs_cv_wxwidgets_lflags=CS_RUN_PATH_NORMALIZE([$WXCONFIG --ldflags])
    	cs_cv_wxwidgets_libs=CS_RUN_PATH_NORMALIZE([$WXCONFIG --libs])
    	cs_cv_wxwidgets_gllibs=CS_RUN_PATH_NORMALIZE([$WXCONFIG --gl-libs])],
        [cs_cv_wxwidgets=no])])

AS_IF([test $cs_cv_wxwidgets = yes],
    [CS_CHECK_BUILD([if wxWidgets SDK is usable], [cs_cv_wxwidgets_ok],
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
	[], [C++], [], [], [],
	[$cs_cv_wxwidgets_cflags],
	[$cs_cv_wxwidgets_lflags],
	[$cs_cv_wxwidgets_libs])],
    [cs_cv_wxwidgets_ok=no])

AS_IF([test $cs_cv_wxwidgets_ok = yes],
    [CS_CHECK_BUILD([if wxWidgets is built with OpenGL support],
	[cs_cv_wxwidgets_opengl],
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
	    [wxGLCanvas* f = new wxGLCanvas(0, wxID_ANY)])],
	[CS_CREATE_TUPLE(
	    [$cs_cv_wxwidgets_ok_cflags],
	    [$cs_cv_wxwidgets_ok_lflags],
	    [$cs_cv_wxwidgets_ok_libs   $cs_cv_wxwidgets_gllibs]) \
	CS_CREATE_TUPLE(
	    [$cs_cv_wxwidgets_ok_cflags $cs_cv_libglu_cflags],
	    [$cs_cv_wxwidgets_ok_lflags $cs_cv_libglu_lflags],
	    [$cs_cv_wxwidgets_ok_libs   $cs_cv_libglu_libs])],
	[C++])],
    [cs_cv_wxwidgets_opengl=no])

AS_IF([test $cs_cv_wxwidgets_opengl = yes],
    [CS_EMIT_BUILD_RESULT([cs_cv_wxwidgets_opengl], [WX])])



#------------------------------------------------------------------------------
# Check if Linux joystick API is present.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([for Linux joystick API], [cs_cv_sys_linux_joystick],
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
    [CS_EMIT_BUILD_PROPERTY([LINUXJOYSTICK.AVAILABLE], [yes])])



#------------------------------------------------------------------------------
# Check for Cg shader package.
#------------------------------------------------------------------------------
CS_CHECK_PROGS([CGC], [cgc])
CS_EMIT_BUILD_PROPERTY([CGC], [$CGC])

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
	    	[[#include <Cg/cg.h>]], [cgGetEnumString(CG_UNKNOWN);])],
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
	    [], [],
	    [CS_EMIT_BUILD_RESULT([cs_cv_libCgGL], [CG])], [],
	    [$cs_cv_libCg_cflags $cs_cv_libgl_cflags $cs_cv_libglu_cflags],
	    [$cs_cv_libCg_lflags $cs_cv_libgl_lflags $cs_cv_libglu_lflags],
	    [$cs_cv_libCg_libs   $cs_cv_libgl_libs   $cs_cv_libglu_libs],
	    [cggl, cgGL, CGGL, Cg, cg, CG])])])

CS_CHECK_LIBCG



#------------------------------------------------------------------------------
# Check for OpenSoundSystem (OSS).
#------------------------------------------------------------------------------
AC_CHECK_HEADER([soundcard.h],
    [CS_HEADER_PROPERTY([CS_HAS_SOUNDCARD_H])
    cs_cv_header_soundcard_h=yes],
    [AC_CHECK_HEADER([sys/soundcard.h],
	[CS_HEADER_PROPERTY([CS_HAS_SYS_SOUNDCARD_H])
	cs_cv_header_soundcard_h=yes],
	[AC_CHECK_HEADER([machine/soundcard.h],
	      [CS_HEADER_PROPERTY([CS_HAS_MACHINE_SOUNDCARD_H])
	      cs_cv_header_soundcard_h=yes],
	      [cs_cv_header_soundcard_h=no])])])

AS_IF([test $cs_cv_header_soundcard_h = yes],
    [CS_EMIT_BUILD_PROPERTY([OSS.AVAILABLE], [yes])])



#------------------------------------------------------------------------------
# Check for Advanced Linux Sound Architecture (ALSA)
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([asound],
    [AC_LANG_PROGRAM(
	[[#include <alsa/asoundlib.h>]],
	[snd_pcm_hw_params_t* p; snd_pcm_hw_params_alloca(&p);])],
    [], [], [CS_EMIT_BUILD_RESULT([cs_cv_libasound], [ALSA])])



#------------------------------------------------------------------------------
# Check for DivX4 or OpenDivX.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([for DivX4], [cs_cv_libdivxdecore4],
    [AC_LANG_PROGRAM(
	[[#include <divx4/decore.h>]],
	[DEC_MEM_REQS r; decore(0, DEC_OPT_MEMORY_REQS, 0, &r);])],
    [CS_CREATE_TUPLE([],[],[-ldivxdecore])], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libdivxdecore4], [DIVX4])], [], [],
    [$cs_cv_libm_cflags], [$cs_cv_libm_lflags], [$cs_cv_libm_libs])

AS_IF([test $cs_cv_libdivxdecore4 != yes],
    [CS_CHECK_BUILD([for OpenDivX], [cs_cv_libdivxdecore],
	[AC_LANG_PROGRAM(
	    [[#include <divx/decore.h>]],
	    [decore(0, DEC_OPT_RELEASE, 0, 0);])],
	[CS_CREATE_TUPLE([],[],[-ldivxdecore])], [], [], [], [],
	[$cs_cv_libm_cflags], [$cs_cv_libm_lflags], [$cs_cv_libm_libs])
    AS_IF([test $cs_cv_libdivxdecore = yes],
	[CS_CHECK_BUILD([is OpenDivX is sufficiently recent],
	    [cs_cv_libdivxdecore_recent],
	    [AC_LANG_PROGRAM(
		[[#include <divx/decore.h>]],
		[DEC_PARAM d; d.color_depth = 0;])],
	    [], [],
	    [CS_EMIT_BUILD_RESULT([cs_cv_libdivxdecore], [OPENDIVX])], [], [],
	    [$cs_cv_libdivxdecore_cflags],
	    [$cs_cv_libdivxdecore_lflags],
	    [$cs_cv_libdivxdecore_libs])])])



#------------------------------------------------------------------------------
# Check for Ascii Art library (aalib).
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([aa],
    [AC_LANG_PROGRAM([[#include <aalib.h>]], [aa_autoinit(0); aa_close(0);])],
    [], [], [CS_EMIT_BUILD_RESULT([cs_cv_libaa], [AALIB])], [], [], [], [],
    [aalib])



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
    [CS_HEADER_PROPERTY([CS_HAS_REGEX])
    CS_EMIT_BUILD_PROPERTY([REGEX.AVAILABLE], [yes])])



#------------------------------------------------------------------------------
# Check if backtrace() is available.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([for backtrace() support], [cs_cv_backtrace],
    [AC_LANG_PROGRAM(
	[[#ifdef HAVE_SYS_TYPES_H
	#include <sys/types.h>
	#endif
	#include <execinfo.h>]],
	[void* foo; int x; x = backtrace (&foo, 0); 
	backtrace_symbols (&foo, x);])],
    [], [],
    [CS_EMIT_BUILD_PROPERTY([BACKTRACE.AVAILABLE], [yes])])



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
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LONG_BITS],
    [AC_CHECK_SIZEOF([long])
    AS_IF([test $ac_cv_sizeof_long = 8],
	[cs_long_bits=64], [cs_long_bits=32])])

CS_CHECK_LONG_BITS
CS_HEADER_PROPERTY([CS_PROCESSOR_SIZE], [$cs_long_bits])
CS_EMIT_BUILD_PROPERTY([TARGET.PROCESSORSIZE], [$cs_long_bits])



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
  [CS_HEADER_PROPERTY([CS_USE_MMX])])



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
    [CS_HEADER_PROPERTY([CS_HAS_STDINT_H])
    CS_CHECK_BUILD([if INT64_C is available], [cs_cv_c_int64],
        [AC_LANG_PROGRAM(
            [[#define __STDC_CONSTANT_MACROS
            #define __STDC_LIMIT_MACROS
            #include <stdint.h>]],
            [(void)INT64_C(0x100000000)])],
        [], [C++], [CS_HEADER_PROPERTY([CS_HAS_INT64_C])])])



#------------------------------------------------------------------------------
# Check if intptr_t, uintptr_t, intmax_t and uintmax_t are available.
#------------------------------------------------------------------------------
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([inttypes.h], [CS_HEADER_PROPERTY([CS_HAS_INTTYPES_H])])
AC_CHECK_TYPE([intptr_t], [CS_HEADER_PROPERTY([CS_HAS_INTPTR_T])], [],
    [[#include <stddef.h>
    #ifdef HAVE_INTTYPES_H
    #include <inttypes.h>
    #endif
    #ifdef HAVE_STDINT_H
    #include <stdint.h>
    #endif]])
AC_CHECK_TYPE([intmax_t], [CS_HEADER_PROPERTY([CS_HAS_INTMAX_T])], [],
    [[#include <stddef.h>
    #ifdef HAVE_INTTYPES_H
    #include <inttypes.h>
    #endif
    #ifdef HAVE_STDINT_H
    #include <stdint.h>
    #endif]])



#------------------------------------------------------------------------------
# Check if the type wchar_t is available and determine its size.  It might be
# built-in, or it might come from the wchar.h header file.  Also determine
# availability of wcslen(), fputws(), fwide() and wcsrtombs().
#------------------------------------------------------------------------------
AC_LANG_PUSH([C++])

AC_CHECK_HEADERS([wchar.h], [CS_HEADER_PROPERTY([CS_HAS_WCHAR_H])])

AC_CHECK_TYPE([wchar_t], [cs_type_wchar_t=yes], [cs_type_wchar_t=no],
    [[#if HAVE_WCHAR_H
    #include <wchar.h>
    #endif]])

AS_IF([test $cs_type_wchar_t = yes],
    [AC_CHECK_FUNC([wcslen], [], [CS_HEADER_PROPERTY([CS_USE_FAKE_WCSLEN])])
    AC_CHECK_FUNC([fputws], [CS_HEADER_PROPERTY([CS_HAVE_FPUTWS])], [])
    AC_CHECK_FUNC([fwide], [CS_HEADER_PROPERTY([CS_HAVE_FWIDE])], [])
    AC_CHECK_FUNC([wcsrtombs], [CS_HEADER_PROPERTY([CS_HAVE_WCSRTOMBS])], [])
    AC_CHECK_SIZEOF([wchar_t], []
	[[#if HAVE_WCHAR_H
	#include <wchar.h>
	#endif]])
    CS_HEADER_PROPERTY([CS_WCHAR_T_SIZE], [$ac_cv_sizeof_wchar_t])],
    [CS_HEADER_PROPERTY([CS_USE_FAKE_WCHAR_TYPE])])

AC_LANG_POP([C++])



#------------------------------------------------------------------------------
# Check if MAXPATHLEN is defined by the system headers.
#------------------------------------------------------------------------------
AC_CHECK_HEADERS([sys/param.h], [CS_HEADER_PROPERTY([CS_HAS_SYS_PARAM_H])])
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
	[],
	[CS_HEADER_PROPERTY([CS_USE_FAKE_SOCKLEN_TYPE])],
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
# Check if C++ compiler understands new C++ `explicit' and 'typename' keywords.
#------------------------------------------------------------------------------
AC_CACHE_CHECK([for C++ `explicit' keyword], [cs_cv_cxx_explicit],
    [CS_BUILD_IFELSE(
	[AC_LANG_PROGRAM([class A { public: explicit A(int); };], [])],
	[], [C++], [cs_cv_cxx_explicit=yes], [cs_cv_cxx_explicit=no])])
AS_IF([test $cs_cv_cxx_explicit = no],
    [CS_HEADER_PROPERTY([CS_USE_FAKE_EXPLICIT_KEYWORD])])

AC_CACHE_CHECK([for C++ `typename' keyword], [cs_cv_cxx_typename],
    [CS_BUILD_IFELSE(
	[AC_LANG_PROGRAM(
	    [template <typename T> class A { public: T a; };], [])],
	[], [C++], [cs_cv_cxx_typename=yes], [cs_cv_cxx_typename=no])])
AS_IF([test $cs_cv_cxx_typename = no],
    [CS_HEADER_PROPERTY([CS_USE_FAKE_TYPENAME_KEYWORD])])



#------------------------------------------------------------------------------
# Check if C++ compiler understands new-style C++ casting syntax.
# For example: `static_cast<int>(foo)' versus `(int)foo'
#
# Specifically check for all four new casting operators since some botched
# compilers have been known to implement only a partial set.  (The OpenStep
# Objective-C++ compiler is one such botched implementation.  It fails to
# recognize reinterpret_cast<> even though it recognizes the others.)
#------------------------------------------------------------------------------
AC_CACHE_CHECK([for C++ new-style casts], [cs_cv_cxx_new_casts],
    [CS_BUILD_IFELSE(
	[AC_LANG_PROGRAM(
	    [int func1() { long n = 1; return static_cast<int>(n); }
	    char* func2() { static char const* s = "const";
		return const_cast<char*>(s); }
	    struct A {}; A* func3(A* a) { return dynamic_cast<A*>(a); }
	    A* func4(void* p) { return reinterpret_cast<A*>(p); }], [])],
	[], [C++], [cs_cv_cxx_new_casts=yes], [cs_cv_cxx_new_casts=no])])

AS_IF([test $cs_cv_cxx_new_casts = no],
    [CS_HEADER_PROPERTY([CS_USE_OLD_STYLE_CASTS])])



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
#	Check if gcc is able to understand csqsqrt.h.  This test will catch not
#	only versions of gcc which do not understand the assembly content, but
#	also buggy versions which fail with an internal error on this file
#	(such as the RedHat 7 gcc).  Furthermore, it is reported that some
#	versions of gcc 3.2.x cause csQsqrt() to return bogus results when
#	compiled in optimize mode on Intel hardware.  It is reported that this
#	same compiler crashes with an internal error when -fPIC is used, so we
#	use -fPIC as a way to detect this buggy compiler.  The MacOS/X PowerPC
#	version of csQsqrt() requires the -force_cpusubtype_ALL flag, so we use
#	that flag when available.  Sets the shell variable cs_cv_cxx_qsqrt_ok
#	to "yes" if csqsqrt.h is usable, otherwise "no".
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_QSQRT],
    [AC_REQUIRE([CS_CHECK_HOST_CPU])
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([CS_CHECK_OPTIMIZE_FLAGS])
    AC_REQUIRE([CS_CHECK_FORCE_CPUSUBTYPE_ALL])
    AC_REQUIRE([CS_CHECK_LONG_BITS])
    AC_CACHE_CHECK([if csQsqrt() compiles], [cs_cv_cxx_qsqrt_ok],
	[AS_IF([test $cs_long_bits -eq 32 && test $ac_compiler_gnu = yes],
	    [cs_qsqrt_cflags="$cs_optimize_flags"
	    AS_IF([test $cs_cv_prog_cxx_pic != no],
		[cs_qsqrt_cflags="$cs_qsqrt_cflags $cs_cv_prog_cxx_pic"])
	    AS_IF([test $cs_cv_prog_cxx_force_cpusubtype_all != no],
		[cs_qsqrt_cflags="$cs_cv_prog_cxx_force_cpusubtype_all"])
	    CS_BUILD_IFELSE(
		[AC_LANG_PROGRAM(
		    [[#define CS_PROCESSOR_$cs_host_cpu_normalized
		    #define CS_COMPILER_GCC
		    #include <csqsqrt.h>
		    ]],
		    [float n = csQsqrt(1.0f); (void)n;])],
		[CS_CREATE_TUPLE([$cs_qsqrt_cflags -I$srcdir/include])], [C++],
		[cs_cv_cxx_qsqrt_ok=yes], [cs_cv_cxx_qsqrt_ok=no],
		[$cs_cv_prog_cxx_enable_warnings \
		$cs_cv_prog_cxx_enable_errors])],
	    [cs_cv_cxx_qsqrt_ok=no])])])

CS_CHECK_QSQRT
AS_IF([test $cs_cv_cxx_qsqrt_ok = no],
    [CS_HEADER_PROPERTY([CS_NO_QSQRT])])



#------------------------------------------------------------------------------
# Check if a compiler bug discovered in gcc 2.96 (RedHat and other unstable
# versions) and gcc 3.0.1 causes csQint() to misbehave.
# -----------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_QINT],
    [AC_REQUIRE([CS_CHECK_OPTIMIZE_FLAGS])
    AC_REQUIRE([CS_CHECK_LONG_BITS])
    AC_CACHE_CHECK([if csQint() functions correctly], [cs_cv_cxx_qint_ok],
	[AS_IF([test $cs_long_bits -eq 32],
	    [AC_LANG_PUSH([C++])
	    cs_cflags_save="$CXXFLAGS"
	    CXXFLAGS="$CXXFLAGS $cs_optimize_flags"
	    AC_RUN_IFELSE(
		[AC_LANG_PROGRAM(
		    [static inline long double2int(double val) {
			long* l;
			val += 68719476736.0;
			l = (long*)((char*)&val + 2);
			return *l; }],
		    [exit(double2int(255.99) != 255);])],
		[cs_cv_cxx_qint_ok=yes],
		[cs_cv_cxx_qint_ok=no],
		[cs_cv_cxx_qint_ok=no])
	    CXXFLAGS="$cs_cflags_save"
	    AC_LANG_POP([C++])],
	    [cs_cv_cxx_qint_ok=no])])])

CS_CHECK_QINT
AS_IF([test $cs_cv_cxx_qint_ok = no],
    [CS_HEADER_PROPERTY([CS_QINT_WORKAROUND])])



#------------------------------------------------------------------------------
# Determine if plugin meta-information should be embedded directly into the
# plugin module or if it should exist in a stand-alone .csplugin file.  The
# macro CS_CHECK_ENABLE_META_EMBEDDING sets the shell variable
# enable_meta_info_embedding to yes or no.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_ENABLE_META_EMBEDDING],
    [AC_REQUIRE([CS_CHECK_HOST])
    AC_MSG_CHECKING([whether to embed plugin meta-information])
    case $cs_host_target in
	unix) cs_embed_meta_info_default=no ;;
	*) cs_embed_meta_info_default=yes ;;
    esac
    AC_ARG_ENABLE([meta-info-embedding],
	[AC_HELP_STRING([--enable-meta-info-embedding],
	    [store plugin meta-information directly inside plugin modules if
	    supported by platform; if disabled, meta-information is stored in
	    stand-alone .csplugin files; this is enabled by default for
	    non-Unix platforms, and disabled by default for Unix platforms
	    since the Unix implementation requires the GPL-licensed libbfd
	    library; enable this option on Unix only if you are certain you
	    want a GPL-licensed library infecting your project])],
	[], [enable_meta_info_embedding=$cs_embed_meta_info_default])
    AC_MSG_RESULT([$enable_meta_info_embedding])])

CS_CHECK_ENABLE_META_EMBEDDING
CS_EMIT_BUILD_PROPERTY([EMBED_META], [$enable_meta_info_embedding])
AS_IF([test $enable_meta_info_embedding = yes],
    [CS_HEADER_PROPERTY([CS_EMBED_PLUGIN_META])])



#------------------------------------------------------------------------------
# Check if libbfd is present (part of GNU binutils).
#
# WARNING
#
# libbfd carries a GPL license which is incompatible with the LGPL license of
# Crystal Space.  On platforms for which libbfd can be used, we check for and
# utilize this library only if explicitly requested by the user via
# --enable-meta-info-embedding.
#
# IMPLEMENTATION NOTES
#
# It seems that some platforms have two version of libiberty installed: one
# from binutils and one from gcc.  The binutils version resides in /usr/lib,
# whereas the gcc version resides in the gcc installation directory.  The gcc
# version, by default, takes precedence at link time over the binutils version.
# Unfortunately, in broken cases, the gcc version of libiberty is missing
# htab_create_alloc() which is required by some libbfd functions.  The
# extensive secondary check of libbfd catches this anomalous case of broken gcc
# libiberty.  It turns out that it is possible to make the linker prefer the
# binutils version by specifying -L/usr/lib, thus the extensive test attempts
# to do so in an effort to resolve this unfortunate issue.
#
# Furthermore, recent versions of Mingw supply libbfd and libiberty.  Since
# Crystal Space uses native Win32 API for meta-information embedding on
# Windows, we do not require these libraries on Windows.  More importantly,
# users do not want to see these GPL-licensed libraries appear in the link
# statement for plugin modules, thus we explicitly disable this test on
# Windows.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_ENABLE_LIBBFD_TEST],
    [AC_REQUIRE([CS_CHECK_ENABLE_META_EMBEDDING])
    AC_REQUIRE([AC_CANONICAL_HOST])
    AS_IF([test $enable_meta_info_embedding = yes],
	[case $host_os in
	    mingw*|cygwin*) cs_check_libbfd=no ;;
	    *) cs_check_libbfd=yes ;;
	esac],
	[cs_check_libbfd=no])])

CS_CHECK_ENABLE_LIBBFD_TEST

AS_IF([test $cs_check_libbfd = yes],
    [CS_CHECK_LIB_WITH([bfd],
	[AC_LANG_PROGRAM([[#include <bfd.h>]], [bfd_init();])],
	[], [], [], [], [], [], [-liberty])],
    [cs_cv_libbfd=no])

AS_IF([test $cs_cv_libbfd = yes],
    [CS_CHECK_BUILD([if libbfd is usable], [cs_cv_libbfd_ok],
	[AC_LANG_PROGRAM([[#include <bfd.h>]],
	    [bfd* p;
	    asection* s;
	    bfd_init();
	    p = bfd_openr(0,0);
	    bfd_check_format(p,bfd_object);
	    bfd_get_section_by_name(p,0);
	    bfd_section_size(p,s);
	    bfd_get_section_contents(p,s,0,0,0);
	    bfd_close(p);])],
	[CS_CREATE_TUPLE() CS_CREATE_TUPLE([],[-L/usr/lib],[])], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libbfd_ok], [LIBBFD])
	CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS],[$cs_cv_libbfd_ok_cflags],[+])
	CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS],[$cs_cv_libbfd_ok_lflags],[+])
	CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS],[$cs_cv_libbfd_ok_libs],[+])],
	[], [],
	[$cs_cv_libbfd_cflags], [$cs_cv_libbfd_lflags], [$cs_cv_libbfd_libs])])



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
