# checkpython.m4                                               -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003,2004 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_CHECK_PYTHON([EMITTER], [SDK-CHECK-DEFAULT], [WITH-DESCRIPTION])
#	Check for Python and a working Python SDK.  Sets the shell variable
#	PYTHON to the name of the Python interpreter and invokes AC_SUBST().
#	The shell variable cs_cv_python is set to "yes" if a working Python SDK
#	is discovered, else "no". If available, then the variables
#	cs_cv_python_cflags, cs_cv_python_lflags, and cs_cv_python_libs are
#	set. (As a convenience, these variables can be emitted to an output
#	file with CS_EMIT_BUILD_RESULT() by passing "cs_cv_python" as its
#	CACHE-VAR argument.) As a convenience, the shell variable
#	cs_cv_python_ext is set to the suffix of Python extension modules (with
#	leading dot; typically ".dll" or ".so").  The SDK check can be enabled
#	or disabled with --with[out]-python.  SDK-CHECK-DEFAULT should be
#	"with" or "without". If SDK-CHECK-DEFAULT is "with" or if it is
#	ommitted, then --with-python is the default, else --without-python is
#	the default.  WITH-DESCRIPTION is the description to use for the
#	--with[out]-python option. The literal string "use" (or "do not use")
#	is prepended to WITH-DESCRIPTION. If omitted, WITH-DESCRIPTION defaults
#	to "Python".  If EMITTER is provided, then CS_EMIT_BUILD_RESULT() is
#	invoked with EMITTER in order to record the results in an output
#	file. As a convenience, if EMITTER is the literal value "emit" or
#	"yes", then CS_EMIT_BUILD_RESULT()'s default emitter will be used.
#	When EMITTER is provided, the following properties are emitted to the
#	output file: PTYHON (the actual interpreter), PYTHON.AVAILABLE ("yes"
#	or "no"), PYTHON.CFLAGS, PYTHON.LFLAGS, and PYTHON.MODULE_EXT.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_PYTHON],
    [AC_REQUIRE([CS_CHECK_PTHREAD])
    CS_CHECK_SUPPRESS_LONG_DOUBLE_WARNINGS

    AC_ARG_WITH([python],
	[AC_HELP_STRING([--]m4_if([$2],[without],[with],[without])[-python],
	    m4_if([$2],[without],[use],[do not use])
		m4_default([$3],[Python]))])
    AS_IF([test -z "$with_python"],
	[with_python=m4_if([$2], [without], [no], [yes])])

    CS_CHECK_PROGS([PYTHON], [python])
    AC_SUBST([PYTHON])
    CS_EMIT_BUILD_PROPERTY([PYTHON],[$PYTHON],[],[],CS_EMITTER_OPTIONAL([$1]))

    AS_IF([test -n "$PYTHON" && test "$with_python" != no],
	[AC_CACHE_CHECK([for python SDK], [cs_cv_python_sdk],
	    [cs_pyver=`AC_RUN_LOG([$PYTHON -c 'import sys, string; \
		print string.join(map(str,sys.version_info[[:2]]),".")'])`
	    cs_cv_pybase="python${cs_pyver}"

	    cs_cv_pybase_cflags=CS_RUN_PATH_NORMALIZE([$PYTHON -c \
		'import distutils.sysconfig; \
		print "-I" + distutils.sysconfig.get_python_inc()'])
	    cs_cv_pybase_cflags="$cs_cv_pybase_cflags \
		$cs_cv_prog_cxx_ignore_long_double"

	    # Depending upon platform and installation, link library might
	    # reside in "${prefix}/lib", "get_python_lib()/config", or
	    # "${prefix}/libs" on Windows.
	    cs_cv_pybase_lflags=CS_RUN_PATH_NORMALIZE([$PYTHON -c \
		'import sys,distutils.sysconfig; \
		print "-L" + distutils.sysconfig.get_python_lib(0,1) + \
		    " -L"+distutils.sysconfig.get_python_lib(0,1)+"/config" + \
		    " -L"+sys.prefix + "/lib" + " -L"+sys.prefix + "/libs"'])

	    cs_cv_pybase_libs=CS_RUN_PATH_NORMALIZE([$PYTHON -c \
		'import distutils.sysconfig; \
		print (distutils.sysconfig.get_config_var("LIBS") or "")+" "+ \
		      (distutils.sysconfig.get_config_var("SYSLIBS") or "")'])

	    cs_cv_python_ext=`AC_RUN_LOG([$PYTHON -c \
		'import distutils.sysconfig; \
		print (distutils.sysconfig.get_config_var("SO") or "")'])`
	    CS_EMIT_BUILD_PROPERTY([PYTHON.MODULE_EXT], [$cs_cv_python_ext],
		[], [], CS_EMITTER_OPTIONAL([$1]))

	    AS_IF([test -n "$cs_pyver" &&
		   test -n "$cs_cv_pybase_cflags" &&
		   test -n "$cs_cv_pybase_lflags"],
		[cs_cv_python_sdk=yes], [cs_cv_python_sdk=no])])

	# Check if Python SDK is usable.  The most common library name is the
	# basename with a few decorations (for example, libpython2.2.a),
	# however some Windows libraries lack the decimal point (for example,
	# libpython22.a or python22.lib), so we must check for both variations.
	# Furthermore, MacOS/X 10.3 supplies a Python.framework, however,
	# earlier releases did not.  Instead, Python on MacOS/X pre-10.3 uses a
	# one-level linker namespace, which means that loadable Python modules
	# do not link against the Python library; instead, unresolved symbols
	# in the modules are satisfied automatically by the Python executable
	# when the module is loaded into the executable.  For this reason,
	# Python on MacOS/X does not even provide a Python link library.  We
	# account for this by trying -bundle, rather than linking against the
	# library.
	AS_IF([test $cs_cv_python_sdk = yes],
	    [cs_pywinlib=`echo "$cs_cv_pybase" | sed 's/\.//g'`
	    cs_pyflags="$cs_pyflags CS_CREATE_TUPLE([],[],[-framework Python])"
	    cs_pyflags="$cs_pyflags CS_CREATE_TUPLE([],[],[-l$cs_cv_pybase])"
	    cs_pyflags="$cs_pyflags CS_CREATE_TUPLE([],[],[-l$cs_pywinlib])"
	    cs_pyflags="$cs_pyflags CS_CREATE_TUPLE(
	    [],[-bundle -flat_namespace -undefined suppress])"
	    CS_CHECK_BUILD([if python SDK is usable], [cs_cv_python],
		[AC_LANG_PROGRAM([[#include <Python.h>]],
		    [Py_Initialize(); Py_Finalize();])],
		[$cs_pyflags], [],
		[CS_EMIT_BUILD_RESULT([cs_cv_python], [PYTHON],
		    CS_EMITTER_OPTIONAL([$1]))], [], [],
		[$cs_cv_pybase_cflags $cs_cv_sys_pthread_cflags],
		[$cs_cv_pybase_lflags $cs_cv_sys_pthread_lflags],
		[$cs_cv_pybase_libs   $cs_cv_sys_pthread_libs])])],
	    [cs_cv_python=no])],
	[cs_cv_python=no])])



#------------------------------------------------------------------------------
# CS_EMIT_CHECK_PYTHON([SDK-CHECK-DEFAULT], [WITH-DESCRIPTION], [EMITTER])
#	DEPRECATED: Previously, layered EMITTER functionality atop
#	CS_CHECK_PYTHON() before CS_CHECK_PYTHON() supported emitters directly.
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_CHECK_PYTHON],
    [CS_CHECK_PYTHON(m4_ifval([$3], [$3], [emit]), [$1], [$2])])



#------------------------------------------------------------------------------
# CS_CHECK_SUPPRESS_LONG_DOUBLE_WARNINGS
#	Some Python installations on MacOS/X uses of 'long double' in the
#	headers which cause excessive warnings from the compiler. Check if
#	those warnings can be suppressed.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_SUPPRESS_LONG_DOUBLE_WARNINGS],
    [CS_CHECK_BUILD_FLAGS([how to suppress 'long double' warnings],
	[cs_cv_prog_cxx_ignore_long_double],
	[CS_CREATE_TUPLE([-Wno-long-double])], [C++])])
