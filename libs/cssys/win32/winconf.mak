# This makefile fragment is utilized by win32conf.bat user-shell determination.
# It tests whether or not the shell understands the single-quote character.

testecho:
	echo 'testing'>conftest.2
