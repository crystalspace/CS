#! /bin/sh
#==============================================================================
# Determine if the Perl SDK is available and, if so, which compiler and link
# flags are required.
#
# IMPORTS
#    checktool()
#	Shell function which checks if the program mentioned as its sole
#	argument can be found in the PATH.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    PERL5_OK
#	Shell variable set to 1 if Perl is available, 0 otherwise.
#    PERL5, PERL
#	Shell variables containing the full path to the Perl executable.
#    PERL5.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if Perl is available, otherwise the variable is not set.
#    PERL5.CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags required for Perl.
#    PERL5.LFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags required for Perl.
#    PERL5.EXTUTILS.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if the ExtUtils::Embed package is availble with which the Perl
#	embedding goop can be synthesized, otherwise the variable is not set.
#    PERL5.EXTUTILS.DYNALOADER
#	Makefile variable emitted to the standard output stream.  Value is
#	"DynaLoader" if the DynaLoader module is available for use by
#	ExtUtils::Embed, otherwise it is the empty string.  If ExtUtils::Embed
#	is not available, then this variable is not set.
#==============================================================================

PERL_OK=0
PERL5_CFLAGS=''
PERL5_LFLAGS=''
PERL5_EXTUTILS=0

PERL5=`checktool perl5`
if [ -z "${PERL5}" ]; then
  PERL5=`checktool perl`
fi
if [ -z "${PERL5}" ]; then
  PERL5_OK=0
else
  msg_checking "for ExtUtils::Embed perl module"
  if ${PERL5} -MExtUtils::Embed -e 0 >/dev/null 2>&1; then
    msg_result "yes"

    PERL5_EXTUTILS=1
    PERL5_CFLAGS=`${PERL5} -MExtUtils::Embed -e ccopts 2>/dev/null`
    PERL5_LFLAGS=`${PERL5} -MExtUtils::Embed -e ldopts 2>/dev/null`

    msg_checking "for DynaLoader perl module"
    if ${PERL5} -MDynaLoader -e 0 >/dev/null 2>&1; then
      PERL5_DYNALOADER=DynaLoader
      msg_result "yes"
    else
      PERL5_DYNALOADER=
      msg_result "no"
    fi
  else
    msg_result "no"
    msg_inform "Recommend you install the latest release of Perl 5."
    msg_inform "http://www.perl.org/"

    msg_checking "for Config perl module"
    if ${PERL5} -MConfig -e 0 >/dev/null 2>&1; then
      msg_result "yes"

      PERL5_CORE=`${PERL5} -MConfig -e 'print $Config{sitearch}' 2>/dev/null`
      PERL5_CORE="${PERL5_CORE}/CORE"
      PERL5_CFLAGS=`${PERL5} -MConfig -e 'print $Config{ccflags}' 2>/dev/null`
      PERL5_CFLAGS="${PERL5_CFLAGS} -I${PERL5_CORE}"
      PERL5_LFLAGS=`${PERL5} -MConfig -e \
	'print $Config{ldflags}, " ", $Config{libs}' 2>/dev/null`
      PERL5_LFLAGS="${PERL5_LFLAGS} -L${PERL5_CORE}"
    else
      msg_result "no"

      PERL5_CORE=`${PERL5} -V:sitearch | ${PERL5} -e \
        '<STDIN> =~ m/\x27(.*)\x27/; print $1' 2>/dev/null`
      PERL5_CORE="${PERL5_CORE}/CORE"
      PERL5_CFLAGS=`${PERL5} -V:ccflags | ${PERL5} -e \
        '<STDIN> =~ m/\x27(.*)\x27/; print $1' 2>/dev/null`
      PERL5_CFLAGS="${PERL5_CFLAGS} -I${PERL5_CORE}"
      PERL5_LFLAGS=`${PERL5} -V:ldflags | ${PERL5} -e \
        '<STDIN> =~ m/\x27(.*)\x27/; print $1' 2>/dev/null`
      PERL5_LFLAGS="${PERL5_LFLAGS} `${PERL5} -V:libs | ${PERL5} -e \
        '<STDIN> =~ m/\x27(.*)\x27/; print $1' 2>/dev/null`"
      PERL5_LFLAGS="${PERL5_LFLAGS} -L${PERL5_CORE}"
    fi
  fi

  PERL5_OK=1
  PERL="${PERL5}"

  echo "PERL5.AVAILABLE = yes"
  echo "PERL5 = ${PERL5}"
  echo "PERL = ${PERL}"
  echo "PERL5.CFLAGS = ${PERL5_CFLAGS}"
  echo "PERL5.LFLAGS = ${PERL5_LFLAGS}"

  if [ ${PERL5_EXTUTILS} -eq 1 ]; then
    echo "PERL5.EXTUTILS.AVAILABLE = yes"
    echo "PERL5.EXTUTILS.DYNALOADER = ${PERL5_DYNALOADER}"
  fi

  if [ -n "${PERL5_CFLAGS}" ]; then
    msg_inform "perl5 cflags... ${PERL5_CFLAGS}"
  fi
  if [ -n "${PERL5_LFLAGS}" ]; then
    msg_inform "perl5 lflags... ${PERL5_LFLAGS}"
  fi
fi

postcondition '${PERL5_OK} -eq 0 -o ${PERL5_OK} -eq 1'
