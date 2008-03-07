#!/bin/sh
#------------------------------------------------------------------------------
# This script prepares CS documentation for online publication by performing
# the following tasks:
#
# - Rename generated .html files to .php so that generated documentation can
#   support programmatic features if necessary.
#
# - Fix internal links to reference new .php extension rather than .html.
#
# - Add a "Search" field.
#
# The script accepts as arguments the paths of directories which it should
# transform. It will process the named directories recursively.
#
# Typically, this script is run as a "browseable-postprocess" phase of a
# "jobber" documentation generation run.
#------------------------------------------------------------------------------

DIRS=$@
EFED="perl bin/efed.pl"

for d in $DIRS; do
  if test -d $d; then
    echo "Preparing $d for online publication."
    $EFED -d \
      -r "html=php" \
      -e "s/href *= *\"(?!http\:\/\/)([^\"]*)\.html([^\"]*)\"/href=\"\$1.php\$2\"/gi" \
      -e "s!(<li><a href=\"pages.php\"><span>Related&nbsp;Pages</span></a></li>)!\
\$1\
<li><form class=\"search\" action=\"/cgi-bin/search.cgi\" method=\"get\">\
<table cellspacing=\"0\" border=\"0\">\
<tr><td><label>Search:</label></td>\
<td><input class=\"search\" type=\"text\" name=\"q\" value=\"\" size=\"20\"/>
<input type=\"hidden\" name=\"tag\" value=\"docs\" /></td></tr>\
</table></form></li>!" \
      -e "s!( \? </a>]</td>)!\$1
<td align=\"left\" valign=\"middle\">[ Search: \
<form style=\"display:inline;\" class=\"search\" action=\"/cgi-bin/search.cgi\" method=\"get\">\
<input class=\"search\" type=\"text\" name=\"q\" value=\"\" size=\"20\"/>\
<input type=\"hidden\" name=\"tag\" value=\"docs\" />\
</form> ]</td>!i" \
      $d
  fi
done
