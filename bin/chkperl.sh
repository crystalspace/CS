
precondition '-n "${LINK}"'

PERLXSI=include/cssys/csperlxs.c

PERL5=`checktool perl5`
if [ ! -x "$PERL5" ]; then
  PERL5=`checktool perl`
fi
if [ ! -x "$PERL5" ]; then
  PERL5_OK=0
else
  PERL5_OK=1
  PERL=$PERL5

  echo "PERL5=$PERL5"
  echo "PERL=$PERL"

  msg_checking 'for ExtUtils::Embed perl module'
  if "$PERL5" -MExtUtils::Embed -e 0; then
    msg_result yes

    PERL5_CFLAGS=`"$PERL5" -MExtUtils::Embed -e ccopts`
    PERL5_LFLAGS=`"$PERL5" -MExtUtils::Embed -e ldopts`

    msg_checking 'for DynaLoader perl module'
    if "$PERL5" -MDynaLoader -e 0; then
      DYNA=DynaLoader
      msg_result yes
    else
      DYNA=
      msg_result no
    fi

    msg_inform 'Generating csperlxs.c'
    "$PERL5" -MExtUtils::Embed -e xsinit -- \
      -o $PERLXSI -std $DYNA cspace
  else
    msg_result no
    msg_inform 'Recommend you download the latest release of Perl 5'

    msg_checking 'for Config perl module'
    if "$PERL5" -MConfig -e 0; then
      msg_result yes

      PERL5_CORE="`"$PERL5" -MConfig -e \
        'print $Config{sitearch}'`/CORE"
      PERL5_CFLAGS="`"$PERL5" -MConfig -e \
        'print $Config{ccflags}'` -I$PERL5_CORE"
      PERL5_LFLAGS="`"$PERL5" -MConfig -e \
        'print $Config{ldflags}, " ", $Config{libs}'` -L$PERL5_CORE"
    else
      msg_result no

      PERL5_CORE="`"$PERL5" -V:sitearch | "$PERL5" -e \
        '<STDIN> =~ m/\x27(.*)\x27/; print $1'`/CORE"
      PERL5_CFLAGS="`"$PERL5" -V:ccflags | "$PERL5" -e \
        '<STDIN> =~ m/\x27(.*)\x27/; print $1'` -I$PERL5_CORE"
      PERL5_LFLAGS="`"$PERL5" -V:ldflags | "$PERL5" -e \
        '<STDIN> =~ m/\x27(.*)\x27/; print $1'` \
                    `"$PERL5" -V:libs | "$PERL5" -e \
        '<STDIN> =~ m/\x27(.*)\x27/; print $1'` -L$PERL5_CORE"
    fi

    msg_inform 'Generating boilerplate csperlxs.c'
    cat >$PERLXSI <<EOF
#if defined(__cplusplus) && !defined(PERL_OBJECT)
  #define is_cplusplus
#endif
#ifdef is_cplusplus
	extern "C" {
#endif
  #include <EXTERN.h>
  #include <perl.h>
  #ifdef PERL_OBJECT
    #define NO_XSLOCKS
    #include <XSUB.h>
    #include "win32iop.h"
    #include <fcntl.h>
    #include <perlhost.h>
  #endif
#ifdef is_cplusplus
	}
  #ifndef EXTERN_C
    #define EXTERN_C extern "C"
  #endif
#else
  #ifndef EXTERN_C
    #define EXTERN_C extern
  #endif
#endif
EXTERN_C void xs_init (pTHXo);
EXTERN_C void boot_DynaLoader (pTHXo_ CV* cv);
EXTERN_C void boot_cspace (pTHXo_ CV* cv);
EXTERN_C void xs_init(pTHXo)
{
  char *file = __FILE__;
  dXSUB_SYS;
  /* DynaLoader is a special case */
  newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
  newXS("cspace::bootstrap", boot_cspace, file);
}
EOF
  fi

  msg_checking 'for perl5 cflags'
  echo "PERL5.CFLAGS=$PERL5_CFLAGS"
  msg_result "$PERL5_CFLAGS"

  msg_checking 'for perl5 lflags'
  echo "PERL5.LFLAGS=$PERL5_LFLAGS"
  msg_result "$PERL5_LFLAGS"
fi

postcondition '${PERL5_OK} -eq 0 -o ${PERL5_OK} -eq 1'

