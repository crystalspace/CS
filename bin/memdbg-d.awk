#!/bin/awk -f
#
# This script is intended to extract all useful information from output of
#
# objdump --debugging
#
# command. Note that --debugging switch is supported only by
# the version of objdump from binutils > 2.8.x

# The startup function that performs initialization
BEGIN {
  FS = " ";
  modname = ""; newmod  = 0;
  funname = ""; funaddr = -1;
  labname = ""; labaddr = -1;
  baseaddr = 0;
}

/^{/ {
  # Filter out accidental records (that do not start with alphanumeric)
  if (funname !~ /^[a-zA-Z0-9_].*/)
    next

  # Remove C-style comments and also "demangle" the name
  numcomp = split (funname, funcomp);
  funname = "";
  comment = 0;
  for (i = 1; i <= numcomp; i++)
  {
    token = funcomp [i];

    if (token ~ "\/[*]")
    {
      comment = 1;
      gsub ("\/[*].*", "", token);
      if (token == "")
        continue;
    }

    if (token ~ "[*]\/")
    {
      comment = 0;
      gsub (".*[*]/", "", token);
      if (token == "")
        continue;
    }

#    if (funcomp [i + 1] ~ /^[(]/)
#    {
#      # current token seems to be function name
#    }

    if (!comment)
      if (funname == "")
        funname = token;
      else
        funname = funname " " token
  }

#  gsub (/\/[*].*[*]\//, "", funname);

  funaddr = $3
  sub ("^0x", "", funaddr);

  if (newmod)
  {
    newmod = 0;
    print "S", funaddr, modname
  }

  print "F", funaddr, funname

  # Now read all lines until we find the closing '}'
  depth = 1;
  while (depth)
  {
    if (getline == -1)
      break;
    if ($1 == "{")
      depth++;
    else if ($1 == "}")
      depth--;
    else if ($1 == "/*")
    {
      sub ("^0x", "", $7);
      print "L", $7, $5
    }
  }
}

/.*:$/ {
  modname = $0;
  sub (":$", "", modname);
  newmod = 1;
}

{
  funname = $0
}
