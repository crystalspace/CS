# AC_PROG_JAM
#
# Checks if jam/mr is installed. If the script can't find jam some usefull
# informations are displayed for the user.
#
# (c)2002 Matze Braun <matze@braunis.de>
AC_DEFUN(CS_PROG_JAM,
[
AC_ARG_ENABLE(jamtest, AC_HELP_STRING([--disable-jamtest],
	[Do not try to check for jam/mr tool]), ,enable_jamtest=yes )

AS_IF([test "$enable_jamtest" = yes],[
  AC_CHECK_PROGS([JAM], [jam])
  AS_IF([test -n "$JAM"],
      [ifelse([$1], , :, [$1])],
      [ifelse([$2], , :, [$2])])
])

])

