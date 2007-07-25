#!/bin/sh
# This script prepares CS documentation for online publication by adding
# user-annotation capability.

DIRS=$@
EFED="perl bin/efed.pl"

for d in $DIRS; do
  if test -d $d; then
    echo "Preparing $d for annotation capability."
    $EFED -d \
      -r "html=php" \
      -e "s/href *= *\"(?!http\:\/\/)([^\"]*)\.html([^\"]*)\"/href=\"\$1.php\$2\"/gi" \
      -e "s:<title>(.*)</title>:<title>\$1</title><?php \\\$theme=\"\$1\"; ?>:i" \
      -e "s:</body>:<?php require(\"annotate.php\"); ?></body>:i" \
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
    cp docs/support/annotate/*.php $d
  fi
done
