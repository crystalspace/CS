#! /bin/sh
# Checks for an installed nasm

# The next shell function is stolen from autoconf
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
        echo "$as_dir/$app$exe"
	IFS="$SAVE_IFS"
	return 0
      fi
    done
  done

  IFS="$SAVE_IFS"
  return 1
}

