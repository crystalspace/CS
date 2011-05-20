# summary.m4                                                   -*- Autoconf -*-
#==============================================================================
# Copyright (C)2011 by Eric Sunshine <sunshine@sunshineco.com>
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
# CS_NOTABLE(FEATURE, [CATEGORY], RESULT, [CHECK])
#	Take note of the result of a check for later use by an end-of-configure
#	summary report emitted by CS_SUMMARIZE().  FEATURE is a short
#	human-readable name of the feature being checked.  Optional CATEGORY is
#	a short human-readable feature classification, useful for grouping
#	similar features in the summary report. If omitted, it defaults to
#	"other".  RESULT is the result of the check. It can be either a
#	$variable reference or shell code.  If RESULT is a $variable and its
#	value is "no", empty, or unset, then the check is assumed to have
#	failed.  Any other value for $variable is assumed to represent
#	success. If RESULT is shell code, then it is expected to evaluate to
#	true or false to indicate the result of the check. CHECK is the actual
#	check, and typically is an invocation of some other Autoconf macro,
#	such as CS_CHECK_BUILD(), AC_LINK_IFELSE(), etc., but can also be
#	arbitrary shell code.  It is possible to latch the value of a check
#	which has already taken place by specifying RESULT and omitting CHECK.
#------------------------------------------------------------------------------
AC_DEFUN([CS_NOTABLE],
    [$4
    AS_IF(m4_bmatch([$3], [^\$], [test -z "$3" || test "no" = "$3"], [$3]),
        [_cs_sum_ok=no], [_cs_sum_ok=yes])
    _cs_summary="$_cs_summary
$1|m4_default([$2],[other])|$_cs_sum_ok"
    ])


#------------------------------------------------------------------------------
# CS_SUMMARIZE([SUCCESS-HEADING], [FAILURE-HEADING])
#	Generate a report indicating success and failure of checks deemed
#	notable via CS_NOTABLE(). SUCCESS-HEADING and FAILURE-HEADING are the
#	human-readable report headings for the lists of checks which succeeded
#	and failed, respectively. If omitted, they default to "Succeeded" and
#	"Failed".
#------------------------------------------------------------------------------
AC_DEFUN([CS_SUMMARIZE],
    [echo "$_cs_summary" | sort -i -t '|' -k 3,3r -k 1,1 |
    awk '
	BEGIN {
          FS="|";
          heading[["yes"]]="m4_default([$1],[Succeeded])";
          heading[["no"]]="m4_default([$2],[Failed])";
          ok=""
        }
	!/^$/ {
	  if (ok != $[3]) {
            ok=$[3];
            if (NR != 1)
              print "";
            print heading[[ok]]
          }
	  printf "  %-20s (%s)\n", $[1], $[2]
	}'
    ])
