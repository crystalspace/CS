#!/bin/sh
# This script prepares CS documentation to work with the usercomment.php script

EFED=bin/efed.pl

DIRS="out/docs/pubapi/ out/docs/html/"

for i in $DIRS; do
	if test -d $i; then
		echo "Converting to php in $i..."
		$EFED -d -r "s:html:php:" -e "s/\.html/\.php/g" -e "s/href *= *\"(?!http\:\/\/)([^\"]*)\.html([^\"]*)\"/href=\"\$1.php\$2\"/gi" -e "s:<title>(.*)</title>:<title>\$1</title><?php \\\$theme=\"\$1\"; ?>:i" -e "s:</body>:<?php require(\"annotate.php\"); ?></body>:i" $i

		cp docs/support/phpdocs/*.php $i
	fi
done
echo ok
