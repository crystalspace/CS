#                                               -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
# Autoconf input script for Crystal Space.  Process this file with
# CS/bin/autogen.sh to produce a configure script.
#==============================================================================
AC_PREREQ([2.56])


#------------------------------------------------------------------------------
# cs_version_number
#	Keep this value synchronized with CS/include/csver.h.
#------------------------------------------------------------------------------
m4_define([cs_version_number], [0.97])



#------------------------------------------------------------------------------
# cs_install_dir_default
#	Default installation directory.
#------------------------------------------------------------------------------
m4_define([cs_install_dir_default], [/usr/local/crystal])



#------------------------------------------------------------------------------
# cs_lib_paths_default
#	Whitespace delimited list of directory tuples in which to search, by
#	default, for external libraries.  Each list item can specify an
#	include|library directory tuple (for example, "/usr/include|/usr/lib"),
#	or a single directory (for example, "/usr").  If the second form is
#	used, then the directory itself is searched first.  If the external
#	resource is not found, then "include" and "lib" subdirectories of the
#	directory are searched.  Thus, specifying "/proj" is shorthand for
#	"/proj|/proj /proj/include|/proj/lib".
#
# Present Cases:
#	/usr/local -- Not all compilers search here by default, so we specify
#		it manually.
#	/sw -- Fink, the MacOS/X manager of Unix packages, installs here by
#		default.
#------------------------------------------------------------------------------
m4_define([cs_lib_paths_default],
    [/usr/local/include|/usr/local/lib /sw/include|/sw/lib])



#------------------------------------------------------------------------------
# CS_BUILD_PROPERTY(KEY, VALUE, [APPEND])
#	A utility function which invokes both CS_MAKEFILE_PROPERTY() and
#	CS_JAMCONFIG_PROPERTY(), thereby emitting a build property suitable for
#	each build system (GNU make & Jam).  Some variable names differ between
#	the makefile and the Jam build systems, thus this implementation
#	performs translation on KEY from the makefile variable name to the Jam
#	variable name automatically.  However, VALUE not translated
#	automatically, so if you have a situation in which a build variable is
#	mentioned in VALUE, then you should instead invoke
#	CS_MAKEFILE_PROPERTY() and CS_JAMCONFIG_PROPERTY() manually with the
#	appropriate values.
#------------------------------------------------------------------------------
AC_DEFUN([CS_BUILD_PROPERTY],
    [CS_MAKEFILE_PROPERTY([$1], [$2], [$3])
    CS_JAMCONFIG_PROPERTY([_CS_TRANSLATE_MAKE_TO_JAM([$1])], [$2], [$3])])



#------------------------------------------------------------------------------
# _CS_TRANSLATE_MAKE_TO_JAM([TEXT])
#	Helper function for CS_BUILD_PROPERTY() which translates makefile-style
#	names to Jam style names.  This is necessary since the makefile and Jam
#	build systems use different variable names in some cases.  It is more
#	convenient to perform the name translation here in one place, than
#	manually in each place a build property is emitted.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_TRANSLATE_MAKE_TO_JAM],
    [m4_case([$1],
	[FLEXBIN], [CMD.FLEX],
	[BISONBIN], [CMD.BISON],
	[SWIGBIN], [CMD.SWIG],
	[LUASWIGBIN], [CMD.LUASWIG],
	[NASM.BIN], [CMD.NASM],
	[CFLAGS.SYSTEM], [COMPILER.CFLAGS],
	[CXXFLAGS.SYSTEM], [COMPILER.C++FLAGS],
	[MACOSX.LFLAGS.CONFIG], [COMPILER.LFLAGS],
	[CFLAGS.optimize], [COMPILER.CFLAGS.optimize],
	[CFLAGS.debug], [COMPILER.CFLAGS.debug],
	[LFLAGS.debug], [COMPILER.LFLAGS.debug],
	[LFLAGS.SYSTEM], [COMPILER.LFLAGS],
	[LIBS.EXE], [COMPILER.LFLAGS],
	[LIBS.SYSTEM], [COMPILER.LFLAGS],
	[USE_XFREE86VM], [XFREE86VM.AVAILABLE],
	[PROC], [TARGET.PROCESSOR],
	[$1])])



#------------------------------------------------------------------------------
# CS_EMIT_BUILD_RESULT(CACHE-VAR, PREFIX)
#	Record the results of CS_CHECK_BUILD() via makefile variables in the
#	makefile and Jam text caches.  If CACHE-VAR indicates that the build
#	succeeded, then the following properties are emitted:
#
#	PREFIX.AVAILABLE = yes
#	PREFIX.CFLAGS = $CACHE-VAR_cflags
#	PREFIX.LFLAGS = $CACHE-VAR_lflags $CACHE-VAR_libs
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_BUILD_RESULT],
    [AS_IF([test "$$1" = yes],
	[CS_BUILD_PROPERTY([$2.AVAILABLE], [yes])
	cs_result_cflags=CS_TRIM([$$1_cflags])
	cs_result_lflags=CS_TRIM([$$1_lflags $$1_libs])
	AS_IF([test -n "$cs_result_cflags"],
	    [CS_BUILD_PROPERTY([$2.CFLAGS], [$cs_result_cflags])])
	AS_IF([test -n "$cs_result_lflags"],
	    [CS_BUILD_PROPERTY([$2.LFLAGS], [$cs_result_lflags])])])])



#------------------------------------------------------------------------------
# CS_EMIT_BUILD_FLAGS(MESSAGE, CACHE-VAR, FLAGS, [LANGUAGE],
#                     MAKEFILE-VARIABLE, [APPEND], [CACHE-VALUE],
#                     [ACTION-IF-RECOGNIED], [ACTION-IF-NOT-RECOGNIED])
#	Check if the compiler or linker recognizes a command-line option.
#	MESSAGE is the "checking" message.  CACHE-VAR is the shell cache
#	variable which receives the flag recognized by the compiler or linker,
#	or "no" if the flag was not recognized.  FLAGS is a
#	whitespace-delimited list of build tuples created with
#	CS_CREATE_TUPLE().  Each tuple from FLAGS is attempted in order until
#	one is found which is recognized by the compiler.  After that, no
#	further flags are checked.  LANGUAGE is typically either C or C++ and
#	specifies which compiler to use for the test.  If LANGUAGE is omitted,
#	C is used.  MAKEFILE-VARIABLE is the name of the makefile variable to
#	insert into the makefile and Jam text caches if a usable flag is
#	encountered.  CACHE-VALUE is the value to assign to the makefile
#	variable.  If omitted, it defaults to $cs_build_cflags.  If you are
#	checking linker flags, rather than compiler flags, then you should
#	specify $cs_build_lflags for this argument.  If APPEND is not the empty
#	string, then CACHE-VALUE is appended to the existing value of the
#	makefile variable.  If the command-line option was recognized, then
#	ACTION-IF-RECOGNIZED is invoked, otherwise ACTION-IF-NOT-RECOGNIZED is
#	invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_BUILD_FLAGS],
    [AC_CACHE_CHECK([$1], [$2],
	[CS_BUILD_IFELSE([], [$3], [$4],
	    [$2=m4_default([$7],[$cs_build_cflags])], [$2=no])])
    AS_IF([test "$$2" != no],
	[$8
	CS_BUILD_PROPERTY([$5], [$$2], [$6])],
	[m4_default([$9],[:])])])



#===============================================================================
# INITIALIZATION
#===============================================================================
AC_INIT([crystal], [cs_version_number], [crystal-main@lists.sourceforge.net])
CS_PACKAGEINFO([Crystal Space],
    [Copyright (C)1998-2003 Jorrit Tyberghein and others],
    [http://crystal.sourceforge.net/])
AC_CONFIG_SRCDIR([libs/cssys/cssys.mak])
AC_CONFIG_AUX_DIR([mk/autoconf])
AC_PREFIX_DEFAULT([cs_install_dir_default])
AC_CANONICAL_HOST



#------------------------------------------------------------------------------
# Check for compilers and common tools.
#------------------------------------------------------------------------------
AC_PROG_CC
CS_MAKEFILE_PROPERTY([CC], [$CC -c])
CS_JAMCONFIG_PROPERTY([CMD.CC], [$CC])

AC_PROG_CXX
CS_MAKEFILE_PROPERTY([CXX], [$CXX -c])
CS_JAMCONFIG_PROPERTY([CMD.C++], [$CXX])

# C++ compiler is used as linker.
CS_MAKEFILE_PROPERTY([LINK], [$CXX])
CS_JAMCONFIG_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.C++)])])

AS_IF([test $ac_compiler_gnu = yes],
    [cs_compiler_name=GCC CS_HEADER_PROPERTY([COMP_GCC])],
    [cs_compiler_name="AS_TR_CPP([$CXX])"])
CS_MAKEFILE_PROPERTY([COMP], [$cs_compiler_name])
CS_JAMCONFIG_PROPERTY([COMPILER.TYPE], [$cs_compiler_name])
CS_HEADER_PROPERTY([CS_COMPILER_NAME],
    [AS_ESCAPE(["])$cs_compiler_name[]AS_ESCAPE(["])])

AC_CHECK_PROGS([FLEXBIN], [flex])
AS_IF([test -n "$FLEXBIN"], [CS_BUILD_PROPERTY([FLEXBIN], [$FLEXBIN])])

AC_CHECK_PROGS([BISONBIN], [bison])
AS_IF([test -n "$BISONBIN"],
    [CS_CHECK_PROG_VERSION([bison], [$BISONBIN -V], [1.35], [9.9],
	[CS_BUILD_PROPERTY([BISONBIN], [$BISONBIN])])])

AC_CHECK_PROGS([DOXYGEN], [doxygen])
AS_IF([test -n "$DOXYGEN"], [CS_BUILD_PROPERTY([CMD.DOXYGEN], [$DOXYGEN])])

AC_CHECK_PROGS([TEXI2DVI], [texi2dvi])
AS_IF([test -n "$TEXI2DVI"], [CS_BUILD_PROPERTY([CMD.TEXI2DVI], [$TEXI2DVI])])

AC_CHECK_PROGS([DVIPS], [dvips])
AS_IF([test -n "$DVIPS"], [CS_BUILD_PROPERTY([CMD.DVIPS], [$DVIPS])])

AC_CHECK_PROGS([DVIPDF], [dvipdf])
AS_IF([test -n "$DVIPDF"], [CS_BUILD_PROPERTY([CMD.DVIPDF], [$DVIPDF])])

AC_CHECK_PROGS([MAKEINFO], [makeinfo])
AS_IF([test -n "$MAKEINFO"], [CS_BUILD_PROPERTY([CMD.MAKEINFO], [$MAKEINFO])])

AC_CHECK_PROGS([INSTALL], [install])
AS_IF([test -n "$INSTALL"], [CS_BUILD_PROPERTY([CMD.INSTALL], [$INSTALL])])

AC_PROG_RANLIB
AS_IF([test -n "$RANLIB"], [CS_BUILD_PROPERTY([CMD.RANLIB], [$RANLIB])])

AC_CHECK_PROGS([SWIGBIN], [swig])
AS_IF([test -n "$SWIGBIN"],
    [CS_CHECK_PROG_VERSION([swig], [$SWIGBIN -version 2>&1], [1.3.14],
	[9.9|.9], [CS_BUILD_PROPERTY([SWIGBIN], [$SWIGBIN])])])

AC_CHECK_PROGS([MAKE], [gnumake gmake make])
AC_CHECK_PROGS([JAM], [jam])
AC_PROG_MAKE_SET



#------------------------------------------------------------------------------
# Determine how to make a directory tree.  We need to know the exact commands,
# so that we can emit them to the makefile, thus the AS_MKDIR_P function is not
# what we want to use here.  First try "mkdir -p", then try the older "mkdirs".
# Finally, if the mkdir command failed to recognize -p, then it might have
# created a directory named "-p", so clean up that bogus directory.
#------------------------------------------------------------------------------
AC_CACHE_CHECK([how to create a directory], [cs_cv_shell_mkdir],
    [cs_cv_shell_mkdir='mkdir'])
CS_BUILD_PROPERTY([CMD.MKDIR], [$cs_cv_shell_mkdir])

AC_CACHE_CHECK([how to create a directory tree], [cs_cv_shell_mkdir_p],
    [if $cs_cv_shell_mkdir -p . 2>/dev/null; then
	cs_cv_shell_mkdir_p='mkdir -p'
    elif mkdirs . 2>/dev/null; then
	cs_cv_shell_mkdir_p='mkdirs'
    fi
    test -r ./-p && rm -rf ./-p])

AS_VAR_SET_IF([cs_cv_shell_mkdir_p],
    [CS_BUILD_PROPERTY([CMD.MKDIRS], [$cs_cv_shell_mkdir_p])],
    [AC_MSG_ERROR([do not know how to create a directory tree])])



#------------------------------------------------------------------------------
# Check for NASM and extensions (such as %xdefine) which we require.
#------------------------------------------------------------------------------
AC_CHECK_PROGS([NASM], [nasmw nasm])
AS_IF([test -n "$NASM"],
    [AC_CACHE_CHECK([if nasm is sufficiently recent], [cs_cv_prog_nasm_ok],
	[echo "%xdefine TEST" > conftest.asm
	AS_IF([AC_RUN_LOG(
	    [$NASM conftest.asm -o conftest.$OBJEXT 1>&2])],
	    [cs_cv_prog_nasm_ok=yes], [cs_cv_prog_nasm_ok=no])
	rm -f conftest.*])
    AS_IF([test $cs_cv_prog_nasm_ok = yes],
	[CS_BUILD_PROPERTY([NASM.AVAILABLE], [yes])
	CS_BUILD_PROPERTY([NASM.BIN], [$NASM])
	CS_HEADER_PROPERTY([DO_NASM])])])



#------------------------------------------------------------------------------
# Check if makedep is installed and has the correct version.  This is an
# offshoot of X consortium's makedepend modified by Andrew Zablotony.  We
# would like to eliminate this in the future and just use the stock version
# rather than the specially augumented one.
#------------------------------------------------------------------------------
AC_CHECK_PROGS([MAKEDEP], [makedep])
AS_IF([test -n "$MAKEDEP"],
    [AC_CACHE_CHECK([for makedep extensions], [cs_cv_prog_makedep_ok],
	[cs_makedep_ver=`makedep -V | sed -e "s/.*Version *//" 2>/dev/null`
	AS_IF([test `expr "${cs_makedep_ver}" ">" 0.0.0` = "1"],
	    [cs_cv_prog_makedep_ok=yes], [cs_cv_prog_makedep_ok=no])])],
    [cs_cv_prog_makedep_ok=no])
AS_IF([test $cs_cv_prog_makedep_ok = yes],
    [CS_MAKEFILE_PROPERTY([MAKEDEP.AVAILABLE], [$cs_cv_prog_makedep_ok])
    CS_MAKEFILE_PROPERTY([CMD.MAKEDEP], [$MAKEDEP])
    CS_MAKEFILE_PROPERTY([DEPEND_TOOL], [mkdep])],
    [CS_MAKEFILE_PROPERTY([DEPEND_TOOL], [cc])])



#------------------------------------------------------------------------------
# Check for pthread.  Also check if pthread implementation supports the
# recursive mutex extension.
#------------------------------------------------------------------------------
m4_define([cs_pthread_flags],
    [CS_CREATE_TUPLE() \
    CS_CREATE_TUPLE([], [], [-lpthread]) \
    CS_CREATE_TUPLE([-pthread], [-pthread], []) \
    CS_CREATE_TUPLE([-pthread], [-pthread], [-lpthread]) \
    CS_CREATE_TUPLE([-pthread], [-pthread], [-lc_r])])

AC_DEFUN([CS_CHECK_PTHREAD],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
	cygwin*)
	    cs_cv_sys_pthread=no
	    ;;
	*)
	    CS_CHECK_BUILD([for pthread], [cs_cv_sys_pthread],
		[AC_LANG_PROGRAM(
		    [#include <pthread.h>
		    void* worker(void* p) { (void)p; return p; }],
		    [pthread_t tid; pthread_create(&tid, 0, worker, 0);])],
		[cs_pthread_flags], [],
		[CS_EMIT_BUILD_RESULT([cs_cv_sys_pthread], [PTHREAD])])
	    ;;
    esac])

CS_CHECK_PTHREAD

AS_IF([test $cs_cv_sys_pthread = yes],
    [AC_CACHE_CHECK([for pthread recursive mutexes],
	[cs_cv_sys_pthread_recursive],
	[CS_BUILD_IFELSE(
	    [AC_LANG_PROGRAM(
		[#include <pthread.h>],
		[pthread_mutexattr_t attr;
		pthread_mutexattr_settype (&attr, CS_RECURSIVE);])],
	    [CS_CREATE_TUPLE([-DCS_RECURSIVE=PTHREAD_MUTEX_RECURSIVE]) \
	    CS_CREATE_TUPLE([-DCS_RECURSIVE=PTHREAD_MUTEX_RECURSIVE_NP])],
	    [C++],
	    [cs_cv_sys_pthread_recursive=`echo $cs_build_cflags | \
		sed 's/.*\(PTHREAD_MUTEX_RECURSIVE_*N*P*\).*/\1/'`],
	    [cs_cv_sys_pthread_recursive=no],
	    [$cs_cv_sys_pthread_cflags -D_GNU_SOURCE],
	    [$cs_cv_sys_pthread_lflags],
	    [$cs_cv_sys_pthread_libs])])
    AS_IF([test $cs_cv_sys_pthread_recursive != no],
	[CS_HEADER_PROPERTY([CS_PTHREAD_MUTEX_RECURSIVE],
	    [$cs_cv_sys_pthread_recursive])])])



#------------------------------------------------------------------------------
# Check if Perl SDK is installed and which compiler and linker flags are
# required to embed Perl.  Also determine if the glue file can be created with
# ExtUtils::Embed or if it must be created manually.
#------------------------------------------------------------------------------
AC_CHECK_PROGS([PERL], [perl5 perl])
AS_IF([test -n "$PERL"],
    [CS_BUILD_PROPERTY([PERL5.AVAILABLE], [yes])
    CS_BUILD_PROPERTY([PERL5], [$PERL])
    CS_BUILD_PROPERTY([PERL], [$PERL])
    CS_BUILD_PROPERTY([CMD.PERL5], [$PERL])

    AC_CACHE_CHECK([for perl ExtUtils::Embed module], [cs_cv_perl_mod_embed],
	[AS_IF([AC_RUN_LOG([$PERL -MExtUtils::Embed -e 0 1>&2])],
	    [cs_cv_perl_mod_embed=yes], [cs_cv_perl_mod_embed=no])])
    AS_IF([test $cs_cv_perl_mod_embed = yes],
	[CS_BUILD_PROPERTY([PERL5.EXTUTILS.EMBED.AVAILABLE], [yes])

	AC_CACHE_CHECK([for perl DynaLoader module],
	    [cs_cv_perl_mod_dynaloader],
	    [AS_IF([AC_RUN_LOG([$PERL -MDynaLoader -e 0 1>&2])],
		[cs_cv_perl_mod_dynaloader=yes],
		[cs_cv_perl_mod_dynaloader=no])])
	AS_IF([test $cs_cv_perl_mod_dynaloader = yes],
	    [CS_BUILD_PROPERTY([PERL5.DYNALOADER.AVAILABLE], [yes])])

	cs_cv_perl_cflags=`AC_RUN_LOG([$PERL -MExtUtils::Embed -e ccopts])`
	cs_cv_perl_lflags=`AC_RUN_LOG([$PERL -MExtUtils::Embed -e ldopts])`],

	# ExtUtils::Embed absent.
	[AC_MSG_NOTICE([Recommend you install the latest release of Perl 5.])
	AC_MSG_NOTICE([http://www.perl.org/])

	AC_CACHE_CHECK([for perl Config module], [cs_cv_perl_mod_config],
	    [AS_IF([AC_RUN_LOG([$PERL -MConfig -e 0 1>&2])],
		[cs_cv_perl_mod_config=yes], [cs_cv_perl_mod_config=no])])
	AS_IF([test $cs_cv_perl_mod_config = yes],
	    [cs_perl_core=`$PERL -MConfig -e 'print $Config{sitearch}'`
	    cs_perl_core="${cs_perl_core}/CORE"
	    cs_cv_perl_cflags=`$PERL -MConfig -e 'print $Config{ccflags}'`
	    cs_cv_perl_cflags="${cs_cv_perl_cflags} -I${cs_perl_core}"
	    cs_cv_perl_lflags=`$PERL -MConfig -e \
		'print $Config{ldflags}, " ", $Config{libs}'`
	    cs_cv_perl_lflags="${cs_cv_perl_lflags} -L${cs_perl_core}"],
	    [cs_perl_core=`$PERL -V:sitearch | \
		$PERL -e '<STDIN> =~ m/\x27(.*)\x27/; print $1'`
	    cs_perl_core="${cs_perl_core}/CORE"
	    cs_cv_perl_cflags=`$PERL -V:ccflags | \
		$PERL -e '<STDIN> =~ m/\x27(.*)\x27/; print $1'`
	    cs_cv_perl_cflags="${cs_cv_perl_cflags} -I${cs_perl_core}"
	    cs_cv_perl_lflags=`$PERL -V:ldflags | \
		$PERL -e '<STDIN> =~ m/\x27(.*)\x27/; print $1'`
	    cs_cv_perl_lflags="${cs_cv_perl_lflags} `$PERL -V:libs | \
		$PERL -e '<STDIN> =~ m/\x27(.*)\x27/; print $1'`"
	    cs_cv_perl_lflags="${cs_cv_perl_lflags} -L${cs_perl_core}"])])

    cs_cv_perl_cflags=CS_TRIM([$cs_cv_perl_cflags])
    cs_cv_perl_lflags=CS_TRIM([$cs_cv_perl_lflags])
    AS_IF([test -n "$cs_cv_perl_cflags$cs_cv_perl_lflags"],
	[CS_BUILD_PROPERTY([PERL5.SDK.AVAILABLE], [yes])
	AS_IF([test -n "$cs_cv_perl_cflags"],
	    [CS_BUILD_PROPERTY([PERL5.CFLAGS], [$cs_cv_perl_cflags])])
	AS_IF([test -n "$cs_cv_perl_lflags"],
	    [CS_BUILD_PROPERTY([PERL5.LFLAGS], [$cs_cv_perl_lflags])])])

#@@@ The following is intended to check whether the Perl SDK builds correctly.
# However, I [res] get link errors on cygwin, tho in theory the SDK should be
# usable.
#   AS_IF([test -n "$cs_cv_perl_cflags"],
#	[CS_BUILD_PROPERTY([PERL5.CFLAGS], [$cs_cv_perl_cflags])])
#   AS_IF([test -n "$cs_cv_perl_lflags"],
#	[CS_BUILD_PROPERTY([PERL5.LFLAGS], [$cs_cv_perl_lflags])])
#   CS_CHECK_BUILD([if Perl SDK is usable], [cs_cv_perl],
#       [AC_LANG_PROGRAM([
#            #include <EXTERN.h>
#	    #include <perl.h>],
#	    [perl_run (0);])],
#    	    [],
#           [], [CS_BUILD_PROPERTY([PERL5.SDK.AVAILABLE], [yes])], [], [],
#           [$cs_cv_perl_cflags], [$cs_cv_perl_lflags],
#           [])
    ])



#------------------------------------------------------------------------------
# Check if Python SDK is installed and which compiler and linker flags are
# required to embed Python.  Several techniques are used in an attempt to
# determine the proper flags.  First, an attempt it made to query Python's
# "distutils.sysconfig" module.  This should work for Python 2.x.  If that
# fails, Python's "sys" module is queried to find the path of the Python
# standard library.  Inside the library's "config" directory is a makefile
# which can be parsed to glean the necessary flags.  If the Python executable
# is not available, try locating the Python directories and Makefile manually.
# As a last resort, if the environment variables PYTHON_INC and PYTHON_LIB are
# set, then use those values.
#------------------------------------------------------------------------------
AC_CHECK_PROGS([PYTHON], [python])
AS_IF([test -n "$PYTHON"], [CS_BUILD_PROPERTY([PYTHON], [$PYTHON])])

AS_IF([test -n "$PYTHON"],
    [AC_CACHE_CHECK([for python distutils module], [cs_cv_python_distutils],
	[AS_IF([AC_RUN_LOG([$PYTHON -c 'import distutils.sysconfig' 1>&2])],
	    [cs_cv_python_distutils=yes], [cs_cv_python_distutils=no])])
    AS_IF([test $cs_cv_python_distutils = yes],
	[CS_BUILD_PROPERTY([PYTHON.DISTUTILS], [yes])])],
    [cs_cv_python_distutils=no])

AC_CACHE_CHECK([for python SDK], [cs_cv_python_sdk],
    [cs_cv_python_sdk=no
    cs_python_lib_dir=''
    cs_python_makefile=''

    AS_IF([test -n "$PYTHON"],
	[# Python 2.x?
	AS_IF([test $cs_cv_python_distutils = yes],
	    [cs_cv_python_base=`AC_RUN_LOG([$PYTHON -c \
		'import distutils.sysconfig ; import os.path ; \
		print os.path.basename( \
		    distutils.sysconfig.get_python_lib(0,1))'])`
	    cs_cv_python_base_cflags=`AC_RUN_LOG([$PYTHON -c \
		'import distutils.sysconfig ; \
		print "-I"+distutils.sysconfig.get_python_inc()'])`
	    cs_cv_python_base_lflags=`AC_RUN_LOG([$PYTHON -c \
		'import distutils.sysconfig ; import os.path ; \
		print "-L"+distutils.sysconfig.get_python_lib(0,1)+" "+ \
		    "-L"+distutils.sysconfig.get_python_lib(0,1)+"/config"'])`
	    cs_cv_python_base_libs=`AC_RUN_LOG([$PYTHON -c \
		'import distutils.sysconfig ; \
		print (distutils.sysconfig.get_config_var("LIBS") or "") + \
		" " + (distutils.sysconfig.get_config_var("SYSLIBS") or "")'])`
	    cs_cv_python_base_cflags=CS_TRIM([$cs_cv_python_base_cflags])
	    cs_cv_python_base_lflags=CS_TRIM([$cs_cv_python_base_lflags])
	    cs_cv_python_base_libs=CS_TRIM([$cs_cv_python_base_libs])
	    cs_cv_python_sdk=yes])

	# Not Python 2.x; try Python 1.x.
	AS_IF([test $cs_cv_python_sdk != yes],
	    [cs_python_paths=`AC_RUN_LOG([$PYTHON -c \
'import sys
for p in sys.path: print p'])`
	    AS_IF([test -n "$cs_python_paths"],
		[for cs_pypath in $cs_python_paths
		do
		    cs_pypath=`echo $cs_pypath | sed 's:/$::'`
		    AS_IF([test -f $cs_pypath/config/Makefile],
			[cs_python_lib_dir=$cs_pypath
			cs_python_makefile=$cs_pypath/config/Makefile
			break])
		done])])],

	[# Unable to query missing Python executable or query failed, so try
	# PYTHON_LIB and well-known locations.
	AS_IF([test -n "$PYTHON_LIB"],
	    [cs_python_lib_dir=$PYTHON_LIB
	    AS_IF([test -f $PYTHON_LIB/config/Makefile],
		[cs_python_makefile=$PYTHON_LIB/config/Makefile])],
	    [for cs_pypath in /usr/local/python /usr/local /usr
	    do
		# If multiple Python subdirectories exists in a given path,
		# we choose the last one since it probably has the highest
		# version number (that is, do not "break" from the loop).
		for cs_pydir in $cs_pypath/lib/python*
		do
		    AS_IF([test -f $cs_pydir/config/Makefile],
			[cs_python_lib_dir=$cs_pydir
			cs_python_makefile=$cs_pydir/config/Makefile])
		done
	    done])])

    # If compiler and linker flags not yet determined but resource directory
    # and makefile located, then try extracting information from Python
    # makefile.
    AS_IF([test $cs_cv_python_sdk != yes],
	[AS_IF([test -n "$cs_python_makefile"],
	    [cs_pylibs=`AC_RUN_LOG(
		[grep '^LIBS[[ 	]]*=' $cs_python_makefile | \
		    sed 's/LIBS[[ 	]]*=//'])`
	    cs_pylibm=`AC_RUN_LOG(
		[grep '^LIBM[[ 	]]*=' $cs_python_makefile | \
		    sed 's/LIBM[[ 	]]*=//'])`
	    cs_pylibc=`AC_RUN_LOG(
		[grep '^LIBC[[ 	]]*=' $cs_python_makefile | \
		    sed 's/LIBC[[ 	]]*=//'])`])

	# Construct CFLAGS and LFLAGS
	AS_IF([test -n "$cs_python_lib_dir"],
	    [cs_cv_python_base=`AS_BASENAME([$cs_python_lib_dir])`
	    AS_IF([test -n "$cs_cv_python_base"],
		[AS_IF([test -n "$PYTHON_INC"],
		    [cs_python_inc_dir=$PYTHON_INC],
		    [cs_pyinc=`AS_DIRNAME([$cs_python_lib_dir])`
		    cs_pyinc=`AS_DIRNAME([$cs_pyinc])`
		    cs_python_inc_dir="$cs_pyinc/include/$cs_cv_python_base"])

		cs_cv_python_base_cflags="-I$cs_python_inc_dir"
		cs_cv_python_base_lflags="-L$cs_python_lib_dir
		    -L$cs_python_lib_dir/config"
		cs_cv_python_base_libs="$cs_pylibs $cs_pylibm $cs_pylibc"

		cs_cv_python_base_cflags=CS_TRIM([$cs_cv_python_base_cflags])
		cs_cv_python_base_lflags=CS_TRIM([$cs_cv_python_base_lflags])
		cs_cv_python_base_libs=CS_TRIM([$cs_cv_python_base_libs])
		cs_cv_python_sdk=yes])])])])

# Check if Python SDK is usable.  Some distributions (such as the one shipped
# with MacOS/X Jaguar which is missing the static link library) are broken.
# The most common library name is the basename with a few decorations (for
# example, "libpython2.2.a"), however some Windows libraries lack the decimal
# point (for example, "libpython22.a" or "python22.lib"), so we must check for
# both variations.
cs_pylib1="-l$cs_cv_python_base"
cs_pylib2=`echo "-l$cs_cv_python_base" | sed 's/\.//g'`
CS_CHECK_BUILD([if python SDK is usable], [cs_cv_python],
    [AC_LANG_PROGRAM([#include <Python.h>],[Py_Initialize(); Py_Finalize();])],
    [CS_CREATE_TUPLE([],[],[$cs_pylib1]) CS_CREATE_TUPLE([],[],[$cs_pylib2])],
    [], [CS_EMIT_BUILD_RESULT([cs_cv_python], [PYTHON])], [], [],
    [$cs_cv_python_base_cflags $cs_cv_sys_pthread_cflags],
    [$cs_cv_python_base_lflags $cs_cv_sys_pthread_lflags],
    [$cs_cv_python_base_libs   $cs_cv_sys_pthread_libs])



#------------------------------------------------------------------------------
# Use the CXXFLAGS and CFLAGS passed to configure Autoconf has defaults which
# are not suitable for CS, so we need to filter out all -O and -g flags.
#------------------------------------------------------------------------------
CFLAGS=`echo "$CFLAGS" | sed 's/-O.//g;s/-g.//g'`
CXXFLAGS=`echo "$CXXFLAGS" | sed 's/-O.//g;s/-g.//g'`

CS_BUILD_PROPERTY([CFLAGS.SYSTEM], [$CPPFLAGS $CFLAGS])
CS_BUILD_PROPERTY([CXXFLAGS.SYSTEM], [$CPPFLAGS $CXXFLAGS])
CS_BUILD_PROPERTY([LFLAGS.SYSTEM], [$LDFLAGS])



#------------------------------------------------------------------------------
# Check how to enable compilation warnings.  Note that on some platforms, it is
# more appropriate to use -Wmost rather than -Wall even if the compiler
# understands both, thus we attempt -Wmost before -Wall.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to enable compilation warnings],
    [cs_cv_prog_cxx_enable_warnings],
    [CS_CREATE_TUPLE([-Wmost]) CS_CREATE_TUPLE([-Wall])], [C++],
    [CFLAGS.SYSTEM], [+])



#------------------------------------------------------------------------------
# Check how to make compiler treat warnings as errors.
#------------------------------------------------------------------------------
AC_CACHE_CHECK([how to treat warnings as errors],
    [cs_cv_prog_cxx_enable_errors],
    [CS_BUILD_IFELSE([], [CS_CREATE_TUPLE([-Werror])], [C++],
	[cs_cv_prog_cxx_enable_errors=$cs_build_cflags],
	[cs_cv_prog_cxx_enable_errors=no])])



#------------------------------------------------------------------------------
# Check if warnings about unknown #pragmas can be disabled.  (MSVC and Borland
# use a number of pragmas not understood by GCC, for instance.)
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to disable unknown @%:@pragma warnings],
    [cs_cv_prog_cxx_disable_pragma_warnings],
    [CS_CREATE_TUPLE([-Wno-unknown-pragmas])], [C++], [CFLAGS.SYSTEM], [+])



#------------------------------------------------------------------------------
# Check if C++ exceptions can be disabled.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to disable C++ exceptions],
    [cs_cv_prog_cxx_disable_exceptions], [CS_CREATE_TUPLE([-fno-exceptions])],
    [C++], [CXXFLAGS.SYSTEM], [+])



#------------------------------------------------------------------------------
# Check if the linker recognizes "-multiply_defined suppress".  This is
# required for MacOS/X 10.1 to avoid warning messages when linking a program
# with "-framework Foundation" if that program does not actually employ any
# Objective-C.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([if -multiply_defined suppress is needed],
    [cs_cv_prog_link_suppress_multiple],
    [CS_CREATE_TUPLE([], [-multiply_defined suppress])], [],
    [MACOSX.LFLAGS.CONFIG], [+], [$cs_build_lflags])



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
	    CS_EMIT_BUILD_FLAGS([if -fvtable-thunks is needed],
		[cs_cv_prog_cxx_vtable_thunks],
		[CS_CREATE_TUPLE([-fvtable-thunks -Werror])],
		[C++], [CXXFLAGS.SYSTEM], [+], [-fvtable-thunks])
	    ;;
    esac])

CS_CHECK_VTABLE_THUNKS



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
    *) AC_MSG_ERROR([unrecognized option; use `no', `minimum', or `maximum'])
       ;;
esac

AS_IF([test $enable_cpu_specific_optimizations != no],
    [AC_CACHE_CHECK([for code generation flags], [cs_cv_prog_cxx_code_gen],
	[cs_cv_prog_cxx_code_gen=no
	AS_IF([test $enable_cpu_specific_optimizations = maximum],
	    [CS_CHECK_CODE_GEN_FLAG([athlon], [-mcpu=athlon -march=athlon])
	    CS_CHECK_CODE_GEN_FLAG([*686], [-mcpu=pentiumpro -march=i686])
	    CS_CHECK_CODE_GEN_FLAG([*686], [-mpentiumpro -march=i686])])
	CS_CHECK_CODE_GEN_FLAG([[*[5-6]86]], [-mcpu=pentium -march=i586])
	CS_CHECK_CODE_GEN_FLAG([[*[5-6]86]], [-mpentium -march=i586])
	CS_CHECK_CODE_GEN_FLAG([[*[3-9]86]], [-mcpu=i486])
	CS_CHECK_CODE_GEN_FLAG([[*[3-9]86]], [-m486])
	CS_CHECK_CODE_GEN_FLAG([alpha], [-mieee])])

    AS_IF([test "$cs_cv_prog_cxx_code_gen" != no],
	[CS_BUILD_PROPERTY(
	    [CFLAGS.SYSTEM], [$cs_cv_prog_cxx_code_gen], [+])])])



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
	[C++], [CFLAGS.optimize], [+], [],
	[cs_optimize_flags="$cs_optimize_flags $cs_cv_prog_cxx_optimize"])

    CS_EMIT_BUILD_FLAGS([if $CXX recognizes -fomit-frame-pointer],
	[cs_cv_prog_cxx_omit_frame_pointer],
	[CS_CREATE_TUPLE([-fomit-frame-pointer])],
	[C++], [CFLAGS.optimize], [+], [],
	[cs_optimize_flags="$cs_optimize_flags \
	    $cs_cv_prog_cxx_omit_frame_pointer"])

    CS_EMIT_BUILD_FLAGS([if $CXX recognizes -ffast-math],
	[cs_cv_prog_cxx_fast_math],
	[CS_CREATE_TUPLE([-ffast-math])], [C++], [CFLAGS.optimize], [+], [],
	[cs_optimize_flags="$cs_optimize_flags $cs_cv_prog_cxx_fast_math"])])

CS_CHECK_OPTIMIZE_FLAGS



#------------------------------------------------------------------------------
# Check for compiler debug flags.
#------------------------------------------------------------------------------
CS_EMIT_BUILD_FLAGS([how to enable compiler debug output],
    [cs_cv_prog_cxx_debug],
    [CS_CREATE_TUPLE([-g3]) CS_CREATE_TUPLE([-g2]) CS_CREATE_TUPLE([-g])],
    [C++], [CFLAGS.debug], [+])
AS_IF([test $cs_cv_prog_cxx_debug != no],
    [CS_BUILD_PROPERTY([LFLAGS.debug], [$cs_cv_prog_cxx_debug], [+])])



#------------------------------------------------------------------------------
# Check if compiler complains about adding /usr/local/include to the header
# search path.  If not, add it to the header search path since not all
# compilers search there by default.  Some compiler, on the other hand,
# complain if it is already in the built-in search path, and we would like to
# avoid the complaint, if possible.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LOCAL_INCLUDE],
    [AS_IF([test -d /usr/local/include],
	[AS_IF([test $cs_cv_prog_cxx_enable_warnings != no && \
	    test $cs_cv_prog_cxx_enable_errors != no],
	    [AC_CACHE_CHECK([if -I/usr/local/include is usable],
		[cs_cv_prog_cxx_local_include],
		[CS_BUILD_IFELSE([],
		    [CS_CREATE_TUPLE([-I/usr/local/include \
			$cs_cv_prog_cxx_enable_warnings \
			$cs_cv_prog_cxx_enable_errors],[],[])], [C++],
			[cs_cv_prog_cxx_local_include=yes],
			[cs_cv_prog_cxx_local_include=no])])],
	    [cs_cv_prog_cxx_local_include=yes])],
	[cs_cv_prog_cxx_local_include=no])
    
    AS_IF([test $cs_cv_prog_cxx_local_include = yes],
	[CS_BUILD_PROPERTY([CFLAGS.SYSTEM], [-I/usr/local/include], [+])
	cs_use_local_include=-I/usr/local/include],
	[cs_use_local_include=''])])

CS_CHECK_LOCAL_INCLUDE

AS_IF([test -d /usr/local/lib],
    [CS_BUILD_PROPERTY([LFLAGS.SYSTEM], [-L/usr/local/include], [+])
    cs_use_local_lib=-L/usr/local/lib],
    [cs_use_local_lib=''])



#------------------------------------------------------------------------------
# Determine host platform.  Recognized families: Unix, Windows, MacOS/X.
#
# CS_CHECK_HOST
#	Sets the shell variables cs_host_target, cs_host_makefile, and
#	cs_host_family.  Client code can use these variables to emit
#	appropriate TARGET and TARGET_MAKEFILE makefile variables, and OS_FOO
#	header define.
#------------------------------------------------------------------------------
cs_host_os_normalized=''
AC_DEFUN([CS_CHECK_HOST],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
	mingw*|cygwin*)
	    cs_host_target=win32gcc
	    cs_host_makefile=libs/cssys/win32/win32gcc.mak
	    cs_host_family=windows
	    ;;
	darwin*)
	    _CS_CHECK_HOST_DARWIN
	    ;;
	*)
	    # Everything else is assumed to be Unix or Unix-like.
	    cs_host_target=unix
	    cs_host_makefile=libs/cssys/unix/unix.mak
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
    # Darwin even though build host is MacOS/X.

    AC_CHECK_HEADER([Carbon/Carbon.h],
	[cs_host_macosx=yes], [cs_host_macosx=no])

    AS_IF([test $cs_host_macosx = yes],
	[AC_MSG_CHECKING([for --with-x])
	AS_IF([test "${with_x+set}" = set && test "$with_x" = "yes"],
	    [AC_MSG_RESULT([yes (assume Darwin)])
	    cs_host_macosx=no],
	    [AC_MSG_RESULT([no])])])

    AS_IF([test $cs_host_macosx = yes],
	[cs_host_target=macosx
	cs_host_makefile=libs/cssys/macosx/macosx.mak
	cs_host_family=unix
	cs_host_os_normalized='MacOS/X'

	CS_HEADER_PROPERTY([OS_MACOSX])
	CS_HEADER_PROPERTY([CS_24BIT_PIXEL_LAYOUT], [CS_24BIT_PIXEL_ARGB])

	# We assume that we always have CoreAudio available on MacOS/X.
	CS_JAMCONFIG_PROPERTY([COREAUDIO.AVAILABLE], [yes])
	CS_JAMCONFIG_PROPERTY([COREAUDIO.LFLAGS], [-framework CoreAudio])

	AC_CACHE_CHECK([for Objective-C compiler], [cs_cv_prog_objc],
	    [cs_cv_prog_objc="$CC"])
	CS_MAKEFILE_PROPERTY([OBJC], [$cs_cv_prog_objc -c])
	CS_JAMCONFIG_PROPERTY([CMD.OBJC], [$cs_cv_prog_objc])
	AC_CACHE_CHECK([for Objective-C++ compiler], [cs_cv_prog_objcxx],
	    [cs_cv_prog_objcxx="$CXX"])
	CS_MAKEFILE_PROPERTY([OBJCXX], [$cs_cv_prog_objcxx -c])
	CS_JAMCONFIG_PROPERTY([CMD.OBJC++], [$cs_cv_prog_objcxx])],

	[cs_host_target=unix
	cs_host_makefile=libs/cssys/unix/unix.mak
	cs_host_family=unix])])

CS_CHECK_HOST

case $cs_host_family in
    windows)
	CS_HEADER_PROPERTY([OS_WIN32])
	AS_IF([test -z "$cs_host_os_normalized"],
	    [cs_host_os_normalized='Win32'])
	;;
    unix)
	CS_HEADER_PROPERTY([OS_UNIX])
	AS_IF([test -z "$cs_host_os_normalized"],
	    [cs_host_os_normalized='Unix'])
	;;
esac

cs_host_os_normalized_uc="AS_TR_CPP([$cs_host_os_normalized])"
CS_MAKEFILE_PROPERTY([TARGET], [$cs_host_target])
CS_MAKEFILE_PROPERTY([TARGET_MAKEFILE], [$cs_host_makefile])
CS_MAKEFILE_PROPERTY([OS], [$cs_host_os_normalized_uc])
CS_JAMCONFIG_PROPERTY([TARGET.OS], [$cs_host_os_normalized_uc])
CS_JAMCONFIG_PROPERTY([TARGET.OS.NORMALIZED], [$cs_host_os_normalized])
CS_HEADER_PROPERTY([CS_PLATFORM_NAME],
    [AS_ESCAPE(["])$cs_host_os_normalized[]AS_ESCAPE(["])])



#------------------------------------------------------------------------------
# Check for typical required libraries (libm, libdl, libnsl).
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_LIBS],
    [AC_LANG_PUSH([C])
    AC_CHECK_LIB([m], [pow],
	[CS_BUILD_PROPERTY([LIBS.EXE], [-lm], [+])
	cs_cv_libm=yes cs_cv_libm_libs='-lm'],
	[cs_cv_libm=no cs_cv_libm_libs=''])
    AC_CHECK_LIB([dl], [dlopen],
	[CS_BUILD_PROPERTY([LIBS.EXE], [-ldl], [+])])
    AC_CHECK_LIB([nsl], [gethostbyname],
	[CS_BUILD_PROPERTY([LIBS.EXE], [-lnsl], [+])])
    AC_LANG_POP([C])])

CS_CHECK_COMMON_LIBS



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
    [CS_BUILD_PROPERTY([X11.AVAILABLE], [yes])
    CS_BUILD_PROPERTY([X_CFLAGS], [$X_CFLAGS])
    CS_BUILD_PROPERTY([X_LIBS], [$X_LIBS])
    CS_BUILD_PROPERTY([X_PRE_LIBS], [$X_PRE_LIBS])
    CS_BUILD_PROPERTY([X_EXTRA_LIBS], [$X_EXTRA_LIBS])
    CS_JAMCONFIG_PROPERTY([X11.CFLAGS], [$X_CFLAGS])
    CS_JAMCONFIG_PROPERTY([X11.LFLAGS],
      [AS_ESCAPE([$(X_PRE_LIBS) $(X_LIBS) -lXext -lX11 $(X_EXTRA_LIBS)])])

    CS_CHECK_LIB_WITH([Xxf86vm],
	[AC_LANG_PROGRAM(
	    [#define XK_MISCELLANY 1
	    #include <X11/Xlib.h>
	    #include <X11/Xutil.h>
	    #include <X11/Xatom.h>
	    #include <X11/extensions/xf86vmode.h>],
	    [XF86VidModeLockModeSwitch(0, 0, 0)])], [], [],
	[CS_BUILD_PROPERTY([USE_XFREE86VM], [yes])
	CS_JAMCONFIG_PROPERTY([XFREE86VM.CFLAGS], [$X_CFLAGS])
	CS_JAMCONFIG_PROPERTY([XFREE86VM.LFLAGS],
	    [AS_ESCAPE([$(X_PRE_LIBS) $(X_LIBS) -lXxf86vm -lXext -lX11 dnl
	    $(X_EXTRA_LIBS)])])], [],
	[$X_CFLAGS], [], [$X_PRE_LIBS $X_LIBS -lX11 -lXext $X_EXTRA_LIBS])])



#------------------------------------------------------------------------------
# Check for OpenGL and companions.  Also check if GLX extensions are supported
# and define the (backward) header property CSGL_EXT_STATIC_ASSERTION if they
# are _not_ supported.
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
# At least one user (Jorrit) has a strange installation in which inclusion of
# <windows.h> fails if an int32 is not present, thus we must take this into
# account.
#------------------------------------------------------------------------------
m4_define([cs_define_int32],
    [@%:@if !HAVE_TYPE_INT32
    typedef long int32;
    @%:@endif
    ])

AC_DEFUN([CS_CHECK_GL_PREPARE],
    [AC_REQUIRE([CS_CHECK_COMMON_LIBS])
    AC_REQUIRE([CS_CHECK_PTHREAD])
    AC_REQUIRE([AC_PATH_X])
    AC_REQUIRE([AC_PATH_XTRA])
    AC_CHECK_TYPE([int32], [AC_DEFINE([HAVE_TYPE_INT32])], [])
    AC_CHECK_HEADERS([windows.h], [], [], [cs_define_int32])])

# CS_GL_INCLUDE(CPP-MACRO,FALLBACK,HEADER)
AC_DEFUN([CS_GL_INCLUDE],
    [AC_REQUIRE([CS_CHECK_GL_PREPARE])
    @%:@if HAVE_WINDOWS_H
    @%:@if !HAVE_TYPE_INT32
    typedef long int32;
    @%:@endif
    @%:@include <windows.h>
    @%:@endif
    @%:@ifndef CS_HEADER_GLOBAL
    @%:@define CS_HEADER_GLOBAL(X,Y) CS_HEADER_GLOBAL_COMPOSE(X,Y)
    @%:@define CS_HEADER_GLOBAL_COMPOSE(X,Y) <X/Y>
    @%:@endif
    @%:@ifdef $1
    @%:@include CS_HEADER_GLOBAL($1,$3)
    @%:@else
    @%:@include <$2/$3>
    @%:@endif])

CS_CHECK_GL_PREPARE

# Apply plaform-specific flags if necessary.
cs_gl_plat_cflags=''
cs_gl_plat_lflags=''
cs_gl_plat_libs=''
AS_IF([test $cs_cv_libm = yes],
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
    CS_CREATE_TUPLE([-DCS_OPENGL_PATH=OpenGL],[],[-framework OpenGL]) \
    CS_CREATE_TUPLE([],[],[-lGL]) \
    CS_CREATE_TUPLE([],[],[-lgl]) \
    $cs_mesa_gl], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libgl], [GL])], [], [],
    [$cs_gl_plat_cflags], [$cs_gl_plat_lflags], [$cs_gl_plat_libs])

AS_IF([test $cs_cv_libgl = yes],
    [# Check for GLU.
    CS_CHECK_BUILD([for GLU], [cs_cv_libglu],
	[AC_LANG_PROGRAM(
	    [CS_GL_INCLUDE([CS_GL_PATH],[GL],[glu.h])], [gluNewQuadric()])],
	[CS_CREATE_TUPLE() \
	$cs_win32_glu \
	CS_CREATE_TUPLE([-DCS_GL_PATH=OpenGL],[],[-framework OpenGL]) \
	CS_CREATE_TUPLE([],[],[-lGLU]) \
	CS_CREATE_TUPLE([],[],[-lglu]) \
	$cs_mesa_glu], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libglu], [GLU])], [], [],
	[$cs_cv_libgl_cflags], [$cs_cv_libgl_lflags], [$cs_cv_libgl_libs])

    # Check for GLUT.
    CS_CHECK_BUILD([for GLUT], [cs_cv_libglut],
	[AC_LANG_PROGRAM(
	    [CS_GL_INCLUDE([CS_GLUT_PATH],[GL],[glut.h])], [glutSwapBuffers])],
	[CS_CREATE_TUPLE() \
	$cs_win32_glut \
	CS_CREATE_TUPLE([-DCS_GLUT_PATH=GLUT],[],[-framework GLUT]) \
	CS_CREATE_TUPLE([],[],[-lGLUT]) \
	CS_CREATE_TUPLE([],[],[-lglut])], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libglut], [GLUT])], [], [],
	[$cs_cv_libgl_cflags], [$cs_cv_libgl_lflags], [$cs_cv_libgl_libs])

    # Check for GLX.
    CS_CHECK_BUILD([for GLX], [cs_cv_libglx],
	[AC_LANG_PROGRAM([#include <GL/glx.h>], [glXWaitGL()])],
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
		[@%:@define GLX_GLXEXT_PROTOTYPES
		@%:@include <GL/glx.h>],
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
	[@%:@if !HAVE_TYPE_INT32
	typedef long int32;
	@%:@endif
	@%:@include <windows.h>
	@%:@include <ddraw.h>
	@%:@include <dsound.h>],
	[DirectDrawCreate(0,0,0); DirectSoundCreate(0,0,0)])],
    [CS_CREATE_TUPLE([],[],[-lddraw]) \
    CS_CREATE_TUPLE([-I/usr/include/directx],[],[-lddraw -ldsound])], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libdirectx], [DIRECTX])])



#------------------------------------------------------------------------------
# Check for zlib.  In addition to the standard installed locations, we also
# search CS/libs/zlib*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([z], [AC_LANG_PROGRAM([#include <zlib.h>], [zlibVersion()])],
    [$srcdir/libs/zlib*], [], [CS_EMIT_BUILD_RESULT([cs_cv_libz], [ZLIB])],
    [AC_MSG_ERROR([zlib is essential for building CS.])])



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
	[AC_LANG_PROGRAM([#include <png.h>],
	    [png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)])],
	[$srcdir/libs/libpng*], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libpng], [PNG])], [],
	[$cs_cv_libz_cflags $cs_cv_libm_cflags],
	[$cs_cv_libz_lflags $cs_cv_libm_lflags],
	[$cs_cv_libz_libs $cs_cv_libm_libs])])



#------------------------------------------------------------------------------
# Check for libjpeg.  In addition to the standard installed locations, we also
# search CS/libs/*jpeg*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([jpeg],
    [AC_LANG_PROGRAM(
	[#include <stddef.h>
	#include <stdio.h>
	#include <jpeglib.h>],
	[jpeg_create_compress(0)])],
    [$srcdir/libs/*jpeg*], [], [CS_EMIT_BUILD_RESULT([cs_cv_libjpeg], [JPEG])])



#------------------------------------------------------------------------------
# Check for libmng.  In addition to the standard installed locations, we also
# search CS/libs/libmng*.
#------------------------------------------------------------------------------
AS_IF([test "$cs_cv_libz" = yes && test "$cs_cv_libjpeg" = yes],
    [CS_CHECK_LIB_WITH([mng],
	[AC_LANG_PROGRAM([#include <libmng.h>], [mng_version_release()])],
	[$srcdir/libs/libmng*], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libmng], [MNG])], [],
	[$cs_cv_libjpeg_cflags $cs_cv_libz_cflags],
	[$cs_cv_libjpeg_lflags $cs_cv_libz_lflags],
	[$cs_cv_libjpeg_libs $cs_cv_libz_libs])])



#------------------------------------------------------------------------------
# Check for libmikmod.  In addition to the standard installed locations, we
# also search CS/libs/libmikmod*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([mikmod],
    [AC_LANG_PROGRAM(
	[#include <mikmod.h>],
	[MikMod_Init("mikmodtest"); MikMod_Exit();])],
    [$srcdir/libs/libmikmod*], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libmikmod], [MIKMOD])], [],
    [$cs_cv_sys_pthread_cflags],
    [$cs_cv_sys_pthread_lflags],
    [$cs_cv_sys_pthread_libs])



#------------------------------------------------------------------------------
# Check for libogg/libvorbis/libvorbisfile.  In addition to the standard
# installed locations, we also search CS/libs/libogg*, CS/libs/libvorbis.*,
# CS/libs/libvorbisfile*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([ogg],
    [AC_LANG_PROGRAM(
	[#include <ogg/ogg.h>],
	[ogg_sync_state state; ogg_sync_init(&state);])],
    [$srcdir/libs/libogg*], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libogg], [OGG])])

AS_IF([test "$cs_cv_libogg" = yes],
    [CS_CHECK_LIB_WITH([vorbis],
	[AC_LANG_PROGRAM([#include <vorbis/codec.h>],
	  [vorbis_info info; vorbis_info_init(&info);])],
	[$srcdir/libs/libvorbis.*], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libvorbis], [VORBIS])], [],
	[$cs_cv_libogg_cflags],
	[$cs_cv_libogg_lflags],
	[$cs_cv_libogg_libs])])

AS_IF([test "$cs_cv_libvorbis" = yes],
    [CS_CHECK_LIB_WITH([vorbisfile],
	[AC_LANG_PROGRAM(
	  [#include <vorbis/vorbisfile.h>
	   #include <stdio.h>],
	  [ OggVorbis_File file;
	    ov_test(stdin, &file, "", 0); ])],
	[$srcdir/libs/libvorbisfile*], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libvorbisfile], [VORBISFILE])], [],
	[$cs_cv_libogg_cflags $cs_cv_libvorbis_cflags],
	[$cs_cv_libogg_lflags $cs_cv_libvorbis_lflags],
	[$cs_cv_libogg_libs $cs_cv_libvorbis_libs])])


#------------------------------------------------------------------------------
# Check for libpgserver (PicoGUI server library).
# In addition to the standard installed locations,
# we also search CS/libs/libpgserver*
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([pgserver],
  [AC_LANG_PROGRAM([#include <pgserver/g_error.h>], [g_error e;])],
  [$srcdir/libs/libpgserver*], [],
  [CS_EMIT_BUILD_RESULT([cs_cv_libpgserver], [PGSERVER])])



#------------------------------------------------------------------------------
# Check for lib3ds.  In addition to the standard installed locations, we also
# search CS/libs/lib3ds*.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LIB3DS_PREPARE],
    [AC_REQUIRE([CS_CHECK_COMMON_LIBS])])

CS_CHECK_LIB3DS_PREPARE

CS_CHECK_LIB_WITH([3ds],
    [AC_LANG_PROGRAM([#include <lib3ds/file.h>],[lib3ds_file_new();])],
    [$srcdir/libs/lib3ds*], [], [], [], [], [], [$cs_cv_libm_libs])

AS_IF([test $cs_cv_lib3ds = yes],
    [CS_CHECK_BUILD([if lib3ds is sufficiently recent], [cs_cv_lib3ds_recent],
	[AC_LANG_PROGRAM([#include <lib3ds/io.h>], [lib3ds_io_free(0);])],
	[], [], [CS_EMIT_BUILD_RESULT([cs_cv_lib3ds], [3DS])], [], [],
	[$cs_cv_lib3ds_cflags], [$cs_cv_lib3ds_lflags], [$cs_cv_lib3ds_libs])])



#------------------------------------------------------------------------------
# Check for libode.  In addition to the standard installed locations, we also
# search CS/libs/libode*.  Also check if it is a sufficiently recent version
# (0.035 as of this writing).
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([ode],
    [AC_LANG_PROGRAM([#include <ode/ode.h>], [dWorldCreate()])],
    [$srcdir/libs/libode*], [C++])

AS_IF([test $cs_cv_libode = yes],
    [CS_CHECK_BUILD([if libode is sufficiently recent], [cs_cv_libode_recent],
	[AC_LANG_PROGRAM(
	    [#include <ode/ode.h>],
	    [dWorldCreate();
	    dBodySetGravityMode(0,0);
	    dCloseODE();])],
	[], [C++], [CS_EMIT_BUILD_RESULT([cs_cv_libode], [ODE])], [], [],
	[$cs_cv_libode_cflags], [$cs_cv_libode_lflags], [$cs_cv_libode_libs])])



#------------------------------------------------------------------------------
# Check for libopenal.
#------------------------------------------------------------------------------
# CS_AL_INCLUDE(CPP-MACRO,FALLBACK,HEADER)
AC_DEFUN([CS_AL_INCLUDE],
    [@%:@ifndef CS_HEADER_GLOBAL
    @%:@define CS_HEADER_GLOBAL(X,Y) CS_HEADER_GLOBAL_COMPOSE(X,Y)
    @%:@define CS_HEADER_GLOBAL_COMPOSE(X,Y) <X/Y>
    @%:@endif
    @%:@ifdef $1
    @%:@include CS_HEADER_GLOBAL($1,$3)
    @%:@else
    @%:@include <$2/$3>
    @%:@endif])

AC_DEFUN([CS_CHECK_LIBOPENAL],
    [AC_REQUIRE([CS_CHECK_LOCAL_INCLUDE])
    CS_CHECK_BUILD([for OpenAL], [cs_cv_libopenal],
	[AC_LANG_PROGRAM(
	    [CS_AL_INCLUDE([CS_OPENAL_PATH],[AL],[alut.h])],
	    [alutInit(0,0);])],
	[CS_CREATE_TUPLE([-DCS_OPENAL_PATH=OpenAL],[],[-framework OpenAL]) \
	CS_CREATE_TUPLE([],[],[-lalut -lopenal -ldsound]) \
	CS_CREATE_TUPLE([],[],[-lalut -lopenal]) \
	CS_CREATE_TUPLE([],[],[-lopenal -ldsound]) \
	CS_CREATE_TUPLE([],[],[-lopenal])], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libopenal], [OPENAL])], [], [],
	[$cs_use_local_include $cs_cv_sys_pthread_cflags],
	[$cs_use_local_lib $cs_cv_sys_pthread_lflags],
	[$cs_cv_sys_pthread_libs])])

CS_CHECK_LIBOPENAL



#------------------------------------------------------------------------------
# Check for FreeType2.
#------------------------------------------------------------------------------
AC_CHECK_FT2([2.0],
    [CS_BUILD_PROPERTY([FT2.AVAILABLE], [yes])
    CS_BUILD_PROPERTY([FT2.LFLAGS], [$FT2_LIBS])
    CS_BUILD_PROPERTY([FT2.CFLAGS], [$FT2_CFLAGS])])



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
		    [AC_LANG_PROGRAM([#include <dirent.h>], [opendir("");])],
		    [CS_CREATE_TUPLE() CS_CREATE_TUPLE([],[],[-lmingwex])], [],
		    [AS_IF([test -n "$cs_build_libs"],
			[cs_cv_libmingwex=yes], [cs_cv_libmingwex=no])],
		    [cs_cv_libmingwex=no])])
	    ;;
	*) cs_cv_libmingwex=no ;;
    esac])

CS_CHECK_LIBMINGWX
AS_IF([test $cs_cv_libmingwex = yes],
    [CS_BUILD_PROPERTY([LIBS.SYSTEM], [-lmingwex], [+])])



#------------------------------------------------------------------------------
# Check for svgalib.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([vga],
    [AC_LANG_PROGRAM([#include <vga.h>], [vga_init()])], [], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libvga], [SVGA])
	CS_HEADER_PROPERTY([CS_HAS_SVGALIB])],
    [], [], [], [-lvgagl])



#------------------------------------------------------------------------------
# Check for cal3d.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([cal3d],
    [AC_LANG_PROGRAM([#include <cal3d/cal3d.h>],
	[CalModel m; m.create(0); m.destroy();])], [], [C++])

AS_IF([test $cs_cv_libcal3d = yes],
    [CS_CHECK_BUILD([if cal3d is sufficiently recent], [cs_cv_cal3d_recent],
	[AC_LANG_PROGRAM(
	    [#include <cal3d/cal3d.h>
	    @%:@include <string>],
	    [std::string s(""); CalCoreModel m; m.addAnimHelper(s,0);])],
	[], [C++],
	[CS_EMIT_BUILD_RESULT([cs_cv_libcal3d], [CAL3D])
	    CS_MAKEFILE_PROPERTY([HAS_CAL3D], [yes])],
	[], [],
	[$cs_cv_libcal3d_cflags],
	[$cs_cv_libcal3d_lflags],
	[$cs_cv_libcal3d_libs])])

    

#------------------------------------------------------------------------------
# Check for SDL.
#------------------------------------------------------------------------------
AC_CHECK_PROGS([SDLCONFIG], [sdl12-config sdl11-config sdl-config])
AS_IF([test -n "$SDLCONFIG"],
    [cs_cv_libsdl=yes
    cs_cv_libsdl_cflags=`AC_RUN_LOG([$SDLCONFIG --cflags])`
    cs_cv_libsdl_lflags=''
    cs_cv_libsdl_libs=`AC_RUN_LOG([$SDLCONFIG --libs])`
    CS_EMIT_BUILD_RESULT([cs_cv_libsdl], [SDL])
    CS_MAKEFILE_PROPERTY([HAS_SDL], [yes])])



#------------------------------------------------------------------------------
# Check if Linux joystick API is present.
#------------------------------------------------------------------------------
CS_CHECK_BUILD([for Linux joystick API], [cs_cv_sys_linux_joystick],
    [AC_LANG_PROGRAM(
	[#include <fcntl.h>
	@%:@include <linux/joystick.h>],
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
    [CS_BUILD_PROPERTY([LINUXJOYSTICK.AVAILABLE], [yes])])



#------------------------------------------------------------------------------
# Check if libbfd is present (part of GNU binutils).
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([bfd],
    [AC_LANG_PROGRAM([#include <bfd.h>], [bfd_init();])], [], [],
    [CS_EMIT_BUILD_RESULT([cs_cv_libbfd], [LIBBFD])
    CS_BUILD_PROPERTY([LIBS.EXE], [$cs_cv_libbfd_libs], [+])],
    [], [], [], [-liberty])



#------------------------------------------------------------------------------
# Check if objcopy is present (part of GNU binutils).
#------------------------------------------------------------------------------
AC_CHECK_PROGS([OBJCOPY], [objcopy])
AS_IF([test -n "$OBJCOPY"],
    [CS_BUILD_PROPERTY([OBJCOPY.AVAILABLE], [yes])
    CS_BUILD_PROPERTY([CMD.OBJCOPY], [$OBJCOPY])])



#------------------------------------------------------------------------------
# Determine if plugin modules must implement main().  This is needed on some
# platforms to ensure that global constructors and functions which initialize
# global variables in the plugin module get invoked.
#
# @@@ FIXME: Currently, we answer this question by making the (potentially
# incorrect) assumption that plugins on platforms other than Windows, MacOS/X,
# and Darwin require main().  In the future, we should use a proper test to
# make this determination.
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
# Determine host CPU.
#
# CS_CHECK_HOST_CPU
#	Set the shell variable cs_host_cpu to a normalized form of the CPU name
#	returned by config.guess/config.sub.  Also takes the normalized name,
#	uppercases and appends it to the string "PROC_" to form a name suitable
#	for the C preprocessor.  Assigns this value to the shell variable
#	cs_host_cpu_cpp_define.  Typically, Crystal Space's conception of CPU
#	name is the same as that returned by config.guess/config.sub, but there
#	may be exceptions as seen in the `case' statement.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_HOST_CPU],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_cpu in
	[[Ii][3-9]86*|[Xx]86*]) cs_host_cpu=x86 ;;
	*) cs_host_cpu=$host_cpu ;;
    esac
    cs_host_cpu_normalized="AS_TR_CPP([$cs_host_cpu])"])

CS_CHECK_HOST_CPU
CS_BUILD_PROPERTY([PROC], [$cs_host_cpu_normalized])
CS_HEADER_PROPERTY([PROC_$cs_host_cpu_normalized])
CS_HEADER_PROPERTY([CS_PROCESSOR_NAME],
    [AS_ESCAPE(["])$cs_host_cpu[]AS_ESCAPE(["])])

AS_IF([test $cs_host_cpu = x86], [CS_HEADER_PROPERTY([DO_MMX])])



#------------------------------------------------------------------------------
# Check host endianess.
#------------------------------------------------------------------------------
AC_C_BIGENDIAN(
    [CS_HEADER_PROPERTY([CS_BIG_ENDIAN])],
    [CS_HEADER_PROPERTY([CS_LITTLE_ENDIAN])])



#------------------------------------------------------------------------------
# Check if C++ compiler has a built-in `bool' type.
#------------------------------------------------------------------------------
AC_LANG_PUSH([C++])
AC_CHECK_TYPE([bool], [], [CS_HEADER_PROPERTY([CS_USE_FAKE_BOOL_TYPE])])
AC_LANG_POP([C++])



#------------------------------------------------------------------------------
# Check if stdint.h is available and if the INT64_C() macro is available.
#------------------------------------------------------------------------------
AC_CHECK_HEADER([stdint.h],
    [cs_cv_header_stdint_h=yes], [cs_cv_header_stdint_h=no])

AS_IF([test $cs_cv_header_stdint_h = yes],
    [CS_CHECK_BUILD([if INT64_C is available], [cs_cv_c_int64],
        [AC_LANG_PROGRAM(
            [#define __STDC_CONSTANT_MACROS
            #define __STDC_LIMIT_MACROS
            #include <stdint.h>],
            [(void)INT64_C(0x100000000)])],
        [], [C++], [CS_HEADER_PROPERTY([CS_HAS_STDINT_H])])])



#------------------------------------------------------------------------------
# Check for sizeof (long) to determine whether the platform is 32 or 64 bit.
#------------------------------------------------------------------------------
AC_CHECK_SIZEOF([long])
AS_IF([test $ac_cv_sizeof_long = 8],
    [CS_HEADER_PROPERTY([CS_PLATFORM_IS_64BITS])])



#------------------------------------------------------------------------------
# Check if the type wchar_t is available and determine its size.  It might be
# built-in, or it might come from the wchar.h header file.  Also determine
# availability of wsclen().
#------------------------------------------------------------------------------
AC_LANG_PUSH([C++])

AC_CHECK_HEADER([wchar.h], [CS_HEADER_PROPERTY([CS_HAS_WCHAR_H])])

AC_CHECK_TYPE([wchar_t], [cs_type_wchar_t=yes], [cs_type_wchar_t=no],
    [#if HAVE_WCHAR_H
    #include <wchar.h>
    @%:@endif])

AS_IF([test $cs_type_wchar_t = yes],
    [AC_CHECK_FUNC([wcslen], [], [CS_HEADER_PROPERTY([CS_USE_FAKE_WCSLEN])])
    AC_CHECK_SIZEOF([wchar_t], []
	[#if HAVE_WCHAR_H
	#include <wchar.h>
	@%:@endif])
    CS_HEADER_PROPERTY([CS_WCHAR_T_SIZE], [$ac_cv_sizeof_wchar_t])],
    [CS_HEADER_PROPERTY([CS_USE_FAKE_WCHAR_TYPE])])

AC_LANG_POP([C++])



#------------------------------------------------------------------------------
# Check for audio support.
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
    [CS_BUILD_PROPERTY([OSS.AVAILABLE], [yes])])



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
	[#if HAVE_WINSOCK_H
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
	@%:@endif],
	[struct in_addr a; inet_ntoa(a); socket(0,0,0);])],
	[cs_socket_flags], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_sys_socket], [SOCKET])])

AS_IF([test $cs_cv_sys_socket = yes],
    [AC_CHECK_TYPE([socklen_t], [], [CS_HEADER_PROPERTY([socklen_t],[int])],
	[#if HAVE_UNISTD_H
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
	@%:@endif])])



#------------------------------------------------------------------------------
# Check if C++ compiler understands new C++ `explicit' keyword.
#------------------------------------------------------------------------------
AC_CACHE_CHECK([for C++ `explicit' keyword], [cs_cv_cxx_explicit],
    [CS_BUILD_IFELSE(
	[AC_LANG_PROGRAM([class A { public: explicit A(int); };], [])],
	[], [C++], [cs_cv_cxx_explicit=yes], [cs_cv_cxx_explicit=no])])

AS_IF([test $cs_cv_cxx_explicit = no],
    [CS_HEADER_PROPERTY([CS_USE_FAKE_EXPLICIT_KEYWORD])])



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
# CS_CHECK_QSQRT
#	Check if gcc is able to understand CS/include/qsqrt.h.  This test will
#	catch not only versions of gcc which do not understand the assembly
#	content, but also buggy versions which fail with an internal error on
#	this file (such as the RedHat 7 gcc).  Furthermore, it is reported that
#	some versions of gcc 3.2.x cause qsqrt() to return bogus results when
#	compiled in optimize mode.  It is reported that this same compiler
#	crashes with an internal error when -fPIC is used, so we use -fPIC as a
#	way to detect this buggy compiler.  Sets the shell variable
#	cs_cv_cxx_qsqrt_ok to "yes" if qsqrt.h is usable, otherwise "no".
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_QSQRT],
    [AC_REQUIRE([CS_CHECK_HOST_CPU])
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([CS_CHECK_OPTIMIZE_FLAGS])
    AS_IF([test $ac_compiler_gnu = yes],
	[AC_CACHE_CHECK([if $CXX recognizes -fPIC], [cs_cv_prog_cxx_fpic],
	    [CS_BUILD_IFELSE([], [CS_CREATE_TUPLE([-fPIC])], [C++],
	    [cs_cv_prog_cxx_fpic=-fPIC], [cs_cv_prog_cxx_fpic=no])])
	AC_CACHE_CHECK([if qsqrt() compiles], [cs_cv_cxx_qsqrt_ok],
	    [cs_qsqrt_cflags="$cs_optimize_flags"
	    AS_IF([test $cs_cv_prog_cxx_fpic != no],
		[cs_qsqrt_cflags="$cs_qsqrt_cflags $cs_cv_prog_cxx_fpic"])
	    CS_BUILD_IFELSE(
		[AC_LANG_PROGRAM(
		    [#define PROC_$cs_host_cpu_normalized
		    #define COMP_GCC
		    #include "include/qsqrt.h"],
		    [float n = qsqrt(1.0f); (void)n;])],
		[CS_CREATE_TUPLE([$cs_qsqrt_cflags])], [C++],
		[cs_cv_cxx_qsqrt_ok=yes], [cs_cv_cxx_qsqrt_ok=no])])],
	[cs_cv_cxx_qsqrt_ok=no])])

CS_CHECK_QSQRT
AS_IF([test $cs_cv_cxx_qsqrt_ok = no],
    [CS_HEADER_PROPERTY([CS_NO_QSQRT])])



#------------------------------------------------------------------------------
# Check if a compiler bug discovered in gcc 2.96 (RedHat and other unstable
# versions) and gcc 3.0.1 causes qint() to misbehave.
# -----------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_QINT],
    [AC_REQUIRE([CS_CHECK_OPTIMIZE_FLAGS])
    AC_CACHE_CHECK([if qint() functions correctly], [cs_cv_cxx_qint_ok],
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
	AC_LANG_POP([C++])])])

CS_CHECK_QINT
AS_IF([test $cs_cv_cxx_qint_ok = no],
    [CS_HEADER_PROPERTY([CS_QINT_WORKAROUND])])



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
	*) AC_MSG_ERROR([m4_text_wrap(
	    [unrecognized mode --enable-mode=$enableval;
	    use `optimize', `debug', or `profile'], [    ], [[]], [60])]) ;;
    esac])

AC_MSG_CHECKING([build mode])
AS_VAR_SET_IF([cs_build_mode], [], [cs_build_mode=optimize])
AC_MSG_RESULT([$cs_build_mode])
CS_BUILD_PROPERTY([MODE], [$cs_build_mode])



#------------------------------------------------------------------------------
# Determine if modules should be built as plugins or static link libraries.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([if building plugins])
AC_ARG_ENABLE([plugins], [AC_HELP_STRING([--enable-plugins],
    [create plugin modules instead of static link libraries (default YES)])],
    [], [enable_plugins=yes])
AC_MSG_RESULT([$enable_plugins])
CS_BUILD_PROPERTY([USE_PLUGINS], [$enable_plugins])
AS_IF([test $enable_plugins != yes], [CS_HEADER_PROPERTY([CS_STATIC_LINKED])])



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
CS_BUILD_PROPERTY([EXTENSIVE_MEMDEBUG],
    [$enable_extensive_memory_debugger])
AS_IF([test $enable_extensive_memory_debugger = yes],
    [CS_HEADER_PROPERTY([CS_EXTENSIVE_MEMDEBUG])])



#------------------------------------------------------------------------------
# Determine whether the new renderer should be activated.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([whether to use new renderer])
AC_ARG_ENABLE([new-renderer], [AC_HELP_STRING([--enable-new-renderer],
    [enable use of new renderer (_EXPERIMENTAL_!) (default NO)])],
    [], [enable_new_renderer=no])
AC_MSG_RESULT([$enable_new_renderer])
CS_BUILD_PROPERTY([USE_NEW_RENDERER], [$enable_new_renderer])
AS_IF([test $enable_new_renderer = yes], 
  [CS_HEADER_PROPERTY([CS_USE_NEW_RENDERER])])



#------------------------------------------------------------------------------
# Determine installation directory for GNU make build system.
#------------------------------------------------------------------------------
AS_IF([test $prefix = NONE],
    [cs_install_dir=cs_install_dir_default], [cs_install_dir=$prefix])
CS_MAKEFILE_PROPERTY([INSTALL_DIR], [$cs_install_dir])



#------------------------------------------------------------------------------
# Determine installation directories for Jam build system.
#------------------------------------------------------------------------------
CS_OUTPUT_INSTALLDIRS



#------------------------------------------------------------------------------
# Determine if makefile cache should be used and monitored.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([whether to cache makefiles])
AC_ARG_ENABLE([makefile-cache],
    [AC_HELP_STRING([--enable-makefile-cache],
	[enable caching of makefile information to speed up build process;
	(default YES)])],
    [], [enable_makefile_cache=yes])
AC_MSG_RESULT([$enable_makefile_cache])
CS_MAKEFILE_PROPERTY([USE_MAKEFILE_CACHE], [$enable_makefile_cache])

AS_IF([test $enable_makefile_cache = yes],
    [AC_MSG_CHECKING([whether to auto-refresh cached makefiles])
    AC_ARG_ENABLE([makefile-cache-monitor],
	[AC_HELP_STRING([--enable-makefile-cache-monitor],
	    [auto-refresh makefile cache when source makefiles change
	    (default YES); auto-refresh entails very slight performance
	    penalty but is more robust])],
	[], [enable_makefile_cache_monitor=yes])
    AC_MSG_RESULT([$enable_makefile_cache_monitor])],
    [enable_makefile_cache_monitor=no])
CS_MAKEFILE_PROPERTY([MONITOR_MAKEFILE_CACHE],
    [$enable_makefile_cache_monitor])



#------------------------------------------------------------------------------
# Boilerplate: volatile.h
#------------------------------------------------------------------------------
CS_HEADER_PREPEND(
[// File generated automatically.
#ifndef __CS_VOLATILE_H__
#define __CS_VOLATILE_H__
])

CS_HEADER_APPEND(
[#endif // __CS_VOLATILE_H__
])



#------------------------------------------------------------------------------
# Boilerplate: config.mak
#------------------------------------------------------------------------------
CS_EMIT_PACKAGEINFO([make])
CS_MAKEFILE_PROPERTY([SRCDIR], [$srcdir])

# Hack: The makefile system presently assumes that CFLAGS.SYSTEM should be
# used for both C and C++, and ignores CXXFLAGS.SYSTEM.  Eliminate this hack
# when the makefile system has been repaired.
CS_MAKEFILE_PROPERTY([CFLAGS.SYSTEM], [AS_ESCAPE([$(CXXFLAGS.SYSTEM)])], [+])



#------------------------------------------------------------------------------
# Boilerplate: Jamconfig
#------------------------------------------------------------------------------
CS_EMIT_PACKAGEINFO([jam])
CS_JAMCONFIG_PROPERTY([JAMCONFIG_READ], [yes])



#------------------------------------------------------------------------------
# Output generated files.
#------------------------------------------------------------------------------
CS_HEADER_OUTPUT([include/volatile.h])
CS_MAKEFILE_OUTPUT([config.mak])
CS_JAMCONFIG_OUTPUT([Jamconfig])
AC_CONFIG_FILES([Makefile Jamfile])

# Queue makefile cache creation if caching enabled.
AS_IF([test $enable_makefile_cache = yes],
    [AC_CONFIG_COMMANDS([cache.mak],
	[rm -f cache.mak ; ${MAKE} recache], [MAKE=${MAKE-make}])])

AC_OUTPUT
