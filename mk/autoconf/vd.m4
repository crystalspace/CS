dnl ****************************************************************
dnl **                      helper definitions                    **
dnl ****************************************************************
m4_define([CS_VCHK_RUNTH], [m4_pushdef([i], [$1])m4_if($1,0,,[CS_VCHK_RUNTH(m4_decr($1), [$2])][$2])m4_popdef([i])])
m4_define([CS_VCHK_PREFIX],[])
m4_define([CS_VCHK_SUFFIX],[])
m4_define([CS_VCHK_GROUPPREFIX], [\(])
m4_define([CS_VCHK_GROUPSUFFIX], [\)])
m4_define([CS_VCHK_CHAR], [[[[a-zA-Z]]]])
m4_define([CS_VCHK_DIGIT], [[[0-9]]])
m4_define([CS_VCHK_SEQUENCE], [CS_VCHK_PREFIX[]CS_VCHK_SINGLE[]CS_VCHK_SINGLE[]*CS_VCHK_SUFFIX[]])
m4_define([CS_VCHK_OPTSEQUENCE], [CS_VCHK_PREFIX[]CS_VCHK_SINGLE[]*CS_VCHK_SUFFIX[]])
m4_define([CS_VCHK_REXSEQ], [m4_bpatsubst($1, [$2], [[]CS_VCHK_SEQUENCE[]])])
m4_define([CS_VCHK_GROUPINGON], [m4_pushdef([CS_VCHK_PREFIX], [CS_VCHK_GROUPPREFIX])m4_pushdef([CS_VCHK_SUFFIX], [CS_VCHK_GROUPSUFFIX])])
m4_define([CS_VCHK_GROUPINGOFF], [m4_popdef([CS_VCHK_SUFFIX])m4_popdef([CS_VCHK_PREFIX])])
m4_define([CS_VCHK_OPTON], [m4_pushdef([CS_VCHK_SEQUENCE], [CS_VCHK_OPTSEQUENCE])])
m4_define([CS_VCHK_OPTOFF], [m4_popdef([CS_VCHK_SEQUENCE])])
dnl ****************************************************************
dnl **                      FlagsOn / FlagsOff                    **
dnl ****************************************************************
m4_define([CS_VCHK_FLAGSON], [dnl
m4_if(  $#,       0,,
        $1,,,
       [$1], [group], [CS_VCHK_GROUPINGON[]],
       [$1],   [opt], [CS_VCHK_OPTON[]])dnl
m4_if( $#, 0,, $1,,,[CS_VCHK_FLAGSON(m4_shift($@))])])dnl
dnl
m4_define([CS_VCHK_FLAGSOFF], [dnl
m4_if(   $#,       0,,
         $1,,,
       $1, [group], [CS_VCHK_GROUPINGOFF[]],
       [$1],   [opt], [CS_VCHK_OPTOFF[]])dnl
m4_if( $#, 0,, $1,,,[CS_VCHK_FLAGSOFF(m4_shift($@))])])dnl
dnl ****************************************************************
dnl **                      rexify / sedify                       **
dnl ****************************************************************
m4_define([CS_VCHK_REXIFY], [m4_pushdef([CS_VCHK_SINGLE], [$1])dnl
CS_VCHK_FLAGSON(m4_shift(m4_shift(m4_shift($@))))dnl
CS_VCHK_REXSEQ([$3], [$2])dnl
CS_VCHK_FLAGSOFF(m4_shift(m4_shift(m4_shift($@))))dnl
m4_popdef([CS_VCHK_SINGLE])])dnl
dnl
m4_define([CS_VCHK_QUOTESEP], [m4_bpatsubst($1, [[^9_]], [\\\&])])
dnl
m4_define([CS_VCHK_REXCHAR], [CS_VCHK_REXIFY([CS_VCHK_CHAR], [__*], $@)])
m4_define([CS_VCHK_REXDIGIT],  [CS_VCHK_REXIFY([CS_VCHK_DIGIT], [99*], $@)])
m4_define([CS_VCHK_SEDIFY], [CS_VCHK_REXDIGIT([CS_VCHK_REXCHAR([CS_VCHK_QUOTESEP([$1])], m4_shift($@))], m4_shift($@))])
m4_define([CS_VCHK_SEDEXPRALL], [/CS_VCHK_SEDIFY([$1])/!d;s/.*\(CS_VCHK_SEDIFY([$1])\).*/\1/])
m4_define([CS_VCHK_SEDEXPRNTH], [/CS_VCHK_SEDIFY([$1])/!d;s/.*CS_VCHK_SEDIFY([$1],[group]).*/\$2/])
dnl
dnl ****************************************************************
dnl **                      Pattern splitting                     **
dnl ****************************************************************
m4_define([CS_VCHK_SPLITSEP], [CS_VCHK_REXIFY([s], [[^9_][^9_]*], $@)])
m4_define([CS_VCHK_SPLITDIGIT], [CS_VCHK_REXIFY([d], [99*], $@)])
m4_define([CS_VCHK_SPLITCHAR], [CS_VCHK_REXIFY([c], [__*], $@)])
dnl
dnl ****************************************************************
dnl ** return a list of 's' 'd' 'c' 'e' chars denoting the kind   **
dnl ** pattern parts: separator, digit, char, end                 **
dnl ****************************************************************
m4_define([CS_VCHK_PATTERNLIST], [m4_pushdef([CS_VCHK_SEQUENCE], [CS_VCHK_SINGLE ])dnl
m4_translit(CS_VCHK_SPLITDIGIT([CS_VCHK_SPLITCHAR([CS_VCHK_SPLITSEP([$1])])]), [ ], m4_if([$2],,[ ],[$2]))e[]dnl
m4_popdef([CS_VCHK_SEQUENCE])])
dnl
dnl ****************************************************************
dnl ** Build the shell commands we emmit into ./configure         **
dnl **                                                            **
dnl ****************************************************************
m4_define([CS_VCHK_PATCOUNT], [len(m4_bpatsubst(CS_VCHK_PATTERNLIST([$1]), [[^dc]]))])
dnl CS_VCHK_EXTRACTVERSION(EXTRACT_CALL, MIN_VERSION, PATTERN, PRGPREFIX, COMPARISION)
m4_define([CS_VCHK_EXTRACTVERSION], 
[dnl
ac_[]$4[]_is_version=`$1 | sed 'CS_VCHK_SEDEXPRALL([$3])'`
ac_[]$4[]_min_version=`echo $2 | sed 'CS_VCHK_SEDEXPRALL([$3])'`
CS_VCHK_RUNTH( CS_VCHK_PATCOUNT([$3]),
	[ac_[]$4[]_is_version_[]i=`echo $ac_[]$4[]_is_version | sed 'CS_VCHK_SEDEXPRNTH([$3], [i])'`
])dnl
CS_VCHK_RUNTH( CS_VCHK_PATCOUNT([$3]),
	[ac_[]$4[]_min_version_[]i=`echo $ac_[]$4[]_min_version | sed 'CS_VCHK_SEDEXPRNTH([$3], [i])'`
])dnl
ac_cv_[]$4[]_bad_luck=
CS_VCHK_RUNTH( CS_VCHK_PATCOUNT([$3]),
	[test -z $ac_cv_[]$4[]_bad_luck && expr "$ac_[]$4[]_is_version_[]i" "$5" "$ac_[]$4[]_min_version_[]i" >/dev/null || ac_cv_[]$4[]_bad_luck=yes
])dnl
])

##############################################################################
# AC_CHECK_PROG_VERSION(PRG, EXTRACT_CALL, VERSION, PATTERN, PREFIX [,
#                       YES_ACTION, [NO_ACTION, [CMP]]])
# Check the version of a program PRG. 
# Version information is emitted by EXTRACT_CALL.
# Program version is compared against VERSION.
# The pattern of the version string matches PATTERN
# PREFIX will be part of the generated shell variables. It is meant to make
# the variables unique.
# The extracted version and the supplied version are compared with the CMP op,
# i.e: EXTRACTED_VERSION CMP SUPPLIED_VERSION
# CMP defaults to >=
# YES_ACTION is taken if comparision yields true. NO_ACTION if not.
#
# PATTERN literals: 9 .. marks a non empty sequence of digits
#                   _ .. marks a non empty sequence of characters from [a-zA-Z]
#                     .. everything else is taken as separator -you better dont 
#                        try stuff like space, slash or comma
#
# The test results in ac_cv_PREFIX_bad_luck being either empty or equal to
# string yes.
##############################################################################
AC_DEFUN(AC_CHECK_PROG_VERSION,
[dnl
m4_define([DO_CMP], [m4_if($8,,[>=],[$8])])
AC_MSG_CHECKING([for $1 - version DO_CMP $3])
AC_CACHE_VAL(ac_cv_$5_bad_luck, 
             [CS_VCHK_EXTRACTVERSION([$2], [$3], [$4], [$5], [DO_CMP])])
if test -z $ac_cv_$5_bad_luck ; then
   AC_MSG_RESULT(yes)
   m4_if([$6], , :, [$6])
else
   AC_MSG_RESULT([no (version $ac_$5_is_version)])
   m4_if([$7], , :, [$7])
fi
])

############################################
# Some stock checks
###########################################
AC_DEFUN(AC_CHECK_BISON_VERSION, [dnl
AC_CHECK_PROG_VERSION([bison], [bison -V], 
		      [$1], [9.9], [bsn], [$2], [$3], [$4])
])

AC_DEFUN(AC_CHECK_SWIG_VERSION,[dnl
AC_CHECK_PROG_VERSION([swig], [swig -version 2>&1], 
		      [$1], [9.9.9], [swig], [$2], [$3], [$4])
])
