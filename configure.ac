#                                               -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
# Autoconf input script for Crystal Space.  Process this file with Autoconf
# to produce a configure script.
#==============================================================================

AC_PREREQ([2.56])


#------------------------------------------------------------------------------
# cs_version_number
#	Keep this value synchronized with CS/include/csver.h.
#------------------------------------------------------------------------------
m4_define([cs_version_number], [0.95])



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
#	/sw -- Fink, the MacOS/X manager of Unix packages installs here by
#		default.
#------------------------------------------------------------------------------
m4_define([cs_lib_paths_default],
    [/usr/local/include|/usr/local/lib /sw/include|/sw/lib])



#------------------------------------------------------------------------------
# Replacement for AS_MKDIR_P() from m4sugar/m4sh.m4 which fixes two problems
# which are present at least as recently as Autoconfg 2.57.
#
# 1) Removes bogus "-p" directory which the stock AS_MKDIR_P() leaves laying
#    around in the working directory if the mkdir command does not recognize
#    the -p option.
# 2) Takes advantage of the older "mkdirs" program if it exists and if "mkdir
#    -p" does not work.
#------------------------------------------------------------------------------
m4_defun([_AS_MKDIR_P_PREPARE],
[if mkdir -p . 2>/dev/null; then
  as_mkdir_p='mkdir -p'
elif mkdirs . 2>/dev/null; then
  as_mkdir_p='mkdirs'
else
  as_mkdir_p=''
fi
test -r ./-p && rm -rf ./-p
])# _AS_MKDIR_P_PREPARE

m4_define([AS_MKDIR_P],
[AS_REQUIRE([_$0_PREPARE])dnl
{ if test -n "$as_mkdir_p"; then
    $as_mkdir_p $1
  else
    as_dir=$1
    as_dirs=
    while test ! -d "$as_dir"; do
      as_dirs="$as_dir $as_dirs"
      as_dir=`AS_DIRNAME("$as_dir")`
    done
    test ! -n "$as_dirs" || mkdir $as_dirs
  fi || AS_ERROR([cannot create directory $1]); }
])# AS_MKDIR_P



#------------------------------------------------------------------------------
# CS_TRIM(STRING)
#	Strip leading and trailing spaces from STRING and collapse internal
#	runs of multiple spaces to a single space.
#------------------------------------------------------------------------------
AC_DEFUN([CS_TRIM], [`echo x$1 | sed 's/^x//;s/   */ /g;s/^ //;s/ $//'`])



#------------------------------------------------------------------------------
# Text cache facility.  These macros provide a way to incrementally store
# arbitrary text in a shell variable, and to write the saved text to a file.
#
# CS_TEXT_CACHE_APPEND(VARIABLE, TEXT)
#	Append text to the contents of the named shell variable.  If the text
#	contains references to shell variables (such as $foo), then those
#	references will be expanded.  If expansion is not desired, then protect
#	the text with AS_ESCAPE().
#
# CS_TEXT_CACHE_PREPEND(VARIABLE, TEXT)
#	Prepend text verbatim to the contents of the named shell variable.  If
#	the text contains references to shell variables (such as $foo), then
#	those references will be expanded.  If expansion is not desired, then
#	protect the text with AS_ESCAPE().
#
# CS_TEXT_CACHE_OUTPUT(VARIABLE, FILENAME)
#	Instruct config.status to write the contents of the named shell
#	variable to the given filename.  If the file resides in a directory,
#	the directory will be created, if necessary.  If the output file
#	already exists, and if the cached text is identical to the contents of
#	the existing file, then the existing file is left alone, thus its time
#	stamp remains unmolested.  This heuristic may help to minimize rebuilds
#	when the file is listed as a dependency in a makefile.
#------------------------------------------------------------------------------
AC_DEFUN([CS_TEXT_CACHE_APPEND], [$1="${$1}$2"])
AC_DEFUN([CS_TEXT_CACHE_PREPEND], [$1="$2${$1}"])
AC_DEFUN([CS_TEXT_CACHE_OUTPUT],
    [AC_CONFIG_COMMANDS([$2],
	[echo $ECHO_N "$$1$ECHO_C" > $tmp/tcache
	    if diff $2 $tmp/tcache >/dev/null 2>&1; then
		AC_MSG_NOTICE([$2 is unchanged])
	    else
		rm -f $2
		mv $tmp/tcache $2
	    fi],
	[$1='$$1' cs_dir=`AS_DIRNAME([$2])`
	    AS_MKDIR_P([AS_ESCAPE([$cs_dir])])])])



#------------------------------------------------------------------------------
# Text cache facility for makefile-style properties.  The cache is stored in
# the shell variable cs_makefile_text.
#
# CS_MAKEFILE_APPEND(TEXT)
#	Append text to the makefile text cache.  This is a cover for
#	CS_TEXT_CACHE_APPEND().
#
# CS_MAKEFILE_PREPEND(TEXT)
#	Prepend text to the makefile text cache.  This is a cover for
#	CS_TEXT_CACHE_PREPEND().
#
# CS_MAKEFILE_PROPERTY(KEY, VALUE, [APPEND])
#	Append a line of the form "KEY = VALUE" to the makefile text cache.  If
#	the APPEND argument is not the empty string, then VALUE is appended to
#	the existing value of KEY using the form "KEY += VALUE".  Note that if
#	VALUE references other makefile variables, for example $(OBJS), then be
#	sure to protect the value with AS_ESCAPE().  For example:
#	CS_MAKEFILE_PROPERTY([ALLOBJS], [AS_ESCAPE([$(OBJS) $(LIBOBJS)])])
#
# CS_MAKEFILE_OUTPUT(FILENAME)
#	Instruct config.status to write the makefile text cache to the given
#	filename.  This is a cover for CS_TEXT_CACHE_OUTPUT().
#------------------------------------------------------------------------------
AC_DEFUN([CS_MAKEFILE_APPEND],
    [CS_TEXT_CACHE_APPEND([cs_makefile_text], [$1])])
AC_DEFUN([CS_MAKEFILE_PREPEND],
    [CS_TEXT_CACHE_PREPEND([cs_makefile_text], [$1])])
AC_DEFUN([CS_MAKEFILE_PROPERTY],
[CS_MAKEFILE_APPEND([$1 m4_ifval([$3], [+=], [=]) $2
])])
AC_DEFUN([CS_MAKEFILE_OUTPUT],[CS_TEXT_CACHE_OUTPUT([cs_makefile_text], [$1])])



#------------------------------------------------------------------------------
# Text cache facility for C-style #define properties.  The cache is stored in
# the shell variable cs_header_text.
#
# CS_HEADER_APPEND(TEXT)
#	Append text to the C header text cache.  This is a cover for
#	CS_TEXT_CACHE_APPEND().
#
# CS_HEADER_PREPEND(TEXT)
#	Prepend text to the C header text cache.  This is a cover for
#	CS_TEXT_CACHE_PREPEND().
#
# CS_HEADER_PROPERTY(KEY, [VALUE])
#	Append a line of the form "#define KEY VALUE" to the C header text
#	cache.  If the VALUE argument is omitted, then the appended line has
#	the simplified form "#define KEY".
#
# CS_HEADER_OUTPUT(FILENAME)
#	Instruct config.status to write the C header text cache to the given
#	filename.  This is a cover for CS_TEXT_CACHE_OUTPUT().
#------------------------------------------------------------------------------
AC_DEFUN([CS_HEADER_APPEND], [CS_TEXT_CACHE_APPEND([cs_header_text], [$1])])
AC_DEFUN([CS_HEADER_PREPEND], [CS_TEXT_CACHE_PREPEND([cs_header_text], [$1])])
AC_DEFUN([CS_HEADER_PROPERTY],
[CS_HEADER_APPEND([@%:@define $1[]m4_ifval([$2], [ $2], [])
])])
AC_DEFUN([CS_HEADER_OUTPUT], [CS_TEXT_CACHE_OUTPUT([cs_header_text], [$1])])



#------------------------------------------------------------------------------
# CS_SPLIT(LINE, [OUTPUT-VARIABLES], [DELIMITER], [FILLER])
#	Split LINE into individual tokens.  Tokens are delimited by DELIMITER,
#	which is the space character if omitted.  OUTPUT-VARIABLES is a
#	comma-delimited list of shell variables which should receive the
#	extracted tokens.  If there are two few tokens to fill the output
#	variables, then the excess variables will be assigned the empty string.
#	If there are too few output variables, then the excess tokens will be
#	ignored.  If OUTPUT-VARIABLES is omitted, then the split tokens will be
#	assigned to the shell meta-variables $1, $2, $3, etc.  When
#	OUTPUT-VARIABLES is omitted, FILLER is assigned to meta-variables in
#	cases where DELIMITER delimits a zero-length token.  FILLER defaults
#	to "filler".  For example, if DELIMITER is "+" and OUTPUT-VARIABLES is
#	omitted, given the line "one++three", $1 will be "one", $2 will be
#	"filler", and $3 will be "three".
#------------------------------------------------------------------------------
AC_DEFUN([CS_SPLIT],
    [m4_define([cs_split_filler], m4_default([$4],[filler]))
    set cs_split_filler `echo "$1" | awk 'BEGIN { FS="m4_default([$3],[ ])" }
	{ for (i=1; i <= NF; ++i)
	    { if ($i == "") print "cs_split_filler"; else print $i } }'`
    shift
    m4_map([_CS_SPLIT], [$2])])

AC_DEFUN([_CS_SPLIT],
    [AS_IF([test $[@%:@] -eq 0], [$1=''],
	[AS_IF([test "$[1]" = cs_split_filler], [$1=''], [$1=$[1]])
	shift])])



#------------------------------------------------------------------------------
# CS_SPLIT_TUPLE(TUPLE, OUTPUT-VARIABLES)
#	Split a build-tuple into its component parts.  A build tuple is
#	constructed by CS_CREATE_TUPLE() and is comprised of compiler flags,
#	linker flags, and library references.  OUTPUT-VARIABLES is a
#	comma-delimited list of shell variables which should receive the
#	extracted compiler flags, linker flags, and library references,
#	respectively.
#------------------------------------------------------------------------------
AC_DEFUN([CS_SPLIT_TUPLE],
    [CS_SPLIT([$1], [cs_dummy,$2], [@])
    m4_map([_CS_SPLIT_TUPLE], [$2])])

AC_DEFUN([_CS_SPLIT_TUPLE],
    [$1=`echo $$1 | sed 'y%@%:@% %'`
    ])



#------------------------------------------------------------------------------
# CS_CREATE_TUPLE([CFLAGS], [LFLAGS], [LIBS])
#	Construct a build-tuple which is comprised of compiler flags, linker
#	flags, and library references.  Build tuples are encoded so as to
#	preserve whitespace in each component.  This makes it possible for
#	macros (such as CS_BUILD_IFELSE) which employ build tuples to accept
#	whitespace-delimited lists of tuples, and for shell "for" statements to
#	iterate over tuple lists without compromising whitespace embedded
#	within individual flags or library references.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CREATE_TUPLE], [`echo @$1@$2@$3 | sed 'y% %@%:@%'`])



#------------------------------------------------------------------------------
# CS_LANG_CFLAGS
#	Return the literal string CFLAGS if the current language is C.  Return
#	the literal string CXXFLAGS if the current language is C++.  Generic
#	compiler test macros which need to modify or save the compiler flags
#	can invoke this macro to get the name of the compiler flags environment
#	variable (either CFLAGS or CXXFLAGS) depending upon the current
#	language.  For example:
#		CS_LANG_CFLAGS="$CS_LANG_CFLAGS -Wall"
#	With C, this expands to:
#		CFLAGS="$CFLAGS -Wall"
#	With C++, it expands to:
#		CXXFLAGS="$CXXFLAGS -Wall"
#------------------------------------------------------------------------------
AC_DEFUN([CS_LANG_CFLAGS], [AC_LANG_CASE([C], [CFLAGS], [C++], [CXXFLAGS])])



#------------------------------------------------------------------------------
# CS_BUILD_IFELSE([PROGRAM], [FLAGS], [LANGUAGE], [ACTION-IF-BUILT],
#                 [ACTION-IF-NOT-BUILT], [OTHER-CFLAGS], [OTHER-LFLAGS],
#                 [OTHER-LIBS])
#	Try building a program using the supplied compiler flags, linker flags,
#	and library references.  PROGRAM is typically a program composed via
#	AC_LANG_PROGRAM().  PROGRAM may be omitted if you are interested only
#	in learning if the compiler or linker respects certain flags.  LANGUAGE
#	is typically either C or C++ and specifies which compiler to use for
#	the test.  If LANGUAGE is omitted, C is used.  FLAGS is a whitespace
#	delimited list of build tuples.  Tuples are created with
#	CS_CREATE_TUPLE() and are composed of up to three elements each.  The
#	first element represents compiler flags, the second linker flags, and
#	the third libraries used when linking the program.  Each tuple from
#	FLAGS is attempted in order.  If you want a build attempted with no
#	special flags prior to builds with specialized flags, create an empty
#	tuple with CS_CREATE_TUPLE() at the start of the FLAGS list.  If the
#	build is successful, then the shell variables cs_build_ok is set to
#	"yes", cs_build_cflags, cs_build_lflags, and cs_build_libs are set to
#	the tuple elements which resulted in the successful build, and
#	action-if-built is invoked.  Upon successful build, no further tuples
#	are consulted.  If no tuple results in a successful build, then
#	cs_build_ok is set to "no" and action-if-not-built is invoked.
#	OTHER-CFLAGS, OTHER-LFLAGS, and OTHER-LIBS specify additional compiler
#	flags, linker flags, and libraries which should be used with each tuple
#	build attempt.  Upon successful build, these additional flags are
#	reflected in the variables cs_build_cflags, cs_build_lflags, and
#	cs_build_libs.
#------------------------------------------------------------------------------
AC_DEFUN([CS_BUILD_IFELSE],
    [AC_LANG_PUSH(m4_default([$3],[C]))
    cs_cflags_save="$CS_LANG_CFLAGS"
    cs_lflags_save="$LDFLAGS"
    cs_libs_save="$LIBS"
    cs_build_ok=no

    for cs_build_item in m4_default([$2],[CS_CREATE_TUPLE()])
    do
	CS_SPLIT_TUPLE(
	    [$cs_build_item],[cs_cflags_test,cs_lflags_test,cs_libs_test])
	CS_LANG_CFLAGS="$cs_cflags_test $6 $cs_cflags_save"
	LDFLAGS="$cs_lflags_test $7 $cs_lflags_save"
	LIBS="$cs_libs_test $8 $cs_libs_save"
	AC_LINK_IFELSE(m4_default([$1], [AC_LANG_PROGRAM([],[])]),
	    [cs_build_ok=yes break])
    done

    CS_LANG_CFLAGS=$cs_cflags_save
    LDFLAGS=$cs_lflags_save
    LIBS=$cs_libs_save
    AC_LANG_POP(m4_default([$3],[C]))

    AS_IF([test $cs_build_ok = yes],
	[cs_build_cflags=CS_TRIM([$cs_cflags_test $6])
	cs_build_lflags=CS_TRIM([$cs_lflags_test $7])
	cs_build_libs=CS_TRIM([$cs_libs_test $8])
	m4_default([$4],[:])],
	[m4_default([$5],[:])])])



#------------------------------------------------------------------------------
# CS_CHECK_BUILD(MESSAGE, CACHE-VAR, [PROGRAM], [FLAGS], [LANGUAGE],
#                [ACTION-IF-BUILT], [ACTION-IF-NOT-BUILT], [IGNORE-CACHE],
#                [OTHER-CFLAGS], [OTHER-LFLAGS], [OTHER-LIBS])
#	Like CS_BUILD_IFELSE() but also prints "checking" and result messages,
#	and optionally respects the cache.  Sets CACHE-VAR to "yes" upon
#	success, else "no" upon failure.  Additionally, sets CACHE-VAR_cflags,
#	CACHE-VAR_lflags, and CACHE-VAR_libs to the values which resulted in a
#	successful build.  If IGNORE-CACHE is "yes", then the cache variables
#	are ignored upon entry to this macro, however they are still set to
#	appropriate values upon exit.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_BUILD],
    [AS_IF([test "$8" != yes],
	[AC_CACHE_CHECK([$1], [$2],
	    [CS_BUILD_IFELSE([$3], [$4], [$5],
		[$2=yes
		$2_cflags=$cs_build_cflags
		$2_lflags=$cs_build_lflags
		$2_libs=$cs_build_libs],
		[$2=no], [$9], [$10], [$11])])],
	[AC_MSG_CHECKING([$1])
	    CS_BUILD_IFELSE([$3], [$4], [$5],
		[$2=yes
		$2_cflags=$cs_build_cflags
		$2_lflags=$cs_build_lflags
		$2_libs=$cs_build_libs],
		[$2=no], [$9], [$10], [$11])
	    AC_MSG_RESULT([$$2])])
    AS_IF([test $$2 = yes], [m4_default([$6],[:])],
	[$2_cflags=''
	$2_lflags=''
	$2_libs=''
	m4_default([$7],[:])])])



#------------------------------------------------------------------------------
# CS_CHECK_LIB_WITH(LIBRARY, PROGRAM, [SEARCH-LIST], [LANGUAGE],
#                   [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND], [OTHER-CFLAGS],
#                   [OTHER-LFLAGS], [OTHER-LIBS])
#	Similar to AC_CHECK_LIB(), but allows caller to to provide list of
#	directories in which to search for LIBRARY, and allows user to override
#	library location via --with-libLIBRARY=dir.  LIBRARY is the name of the
#	library which is to be located (for example, "readline" for
#	libreadline.a).  PROGRAM, which is typically composed with
#	AC_LANG_PROGRAM(), is a program which refernces at least one function
#	or symbol in LIBRARY.  SEARCH-LIST is a whitespace-delimited list of
#	paths in which to search for the library and its header files, in
#	addition to those searched by the compiler and linker by default, and
#	those referenced by the cs_lib_paths_default macro.  Each list item can
#	specify an include|library directory tuple (for example,
#	"/usr/include|/usr/lib"), or a single directory (for example, "/usr").
#	If the second form is used, then the directory itself is searched
#	first.  If the library resources are not found, then "include" and
#	"lib" subdirectories of the directory are searched.  Thus, specifying
#	"/proj" is shorthand for "/proj|/proj /proj/include|/proj/lib".  Items
#	in the search list can include wildcards.  SEARCH-LIST can be overriden
#	by the user with the --with-libLIBRARY=dir option, in which case only
#	"dir" and "dir/include|dir/lib" are searched.  If SEARCH-LIST is
#	omitted and the user did not override the search list via
#	--with-libLIBRARY=dir, then only the directories normally searched by
#	the compiler and the directories mentioned via cs_lib_paths_default are
#	searched.  LANGUAGE is typically either C or C++ and specifies which
#	compiler to use for the test.  If LANGUAGE is omitted, C is used.
#	OTHER-CFLAGS, OTHER-LFLAGS, and OTHER-LIBS can specify additional
#	compiler flags, linker flags, and libraries needed to successfully link
#	with LIBRARY.  If the library is found and can be successfully linked
#	into a program, then the shell cache variable cs_cv_libLIBRARY is set
#	to "yes"; cs_cv_libLIBRARY_cflags, cs_cv_libLIBRARY_lflags, and
#	cs_cv_libLIBRARY_libs are set, respectively, to the compiler flags
#	(including OTHER-CFLAGS), linker flags (including OTHER-LFLAGS), and
#	library references (including OTHER-LIBS) which resulted in a
#	successful build; and ACTION-IF-FOUND is invoked.  If the library was
#	not found or was unlinkable, or if the user disabled the library with
#	--without-libLIBRARY, then cs_cv_libLIBRARY is set to "no" and
#	ACTION-IF-NOT-FOUND is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LIB_WITH],
    [AC_ARG_WITH([lib$1], [AC_HELP_STRING([--with-lib$1=dir],
	[specify location of lib$1 if not detected automatically; searches dir,
	dir/include, and dir/lib])])

    AS_IF([test -z "$with_lib$1"], [with_lib$1=yes])
    AS_IF([test "$with_lib$1" != no],
	[# If --with-lib value is same as cached value, then assume other
	 # cached values are also valid; otherwise, ignore all cached values.
	AS_IF([test "$with_lib$1" != "$cs_cv_with_lib$1"],
	    [cs_ignore_cache=yes], [cs_ignore_cache=no])

	AS_IF([test $with_lib$1 != yes],
	    [cs_check_lib_paths=$with_lib$1],
	    [cs_check_lib_paths="| cs_lib_paths_default $3"])

	_CS_CHECK_LIB_CREATE_FLAGS(
	    [cs_check_lib_flags], [$1], [$cs_check_lib_paths])

	CS_CHECK_BUILD([for lib$1], [cs_cv_lib$1], [$2], [$cs_check_lib_flags],
	    [$4], [], [], [$cs_ignore_cache], [$7], [$8], [$9])],
	[cs_cv_lib$1=no])
    cs_cv_with_lib$1="$with_lib$1"

    AS_IF([test "$cs_cv_lib$1" = yes],
	[m4_default([$5],[:])], [m4_default([$6],[:])])])



#------------------------------------------------------------------------------
# _CS_CHECK_LIB_CREATE_FLAGS(VARIABLE, LIBRARY, PATHS)
#	Helper macro for CS_CHECK_LIB_WITH().  Constructs a list of build
#	tuples suitable for CS_CHECK_BUILD() and assigns the tuple list to the
#	shell variable VARIABLE.  LIBRARY and PATHS have the same meanings as
#	the like-named arguments of CS_CHECK_LIB_WITH().
#------------------------------------------------------------------------------
AC_DEFUN([_CS_CHECK_LIB_CREATE_FLAGS],
    [$1=''
    for cs_lib_item in $3
    do
	case $cs_lib_item in
	    *\|*) CS_SPLIT(
		    [$cs_lib_item], [cs_check_incdir,cs_check_libdir], [|])
		_CS_CHECK_LIB_CREATE_FLAG([$1],
		    [$cs_check_incdir], [$cs_check_libdir], [$2])
		;;
	    *)  _CS_CHECK_LIB_CREATE_FLAG(
		    [$1], [$cs_lib_item], [$cs_lib_item], [$2])
		_CS_CHECK_LIB_CREATE_FLAG([$1],
		    [$cs_lib_item/include], [$cs_lib_item/lib], [$2])
		;;
	esac
    done])



#------------------------------------------------------------------------------
# _CS_CHECK_LIB_CREATE_FLAG(VARIABLE, HEADER-DIR, LIBRARY-DIR, LIBRARY)
#	Helper macro for _CS_CHECK_LIB_CREATE_FLAGS().  Constructs a single
#	build tuple suitable for CS_CHECK_BUILD() and appends the tuple to the
#	shell variable VARIABLE.  LIBRARY has the same meanings as the
#	like-named arguments of CS_CHECK_LIB_WITH().
#------------------------------------------------------------------------------
AC_DEFUN([_CS_CHECK_LIB_CREATE_FLAG],
   [AS_IF([test -n "$2"], [cs_check_lib_cflag="-I$2"], [cs_check_lib_cflag=''])
    AS_IF([test -n "$3"], [cs_check_lib_lflag="-L$3"], [cs_check_lib_lflag=''])
    AS_IF([test -n "$4"], [cs_check_lib_libs="-l$4" ], [cs_check_lib_libs='' ])
    $1="$$1 CS_CREATE_TUPLE(
	[$cs_check_lib_cflag], [$cs_check_lib_lflag], [$cs_check_lib_libs])"])



#------------------------------------------------------------------------------
# CS_MAKEFILE_BUILD_RESULT(CACHE-VAR, PREFIX)
#	Record the results of CS_CHECK_BUILD() via makefile variables in the
#	makefile text cache.  If CACHE-VAR indicates that the build succeeded,
#	then the following makefile properties are emitted:
#
#	PREFIX.AVAILABLE = yes
#	PREFIX.CFLAGS = $CACHE-VAR_cflags
#	PREFIX.LFLAGS = $CACHE-VAR_lflags $CACHE-VAR_libs
#------------------------------------------------------------------------------
AC_DEFUN([CS_MAKEFILE_BUILD_RESULT],
    [AS_IF([test "$$1" = yes],
	[CS_MAKEFILE_PROPERTY([$2.AVAILABLE], [yes])
	cs_result_cflags=CS_TRIM([$$1_cflags])
	cs_result_lflags=CS_TRIM([$$1_lflags $$1_libs])
	AS_IF([test -n "$cs_result_cflags"],
	    [CS_MAKEFILE_PROPERTY([$2.CFLAGS], [$cs_result_cflags])])
	AS_IF([test -n "$cs_result_lflags"],
	    [CS_MAKEFILE_PROPERTY([$2.LFLAGS], [$cs_result_lflags])])])])



#------------------------------------------------------------------------------
# CS_MAKEFILE_BUILD_FLAGS(MESSAGE, CACHE-VAR, FLAGS, [LANGUAGE],
#                         MAKEFILE-VARIABLE, [APPEND], [CACHE-VALUE])
#	Check if the compiler or linker recognizes a command-line option.
#	MESSAGE is the "checking" message.  CACHE-VAR is the shell cache
#	variable which receives the flag recognized by the compiler or linker,
#	or "no" if the flag was not recognized.  FLAGS is a
#	whitespace-delimited list of build tuples created with
#	CS_CREATE_TUPLE().  Each tuple from FLAGS is attempted in order until
#	one is found which is recognized by the compiler.  After that, no
#	further other flags are checked.  LANGUAGE is typically either C or C++
#	and specifies which compiler to use for the test.  If LANGUAGE is
#	omitted, C is used.  MAKEFILE-VARIABLE is the name of the makefile
#	variable to insert into the makefile text cache if a usable flag is
#	encountered.  CACHE-VALUE is the value to assign to the makefile
#	variable.  If omitted, it defaults to $cs_build_cflags.  If you are
#	checking linker flags, rather than compiler flags, then you should
#	specify $cs_build_lflags for this argument.  If APPEND is not the empty
#	string, then CACHE-VALUE is appended to the existing value of the
#	makefile variable.
#------------------------------------------------------------------------------
AC_DEFUN([CS_MAKEFILE_BUILD_FLAGS],
    [AC_CACHE_CHECK([$1], [$2],
	[CS_BUILD_IFELSE([], [$3], [$4],
	    [$2=m4_default([$7],[$cs_build_cflags])], [$2=no])])
    AS_IF([test "$$2" != no], [CS_MAKEFILE_PROPERTY([$5], [$$2], [$6])])])



#===============================================================================
# INITIALIZATION
#===============================================================================
AC_INIT([Crystal Space], [cs_version_number],
    [crystal-main@lists.sourceforge.net])
AC_CONFIG_SRCDIR([libs/cssys/cssys.mak])
AC_CONFIG_AUX_DIR([bin])
AC_PREFIX_DEFAULT([cs_install_dir_default])
AC_CANONICAL_HOST



#------------------------------------------------------------------------------
# Check for compilers and common tools.
#------------------------------------------------------------------------------
AC_PROG_CC
CS_MAKEFILE_PROPERTY([CC], [$CC -c])

AC_PROG_CXX
CS_MAKEFILE_PROPERTY([CXX], [$CXX -c])
CS_MAKEFILE_PROPERTY([LINK], [$CXX])
AS_IF([test $ac_compiler_gnu = yes],
    [cs_compiler_name=GCC CS_HEADER_PROPERTY([COMP_GCC])],
    [cs_compiler_name="AS_TR_CPP([$CXX])"])
CS_MAKEFILE_PROPERTY([COMP], [$cs_compiler_name])
CS_HEADER_PROPERTY([CS_COMPILER_NAME],
    [AS_ESCAPE(["])$cs_compiler_name[]AS_ESCAPE(["])])

AC_CHECK_PROGS([FLEXBIN], [flex])
AS_IF([test -n "$FLEXBIN"], [CS_MAKEFILE_PROPERTY([FLEXBIN], [$FLEXBIN])])

AC_CHECK_PROGS([BISONBIN], [bison])
AS_IF([test -n "$BISONBIN"], [CS_MAKEFILE_PROPERTY([BISONBIN], [$BISONBIN])])

AC_PROG_RANLIB
AS_IF([test -n "$RANLIB"], [CS_MAKEFILE_PROPERTY([CMD.RANLIB], [$RANLIB])])

AC_CHECK_PROGS([SWIGBIN], [swig])
AS_IF([test -n "$SWIGBIN"], [CS_MAKEFILE_PROPERTY([SWIGBIN], [$SWIGBIN])])

AC_CHECK_PROGS([LUASWIGBIN], [luaswig])
AS_IF([test -n "$LUASWIGBIN"],
    [CS_MAKEFILE_PROPERTY([LUASWIGBIN], [$LUASWIGBIN])])

AC_CHECK_PROGS([MAKE], [gnumake gmake make])
AC_PROG_MAKE_SET



#------------------------------------------------------------------------------
# Determine how to make a directory tree.  We need to know the exact commands,
# so that we can emit them to the makefile, thus the AS_MKDIR_P function is not
# what we want to use here.  First try "mkdir -p", then try the older "mkdirs".
# If the mkdir command failed to recognize -p, then it might have created a
# directory named "-p", so clean up that bogus directory.
#------------------------------------------------------------------------------
AC_CACHE_CHECK([how to create a directory], [cs_cv_shell_mkdir],
    [cs_cv_shell_mkdir='mkdir'])
CS_MAKEFILE_PROPERTY([CMD.MKDIR], [$cs_cv_shell_mkdir])

AC_CACHE_CHECK([how to create a directory tree], [cs_cv_shell_mkdir_p],
    [if $cs_cv_shell_mkdir -p . 2>/dev/null; then
	cs_cv_shell_mkdir_p='mkdir -p'
    elif mkdirs . 2>/dev/null; then
	cs_cv_shell_mkdir_p='mkdirs'
    fi
    test -r ./-p && rm -rf ./-p])

AS_VAR_SET_IF([cs_cv_shell_mkdir_p],
    [CS_MAKEFILE_PROPERTY([CMD.MKDIRS], [$cs_cv_shell_mkdir_p])],
    [AC_MSG_ERROR([do not know how to create a directory tree])])



#------------------------------------------------------------------------------
# Check for NASM and extensions (such as %xdefine) which we require.
#------------------------------------------------------------------------------
AC_CHECK_PROGS([NASM], [nasm])
AS_IF([test -n "$NASM"],
    [AC_CACHE_CHECK([for nasm extensions], [cs_cv_prog_nasm_ok],
	[echo "%xdefine TEST" >conftest.asm
	AS_IF([AC_RUN_LOG(
	    [$NASM -f elf conftest.asm -o conftest.$OBJEXT 1>&2])],
	    [cs_cv_prog_nasm_ok=yes], [cs_cv_prog_nasm_ok=no])
	rm -f conftest.*])
    AS_IF([test $cs_cv_prog_nasm_ok = yes],
	[CS_MAKEFILE_PROPERTY([NASM.AVAILABLE], [yes])
	CS_MAKEFILE_PROPERTY([NASM.BIN], [$NASM])
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
# Check if Perl SDK is installed and which compiler and linker flags are
# required to embed Perl.  Also determine if the glue file can be created with
# ExtUtils::Embed or if it must be created manually.
#------------------------------------------------------------------------------
AC_CHECK_PROGS([PERL], [perl5 perl])
AS_IF([test -n "$PERL"],
    [CS_MAKEFILE_PROPERTY([PERL5.AVAILABLE], [yes])
    CS_MAKEFILE_PROPERTY([PERL5], [$PERL])
    CS_MAKEFILE_PROPERTY([PERL], [$PERL])

    AC_CACHE_CHECK([for perl ExtUtils::Embed module], [cs_cv_perl_mod_embed],
	[AS_IF([AC_RUN_LOG([$PERL -MExtUtils::Embed -e 0 1>&2])],
	    [cs_cv_perl_mod_embed=yes], [cs_cv_perl_mod_embed=no])])
    AS_IF([test $cs_cv_perl_mod_embed = yes],
	[CS_MAKEFILE_PROPERTY([PERL5.EXTUTILS.AVAILABLE], [yes])

	AC_CACHE_CHECK([for perl DynaLoader module],
	    [cs_cv_perl_mod_dynaloader],
	    [AS_IF([AC_RUN_LOG([$PERL -MDynaLoader -e 0 1>&2])],
		[cs_cv_perl_mod_dynaloader=yes],
		[cs_cv_perl_mod_dynaloader=no])])
	AS_IF([test $cs_cv_perl_mod_dynaloader = yes],
	    [CS_MAKEFILE_PROPERTY([PERL5.EXTUTILS.DYNALOADER], [DynaLoader])])

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
	[CS_MAKEFILE_PROPERTY([PERL5.SDK.AVAILABLE], [yes])
	AS_IF([test -n "$cs_cv_perl_cflags"],
	    [CS_MAKEFILE_PROPERTY([PERL5.CFLAGS], [$cs_cv_perl_cflags])])
	AS_IF([test -n "$cs_cv_perl_lflags"],
	    [CS_MAKEFILE_PROPERTY([PERL5.LFLAGS], [$cs_cv_perl_lflags])])])])



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
AC_CACHE_CHECK([for Python SDK], [cs_cv_python_sdk],
    [cs_cv_python_sdk=no
    cs_python_lib_dir=''
    cs_python_makefile=''

    AS_IF([test -n "$PYTHON"],
	[# Python 2.x?
	AS_IF([AC_RUN_LOG([$PYTHON -c 'import distutils.sysconfig' 1>&2])],
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
		print distutils.sysconfig.get_config_var("LIBS") + " " + \
		distutils.sysconfig.get_config_var("SYSLIBS")'])`
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
CS_CHECK_BUILD([if Python SDK is usable], [cs_cv_python],
    [AC_LANG_PROGRAM([#include <Python.h>],[Py_Initialize(); Py_Finalize();])],

    [CS_CREATE_TUPLE([],[],[$cs_pylib1]) CS_CREATE_TUPLE([],[],[$cs_pylib2])],
    [], [CS_MAKEFILE_BUILD_RESULT([cs_cv_python], [PYTHON])], [], [],
    [$cs_cv_python_base_cflags], [$cs_cv_python_base_lflags],
    [$cs_cv_python_base_libs])



#------------------------------------------------------------------------------
# Check how to enable compilation warnings.  Note that on some platforms, it is
# more appropriate to use -Wmost rather than -Wall even if the compiler
# understands both, thus we attempt -Wmost before -Wall.
#------------------------------------------------------------------------------
CS_MAKEFILE_BUILD_FLAGS([how to enable compilation warnings],
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
CS_MAKEFILE_BUILD_FLAGS([how to disable unknown @%:@pragma warnings],
    [cs_cv_prog_cxx_disable_pragma_warnings],
    [CS_CREATE_TUPLE([-Wno-unknown-pragmas])], [C++], [CFLAGS.SYSTEM], [+])



#------------------------------------------------------------------------------
# Check if C++ exceptions can be disabled.
#------------------------------------------------------------------------------
CS_MAKEFILE_BUILD_FLAGS([how to disable C++ exceptions],
    [cs_cv_prog_cxx_disable_exceptions], [CS_CREATE_TUPLE([-fno-exceptions])],
    [C++], [CFLAGS.SYSTEM], [+])



#------------------------------------------------------------------------------
# Check if the linker recognizes "-multiply_defined suppress".  This is
# required for MacOS/X 10.1 to avoid warning messages when linking a program
# with "-framework Foundation" if that program does not actually employ any
# Objective-C.
#------------------------------------------------------------------------------
CS_MAKEFILE_BUILD_FLAGS([if -multiply_defined suppress is needed],
    [cs_cv_prog_link_suppress_multiple],
    [CS_CREATE_TUPLE([], [-multiply_defined suppress])], [],
    [MACOSX.LFLAGS.CONFIG], [+], [$cs_build_lflags])



#------------------------------------------------------------------------------
# Check if Mingw compiler recognizes -fvtable-thunks.  As of gcc 3.x, this
# option is no longer supported.  Unfortunately, rather than returning an error
# code (as it does with other unrecognized options), the compiler merely prints
# a warning message when -fvtable-thunks is used, thus we use -Werror to turn
# the warning into an error.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_MINGW_VTABLE_THUNKS],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
	mingw*)
	    CS_MAKEFILE_BUILD_FLAGS([if -fvtable-thunks is needed],
		[cs_cv_prog_cxx_vtable_thunks],
		[CS_CREATE_TUPLE([-fvtable-thunks -Werror])],
		[C++], [CFLAGS.SYSTEM], [+], [-fvtable-thunks])
	    ;;
    esac])

CS_CHECK_MINGW_VTABLE_THUNKS



#------------------------------------------------------------------------------
# Check for compiler- and CPU-specific code generation flags.  These flags are
# mutually exclusive.  Exclusivity is enforced by the CS_CHECK_CODE_GEN_FLAG()
# macro.  Note that use of these flags typically means that the resulting
# binary will not be backward compatible.  For instance, if Pentium-specific
# instructions are used, the program will not work with earlier processors,
# such as the Intel 486.  Because these options are potentially dangerous,
# their use it controlled by the user via --enable-cpu-specific-instructions.
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

AC_MSG_CHECKING([whether to use CPU-specific instructions])
AC_ARG_ENABLE([cpu-specific-instructions],
    [AC_HELP_STRING([--enable-cpu-specific-instructions],
	[enable CPU-specific code generation (default YES); use with caution
	since code for one CPU will not work with earlier CPUs (for example,
	Pentium-specific code will not run on an Intel 486)])],
    [], [enable_cpu_specific_instructions=yes])
AC_MSG_RESULT([$enable_cpu_specific_instructions])

AS_IF([test $enable_cpu_specific_instructions = yes],
    [AC_CACHE_CHECK([for code generation flags], [cs_cv_prog_cxx_code_gen],
	[cs_cv_prog_cxx_code_gen=no
	CS_CHECK_CODE_GEN_FLAG([athlon], [-mcpu=athlon -march=athlon])
	CS_CHECK_CODE_GEN_FLAG([*686], [-mcpu=pentiumpro -march=i686])
	CS_CHECK_CODE_GEN_FLAG([*686], [-mpentiumpro -march=i686])
	CS_CHECK_CODE_GEN_FLAG([[*[5-6]86]], [-mcpu=pentium -march=i586])
	CS_CHECK_CODE_GEN_FLAG([[*[5-6]86]], [-mpentium -march=i586])
	CS_CHECK_CODE_GEN_FLAG([[*[3-9]86]], [-mcpu=i486])
	CS_CHECK_CODE_GEN_FLAG([[*[3-9]86]], [-m486])
	CS_CHECK_CODE_GEN_FLAG([alpha], [-mieee])])

    AS_IF([test "$cs_cv_prog_cxx_code_gen" != no],
	[CS_MAKEFILE_PROPERTY(
	    [CFLAGS.SYSTEM], [$cs_cv_prog_cxx_code_gen], [+])])])



#------------------------------------------------------------------------------
# Check for compiler optimization flags.
#------------------------------------------------------------------------------
CS_MAKEFILE_BUILD_FLAGS([how to enable optimizations],
    [cs_cv_prog_cxx_optimize],
    [CS_CREATE_TUPLE([-O3]) CS_CREATE_TUPLE([-O2]) CS_CREATE_TUPLE([-O])],
    [C++], [CFLAGS.optimize], [+])

CS_MAKEFILE_BUILD_FLAGS([if $CXX recognizes -fomit-frame-pointer],
    [cs_cv_prog_cxx_omit_frame_pointer],
    [CS_CREATE_TUPLE([-fomit-frame-pointer])], [C++], [CFLAGS.optimize], [+])

CS_MAKEFILE_BUILD_FLAGS([if $CXX recognizes -ffast-math],
    [cs_cv_prog_cxx_fast_math],
    [CS_CREATE_TUPLE([-ffast-math])], [C++], [CFLAGS.optimize], [+])



#------------------------------------------------------------------------------
# Check for compiler debug flags.
#------------------------------------------------------------------------------
CS_MAKEFILE_BUILD_FLAGS([how to enable compiler debug output],
    [cs_cv_prog_cxx_debug],
    [CS_CREATE_TUPLE([-g3]) CS_CREATE_TUPLE([-g2]) CS_CREATE_TUPLE([-g])],
    [C++], [CFLAGS.debug], [+])
AS_IF([test $cs_cv_prog_cxx_debug != no],
    [CS_MAKEFILE_PROPERTY([LFLAGS.debug], [$cs_cv_prog_cxx_debug], [+])])



#------------------------------------------------------------------------------
# Check if compiler complains about adding /usr/local/include to the header
# search path.  If not, add it to the header search path since not all
# compilers search there by default.  Some compiler, on the other hand,
# complain if it is already in the built-in search path, and we would like to
# avoid the complaint, if possible.
#------------------------------------------------------------------------------
AS_IF([test -d /usr/local/include],
    [AS_IF([test $cs_cv_prog_cxx_enable_warnings != no && \
	test $cs_cv_prog_cxx_enable_errors != no],
	[AC_CACHE_CHECK([checking if -I/usr/local/include is usable],
	    [cs_cv_prog_cxx_local_include],
	    [CS_BUILD_IFELSE([],
		[CS_CREATE_TUPLE([-I/usr/local/include \
		    $cs_cv_prog_cxx_enable_warnings \
		    $cs_cv_prog_cxx_enable_errors],[],[])], [C++],
		    [cs_cv_prog_cxx_local_include=yes],
		    [cs_cv_prog_cxx_local_include=no])])],
	[cs_cv_prog_cxx_local_include=yes])

    AS_IF([test $cs_cv_prog_cxx_local_include = yes],
	[CS_MAKEFILE_PROPERTY([CFLAGS.SYSTEM], [-I/usr/local/include], [+])])])
		


#------------------------------------------------------------------------------
# Check for X-Windows.  If the X-Windows implementation is XFree86, then also
# enable the `VideoMode' extension.
#------------------------------------------------------------------------------
AC_PATH_X
AC_PATH_XTRA

AS_IF([test "$no_x" != yes],
    [CS_MAKEFILE_PROPERTY([X11.AVAILABLE], [yes])
    CS_MAKEFILE_PROPERTY([X_CFLAGS], [$X_CFLAGS])
    CS_MAKEFILE_PROPERTY([X_LIBS], [$X_LIBS])
    CS_MAKEFILE_PROPERTY([X_PRE_LIBS], [$X_PRE_LIBS])
    CS_MAKEFILE_PROPERTY([X_EXTRA_LIBS], [$X_EXTRA_LIBS])

    CS_CHECK_LIB_WITH([Xxf86vm],
	[AC_LANG_PROGRAM(
	    [#define XK_MISCELLANY 1
	    #include <X11/Xlib.h>
	    #include <X11/Xutil.h>
	    #include <X11/Xatom.h>
	    #include <X11/extensions/xf86vmode.h>],
	    [XF86VidModeLockModeSwitch(0, 0, 0)])], [], [],
	[CS_MAKEFILE_PROPERTY([USE_XFREE86VM], [yes])], [],
	[$X_CFLAGS], [], [$X_PRE_LIBS $X_LIBS -lX11 -lXext $X_EXTRA_LIBS])])



#------------------------------------------------------------------------------
# Check for OpenGL and companions.  Prefer Mesa over a vendor's OpenGL
# implementation, unless expclitly disabled via --without-mesa.  Also check if
# GLX extensions are supported and define the header property
# CSGL_EXT_STATIC_ASSERTION if they are _not_ supported.
#------------------------------------------------------------------------------
m4_define([cs_define_int32],
    [@%:@if !HAVE_TYPE_INT32
    typedef long int32;
    @%:@endif
    ])

AC_DEFUN([CS_CHECK_GL_PREPARE],
    [AC_REQUIRE([AC_PATH_X])
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

# Check for OpenGL.
CS_CHECK_BUILD([for OpenGL], [cs_cv_libgl],
    [AC_LANG_PROGRAM([CS_GL_INCLUDE([CS_OPENGL_PATH],[GL],[gl.h])],[glEnd()])],
    [$cs_mesa_gl \
    CS_CREATE_TUPLE([],[],[-lGL]) \
    CS_CREATE_TUPLE([],[],[-lopengl32]) \
    CS_CREATE_TUPLE([-DCS_OPENGL_PATH=OpenGL],[],[-framework OpenGL])], [],
    [CS_MAKEFILE_BUILD_RESULT([cs_cv_libgl], [GL])], [], [],
    [$cs_gl_plat_cflags], [$cs_gl_plat_lflags], [$cs_gl_plat_libs])

AS_IF([test $cs_cv_libgl = yes],
    [cs_gl_plat_cflags="$cs_gl_plat_cflags $cs_cv_libgl_cflags"
    cs_gl_plat_lflags="$cs_gl_plat_lflags $cs_cv_libgl_lflags"
    cs_gl_plat_libs="$cs_gl_plat_libs $cs_cv_libgl_libs"

    # Check for GLU.
    CS_CHECK_BUILD([for GLU], [cs_cv_libglu],
	[AC_LANG_PROGRAM(
	    [CS_GL_INCLUDE([CS_GL_PATH],[GL],[glu.h])], [gluNewQuadric()])],
	[$cs_mesa_glu \
	CS_CREATE_TUPLE([],[],[-lGLU]) \
	CS_CREATE_TUPLE([],[],[-lglu32]) \
	CS_CREATE_TUPLE([-DCS_GL_PATH=OpenGL],[],[-framework OpenGL])], [],
	[CS_MAKEFILE_BUILD_RESULT([cs_cv_libglu], [GLU])], [], [],
	[$cs_gl_plat_cflags], [$cs_gl_plat_lflags], [$cs_gl_plat_libs])

    # Check for glut.
    CS_CHECK_BUILD([for glut], [cs_cv_libglut],
	[AC_LANG_PROGRAM(
	    [CS_GL_INCLUDE([CS_GLUT_PATH],[GL],[glut.h])], [glutSwapBuffers])],
	[CS_CREATE_TUPLE([],[],[-lglut]) \
	CS_CREATE_TUPLE([],[],[-lglut32]) \
	CS_CREATE_TUPLE([-DCS_GLUT_PATH=GLUT],[],[-framework GLUT])], [],
	[CS_MAKEFILE_BUILD_RESULT([cs_cv_libglut], [GLUT])], [], [],
	[$cs_gl_plat_cflags], [$cs_gl_plat_lflags], [$cs_gl_plat_libs])

    # Check for GLX.
    CS_CHECK_BUILD([for GLX], [cs_cv_libglx],
	[AC_LANG_PROGRAM([#include <GL/glx.h>], [glXWaitGL()])],
	[$cs_mesa_glx \
	CS_CREATE_TUPLE([],[],[-lGLX])], [],
	[CS_MAKEFILE_BUILD_RESULT([cs_cv_libglx], [GLX])], [], [],
	[$cs_gl_plat_cflags], [$cs_gl_plat_lflags], [$cs_gl_plat_libs])

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
# Check for zlib.  In addition to the standard installed locations, we also
# search CS/libs/zlib*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([z], [AC_LANG_PROGRAM([#include <zlib.h>], [zlibVersion()])],
    [libs/zlib*], [], [CS_MAKEFILE_BUILD_RESULT([cs_cv_libz], [ZLIB])])



#------------------------------------------------------------------------------
# Check for libpng.  In addition to the standard installed locations, we also
# search CS/libs/libpng*.
#------------------------------------------------------------------------------
AS_IF([test "$cs_cv_libz" = yes],
    [CS_CHECK_LIB_WITH([png],
	[AC_LANG_PROGRAM([#include <png.h>],
	    [png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)])],
	[libs/libpng*], [], [CS_MAKEFILE_BUILD_RESULT([cs_cv_libpng], [PNG])],
	[], [$cs_cv_libz_cflags], [$cs_cv_libz_lflags], [$cs_cv_libz_libs])])



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
    [libs/*jpeg*], [], [CS_MAKEFILE_BUILD_RESULT([cs_cv_libjpeg], [JPEG])])



#------------------------------------------------------------------------------
# Check for libmng.  In addition to the standard installed locations, we also
# search CS/libs/libmng*.
#------------------------------------------------------------------------------
AS_IF([test "$cs_cv_libz" = yes && test "$cs_cv_libjpeg" = yes],
    [CS_CHECK_LIB_WITH([mng],
	[AC_LANG_PROGRAM([#include <libmng.h>], [mng_version_release()])],
	[libs/libmng*], [],
	[CS_MAKEFILE_BUILD_RESULT([cs_cv_libmng], [MNG])], [],
	[$cs_cv_libjpeg_cflags $cs_cv_libz_cflags],
	[$cs_cv_libjpeg_lflags $cs_cv_libz_lflags],
	[$cs_cv_libjpeg_libs $cs_cv_libz_libs])])

#-----------------------------------------------------------------------------
# Check for libmikmod.  In addition to the standard installed locations, we
# also search CS/libs/libmikmod*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([mikmod],
    [AC_LANG_PROGRAM(
	[#include <mikmod.h>],
	[MikMod_Init("mikmodtest"); MikMod_Exit();])],
    [libs/libmikmod*], [],
    [CS_MAKEFILE_BUILD_RESULT([cs_cv_libmikmod], [MIKMOD])])

#-----------------------------------------------------------------------------
# Check for libogg.  In addition to the standard installed locations, we
# also search CS/libs/libogg*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([ogg],
    [AC_LANG_PROGRAM(
	[#include <ogg/ogg.h>],
	[/*some test would be nice here...*/])],
    [libs/libogg*], [],                                                         
    [CS_MAKEFILE_BUILD_RESULT([cs_cv_libogg], [OGG])])

#-----------------------------------------------------------------------------
# Check for lib3ds.  In addition to the standard installed locations, we
# also search CS/libs/lib3ds*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([3ds],
    [AC_LANG_PROGRAM(
	[#include <lib3ds/file.h>],
	[/*some test would be nice here...*/])],
    [libs/lib3ds*], [],                                                         
    [CS_MAKEFILE_BUILD_RESULT([cs_cv_lib3ds], [3DS])])

#-----------------------------------------------------------------------------
# Check for libode.  In addition to the standard installed locations, we
# also search CS/libs/libode*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([ode],
    [AC_LANG_PROGRAM(
	[#include <ode/ode.h>],
	[/*some test would be nice here...*/])],
    [libs/libode*], [],                                                         
    [CS_MAKEFILE_BUILD_RESULT([cs_cv_libode], [ODE])])

#-----------------------------------------------------------------------------
# Check for libopenal.  In addition to the standard installed locations, we
# also search CS/libs/libopenal*.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([openal],
    [AC_LANG_PROGRAM(
	[#include <AL/al.h>],
	[/*some test would be nice here...*/])],
    [libs/libopenal*], [],
    [CS_MAKEFILE_BUILD_RESULT([cs_cv_libopenal], [OPENAL])])

#-----------------------------------------------------------------------------
# Check for freetype2.
#-----------------------------------------------------------------------------
AC_CHECK_FT2([2.0],
    [CS_MAKEFILE_PROPERTY([FT2.AVAILABLE], [yes])
     CS_MAKEFILE_PROPERTY([FT2.LFLAGS], [$FT2_LIBS])
     CS_MAKEFILE_PROPERTY([FT2.CFLAGS], [$FT2_CFLAGS])])

#----------------------------------------------------------------------------
# Check if we should build linux joystick drivers
#----------------------------------------------------------------------------
AC_ARG_ENABLE([linux-joystick-support],
    [AC_HELP_STRING([--enable-linux-joystick-support],
	[enable joystick support on linux platform (default NO); This builds
	plugins for the linux joystick which tries to use the linux kernel
	joystick drivers.])], [], [enable_linux_joystick_support=yes])
AS_IF([test $enable_linux_joystick_support = yes],
    [CS_MAKEFILE_PROPERTY([LINUXJOYSTICK.AVAILABLE], [yes])], [])

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
    [CS_MAKEFILE_PROPERTY([LIBS.SYSTEM], [-lmingwex], [+])])



#------------------------------------------------------------------------------
# Check for svgalib.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([vga],
    [AC_LANG_PROGRAM([#include <vga.h>], [vga_init()])], [], [],
    [CS_MAKEFILE_BUILD_RESULT([cs_cv_libvga], [SVGA])
	CS_HEADER_PROPERTY([CS_HAS_SVGALIB])],
    [], [], [], [-lvgagl])



#------------------------------------------------------------------------------
# Check for cal3d.
#------------------------------------------------------------------------------
CS_CHECK_LIB_WITH([cal3d],
    [AC_LANG_PROGRAM([#include <cal3d/cal3d.h>],
	[CalModel m; m.create(0); m.destroy();])],
    [], [C++],
    [CS_MAKEFILE_BUILD_RESULT([cs_cv_libcal3d], [CAL3D])
	CS_MAKEFILE_PROPERTY([HAS_CAL3D], [yes])])



#------------------------------------------------------------------------------
# Check for pthread.
#------------------------------------------------------------------------------
m4_define([cs_pthread_flags],
    [CS_CREATE_TUPLE() \
    CS_CREATE_TUPLE([], [], [-lpthread]) \
    CS_CREATE_TUPLE([-pthread], [], []) \
    CS_CREATE_TUPLE([-pthread], [], [-lpthread])])

CS_CHECK_BUILD([for pthread], [cs_cv_sys_pthread],
    [AC_LANG_PROGRAM(
	[#include <pthread.h>
	void* worker(void* p) { (void)p; return p; }],
	[pthread_t tid; pthread_create(&tid, 0, worker, 0);])],
    [cs_pthread_flags], [],
    [CS_MAKEFILE_BUILD_RESULT([cs_cv_sys_pthread], [PTHREAD])])



#------------------------------------------------------------------------------
# Check for typical required libraries.
#------------------------------------------------------------------------------
AC_LANG_PUSH([C])
AC_CHECK_LIB([m], [pow], [CS_MAKEFILE_PROPERTY([LIBS.EXE], [-lm], [+])])
AC_CHECK_LIB([dl], [dlopen], [CS_MAKEFILE_PROPERTY([LIBS.EXE], [-ldl], [+])])
AC_CHECK_LIB([nsl], [gethostbyname],
    [CS_MAKEFILE_PROPERTY([LIBS.EXE], [-lnsl], [+])])
AC_LANG_POP([C])



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
    [AC_REQUIRE([AC_PATH_X])
    AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([AC_PROG_CXX])
    # If user explicitly requested --with-x, then assume Darwin+XFree86; else
    # assume MacOS/X.
    AC_MSG_CHECKING([for --with-x])
    if test "$with_x" = "yes"; then
	AC_MSG_RESULT([yes (assume Darwin)])
	cs_host_target=unix
	cs_host_makefile=libs/cssys/unix/unix.mak
	cs_host_family=unix
    else
	AC_MSG_RESULT([no (assume MacOS/X)])
	cs_host_target=macosx
	cs_host_makefile=libs/cssys/macosx/macosx.mak
	cs_host_family=unix
	cs_host_os_normalized='MacOS/X'
	CS_HEADER_PROPERTY([OS_MACOSX])
	CS_HEADER_PROPERTY([OS_MACOSX_PLUGIN_DIR],[AS_ESCAPE(["components/"])])
	CS_MAKEFILE_PROPERTY([MACOSX.PLUGIN_DIR], [components])
	CS_HEADER_PROPERTY([OS_MACOSX_PLUGIN_EXT],[AS_ESCAPE([".csplugin"])])
	CS_MAKEFILE_PROPERTY([MACOSX.PLUGIN_EXT], [.csplugin])
	AC_CACHE_CHECK([for Objective-C compiler], [cs_cv_prog_objc],
	    [cs_cv_prog_objc="$CC -c"])
	CS_MAKEFILE_PROPERTY([OBJC], [$cs_cv_prog_objc])
	AC_CACHE_CHECK([for Objective-C++ compiler], [cs_cv_prog_objcxx],
	    [cs_cv_prog_objcxx="$CXX -c"])
	CS_MAKEFILE_PROPERTY([OBJCXX], [$cs_cv_prog_objcxx])
    fi])

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
CS_HEADER_PROPERTY([CS_PLATFORM_NAME],
    [AS_ESCAPE(["])$cs_host_os_normalized[]AS_ESCAPE(["])])



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
CS_MAKEFILE_PROPERTY([PROC], [$cs_host_cpu_normalized])
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
# Check for audio support.
#------------------------------------------------------------------------------
AC_CHECK_HEADER([soundcard.h],
    [CS_HEADER_PROPERTY([CS_HAS_SOUNDCARD_H])],
    [AC_CHECK_HEADER([sys/soundcard.h],
	[CS_HEADER_PROPERTY([CS_HAS_SYS_SOUNDCARD_H])],
	    [AC_CHECK_HEADER([machine/soundcard.h],
		[CS_HEADER_PROPERTY([CS_HAS_MACHINE_SOUNDCARD_H])])])])



#------------------------------------------------------------------------------
# Check for BSD-style socket support.  Also test for presence of type
# socklen_t.
#------------------------------------------------------------------------------
m4_define([cs_socket_flags],
    [CS_CREATE_TUPLE() \
    CS_CREATE_TUPLE([], [], [-lsocket]) \
    CS_CREATE_TUPLE([], [], [-lsocket -lnsl]) \
    CS_CREATE_TUPLE([], [], [-lwsock32])])

AC_CHECK_HEADERS(
    [unistd.h \
    sys/types.h \
    sys/socket.h \
    sys/ioctl.h \
    netinet/in.h \
    arpa/inet.h \
    netdb.h \
    winsock.h])

CS_CHECK_BUILD([for socket support], [cs_cv_sys_socket],
    [AC_LANG_PROGRAM(
	[#if HAVE_SYS_TYPES_H
	#include <sys/types.h>
	#endif
	#if HAVE_SYS_SOCKET_H
	#include <sys/socket.h>
	#endif
	#if HAVE_NETINET_IN_H
	#include <netinet/in.h>
	#endif
	#if HAVE_ARPA_INET_H
	#include <arpa/inet.h>
	#endif
	#if HAVE_WINSOCK_H
	#include <winsock.h>
	@%:@endif],
	[struct in_addr a; inet_ntoa(a); socket(0,0,0);])],
	[cs_socket_flags], [],
	[CS_MAKEFILE_BUILD_RESULT([cs_cv_sys_socket], [SOCKET])])

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
    #endif
    ])])



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
#	this file (such as the RedHat 7 gcc).  Sets the shell variable
#	cs_cv_cxx_qsqrt_ok to "yes" if qsqrt.h is compilable, otherwise "no".
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_QSQRT],
    [AC_REQUIRE([CS_CHECK_HOST_CPU])
    AC_REQUIRE([AC_PROG_CXX])
    AS_IF([test $ac_compiler_gnu = yes],
	[AC_CACHE_CHECK([if qsqrt() compiles], [cs_cv_cxx_qsqrt_ok],
	    [CS_BUILD_IFELSE(
		[AC_LANG_PROGRAM(
		    [#define PROC_$cs_host_cpu_normalized
		    #define COMP_GCC
		    #include "include/qsqrt.h"],
		    [float n = qsqrt(1.0f); (void)n;])],
		[], [C++],
		[cs_cv_cxx_qsqrt_ok=yes], [cs_cv_cxx_qsqrt_ok=no])])],
	[cs_cv_cxx_qsqrt_ok=no])])

CS_CHECK_QSQRT
AS_IF([test $cs_cv_cxx_qsqrt_ok = no], [CS_HEADER_PROPERTY([CS_NO_QSQRT])])



#------------------------------------------------------------------------------
# Check if a compiler bug discovered in gcc 2.96 (RedHat and other unstable
# versions) and gcc 3.0.1 causes qint() to misbehave.
# -----------------------------------------------------------------------------
AC_CACHE_CHECK([if qint() functions correctly], [cs_cv_cxx_qint_ok],
    [AC_LANG_PUSH([C++])
    cs_cflags_save="$CXXFLAGS"
    AS_IF([test $ac_compiler_gnu = yes], [CXXFLAGS="$CXXFLAGS -O2"])
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
    AC_LANG_POP([C++])])

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
CS_MAKEFILE_PROPERTY([MODE], [$cs_build_mode])



#------------------------------------------------------------------------------
# Determine if modules should be built as plugins or static link libraries.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([if building plugins])
AC_ARG_ENABLE([plugins], [AC_HELP_STRING([--enable-plugins],
    [create plugin modules instead of static link libraries (default YES)])],
    [], [enable_plugins=yes])
AC_MSG_RESULT([$enable_plugins])
CS_MAKEFILE_PROPERTY([USE_PLUGINS], [$enable_plugins])
AS_IF([test $enable_plugins != yes], [CS_HEADER_PROPERTY([CS_STATIC_LINKED])])



#------------------------------------------------------------------------------
# Determine if smart pointer strictness should be enforced.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([if enforcing smart pointer strictness])
AC_ARG_ENABLE([strict-smart-pointers],
    [AC_HELP_STRING([--enable-strict-smart-pointers],
	[enable strict smart pointers (default YES); when enabled, csPtr<T>
	returned from functions can only be assigned to csRef<T>; when
	disabled, csPtr<T> can also be assigned to T*, but client is then
	responsible for invoking DecRef()])], [], [enable_smart_pointers=yes])
AC_MSG_RESULT([$enable_smart_pointers])
AS_IF([test $enable_smart_pointers = yes],
    [CS_HEADER_PROPERTY([CS_STRICT_SMART_POINTERS])])



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
AS_IF([test $enable_extensive_memory_debugger = yes],
    [CS_HEADER_PROPERTY([CS_EXTENSIVE_MEMDEBUG])])



#------------------------------------------------------------------------------
# Determine installation directory.
#------------------------------------------------------------------------------
AC_MSG_CHECKING([for installation directory])
AS_IF([test $prefix = NONE],
    [cs_install_dir=cs_install_dir_default], [cs_install_dir=$prefix])
AC_MSG_RESULT([$cs_install_dir])
CS_MAKEFILE_PROPERTY([INSTALL_DIR], [$cs_install_dir])



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
    AC_MSG_RESULT([$enable_makefile_cache_monitor])
    CS_MAKEFILE_PROPERTY([MONITOR_MAKEFILE_CACHE],
	[$enable_makefile_cache_monitor])])



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
# Output generated files.  Also create makefile cache if caching enabled.
#------------------------------------------------------------------------------
CS_HEADER_OUTPUT([include/volatile.h])
CS_MAKEFILE_OUTPUT([config.mak])
AC_CONFIG_FILES([Makefile])
AS_IF([test $enable_makefile_cache = yes],
    [AC_CONFIG_COMMANDS([cache.mak],
	[rm -f cache.mak ; ${MAKE} recache], [MAKE=${MAKE-make}])])

AC_OUTPUT
