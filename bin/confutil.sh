#! /bin/sh
#==============================================================================
# Configuration helper functions.
#==============================================================================

#------------------------------------------------------------------------------
# Functions for reporting progress and results to users.  
#
# msg_checking()
#    Reports that a check is in progress.
# msg_result()
#    Reports the result of a check.
# msg_inform()
#    Provides an informational message to the user.
#
# These functions write to the standard error stream rather than the standard
# output stream since synthesized makefile commands are emitted to the standard
# output stream by other functions.  Logic for determining ac_n, ac_c, and ac_t
# was taken directly from autoconf.  Note that there is an embedded newline in
# one of the ac_c values.  Do not remove it.
#------------------------------------------------------------------------------

if (echo "testing\c"; echo 1,2,3) | grep c >/dev/null; then
  if (echo -n testing; echo 1,2,3) | sed s/-n/xn/ | grep xn >/dev/null; then
    ac_n= ac_c='
' ac_t='	'
  else
    ac_n=-n ac_c= ac_t=
  fi
else
  ac_n= ac_c='\c' ac_t=
fi

msg_checking()
{
  echo ${ac_n} "checking "$@"... ${ac_c}" 1>&2
}

msg_result()
{
  echo "${ac_t}"$@ 1>&2
}

msg_inform()
{
  echo $@ 1>&2
}


#------------------------------------------------------------------------------
# Pre- and post-condition functions.  Employ these functions at entry to and
# exit from code blocks to ensure that specified conditions are met.  The
# given conditions will be evaluated with the `test' command.
#
# precondition()
#    Assert that the specified condition is true upon entry to a block.
# postcondition()
#    Assert that the specified condition is true upon exit from a block.
#
# Example: precondition '-n "${CXX}"'
#    Ensure that CXX has a value prior to entry to a block.
# Example: postcondition '${X11_OK} -eq 0 -o ${X11_OK} -eq 1'
#    Ensure that X11_OK is set to 0 or 1 after exit from a block.
#------------------------------------------------------------------------------

# Private helper function.
checkcondition()
{
  cnd=$1
  msg=$2
  eval "test $cnd"
  if [ $? -ne 0 ]; then
    echo "$msg failed: $cnd" 1>&2
    exit 1
  fi
}

precondition()
{
  checkcondition "$1" "Precondition"
}

postcondition()
{
  checkcondition "$1" "Postcondition"
}


#------------------------------------------------------------------------------
# shellprotect()
#    Escape special characters in argument which is assumed to be a pathname
#    in order to protect them from the shell.
#------------------------------------------------------------------------------

shellprotect()
{
  echo `echo "$1" | sed -e 's/\\\\/\\\\\\\\/g' -e 's/ /\\\\ /g'`
}


#------------------------------------------------------------------------------
# checkprog()
#    Determine if program specified as argument to this function is in PATH.
#    If the program is found, then its full path is printed and true is
#    returned, otherwise nothing is printed and false is returned.
#    Functionality stolen from autoconf.
#------------------------------------------------------------------------------

exec_extensions=".exe"
echo "#! /bin/sh" > conftest.sh
echo "exit 0" >> conftest.sh
chmod +x conftest.sh
if (PATH=".;."; conftest.sh) >/dev/null 2>&1; then
  PATH_SEPARATOR=';'
else
  PATH_SEPARATOR=':'
fi
rm -f conftest.sh

checkprog()
{
  app=$1
  SAVE_IFS="$IFS"
  IFS=$PATH_SEPARATOR

  for as_dir in $PATH; do
    [ -z "$as_dir" ] && as_dir=.
    for exe in '' $exec_extensions; do
      if [ -f "$as_dir/$app$exe" ]; then
	IFS="$SAVE_IFS"
        echo "$as_dir/$app$exe"
	return 0
      fi
    done
  done

  IFS="$SAVE_IFS"
  return 1
}


#------------------------------------------------------------------------------
# checktool()
#    Determine if tool specified as argument to function is in PATH.  This is
#    identical to checkprog() except that it employs the msg_checking() and
#    msg_result() functions to provide status information to the user.
#------------------------------------------------------------------------------

checktool()
{
  app=$1
  msg_checking "for ${app}"
  rc=`checkprog ${app}`
  if [ -n "${rc}" ]; then
    echo "${rc}"
    msg_result "${rc}"
    return 0
  else
    msg_result "no"
    return 1
  fi
}
