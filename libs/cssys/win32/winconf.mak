# This makefile fragment is utilized by winconf.bat user-shell determination.
# It tests whether or not the shell understands the single-quote character.
# The `testecho' target is only defined if DO_WIN_TEST_ECHO is set to `yes',
# and this variable is only set to `yes' by winconf.bat.  Therefore, even if
# this makefile gets included by some other facility, it will act as a no-op in
# that situation.

ifeq (yes,$(DO_WIN_TEST_ECHO))

testecho:
	echo 'testing'>conftest.2

endif
