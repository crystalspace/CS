#!/usr/bin/perl -w
#==============================================================================
#
#    Copyright (C) 2002 by F.Richter <f.richter@gmx.li>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
#==============================================================================

# Purpose of this script: Create an HTMLHELP TOC out of the Crystal Space docs.
# HHW's automatic TOC generation is not entirely poor, but it does not handle
# the CS docs too well, thus we do it ourselves.  This script is not really
# useful for anything other than the CS docs.  The HH compiler will complain
# about unresolveable URLs in the TOC. Don't worry, it'll work fine, though.

use strict;
use HTMLEntities;

#
# the index needs some special treatment
#
sub doctoc_index {
  	(my $docroot, my $srcfile, my $TOCFILE) = @_;
  	my %indexfiles;
  	my %letterindexed;

  	open(my $INDEXFILE, "<$docroot/$srcfile");
  	while (<$INDEXFILE>) {
  			chomp $_;
				if ( /<A HREF=\"(.*?)\#(.*?)\" class=\"summary-letter\"><b>(.*?)<\/b><\/A>/i ) { # a letter
						if (!exists($letterindexed{$3})) {
  							print $TOCFILE <<EOTOCENTRY;
<LI> <OBJECT type="text/sitemap">
  <param name="Name" value="$3">
  <param name="Local" value="$docroot\\$1#$2">
</OBJECT>
EOTOCENTRY
								$indexfiles{$1} = 1;
								$letterindexed{$3} = 1;
						}
				}
  	}
  	close($INDEXFILE);
  	
  	my %indexwords;
  	
  	foreach (keys %indexfiles) {
  			open(my $INDEXFILE, "<$docroot/$_");
  			while (<$INDEXFILE>) {
  					chomp $_;
  					if ( /<TR><TD><\/TD><TD valign=\"top\"><A HREF=\"(.*?)\">(.*?)<\/A><\/TD><TD valign=\"top\"><A HREF=\".*?\">(.*?)<\/A><\/TD><\/TR>/i ) {
		  					my $ilink = $1;
		  					my $igroup = $2;
		  					my $ititle = $3;
		  					$igroup =~ s/(<.*?>)//g; # strip html code
		  					$igroup = HTMLEntities::encode($igroup);
		  					$ititle =~ s/(<.*?>)//g; 
		  					#$ititle =~ s/^([[:digit:]\.]*?) //; # strip leading section number
		  					$ititle = HTMLEntities::encode($ititle);
 								push(@{$indexwords{$igroup}}, [$ititle, $ilink]);
  					}
  			}
  			close($INDEXFILE);
  	}
  	
  	open(my $HHKFILE, ">manualindex.hhk");
	  print $HHKFILE <<EOHHKHEAD;
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta name="GENERATOR" content="gendoctoc.pl">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<UL>
EOHHKHEAD
 
  	foreach (sort {uc($a) cmp uc($b)} (keys %indexwords)) {
  			print $HHKFILE <<EOHHKENTRY;
<LI> <OBJECT type="text/sitemap">
<param name="Name" value="$_">
<param name="Local" value="#">
</OBJECT>
<UL>
EOHHKENTRY
  			foreach (@{$indexwords{$_}}) {
  				 	(my $ititle, my $ilink) = @{$_};
  			print $HHKFILE <<EOHHKENTRY;
<LI> <OBJECT type="text/sitemap">
<param name="Name" value="$ititle">
<param name="Local" value="$docroot\\$ilink">
</OBJECT>
EOHHKENTRY
  			}
				print $HHKFILE "<\/UL>\n";
  	}

	  print $HHKFILE <<EOHHKEND;
</UL>
</BODY></HTML>
EOHHKEND
  	
  	close($HHKFILE);
}

#
# parse a section
#
sub doctoc_section {
  	(my $docroot, my $srcfile, my $TOCFILE) = @_;

  	open(my $INDEXFILE, "<$docroot/$srcfile");
  	while (<$INDEXFILE>) {
  			chomp $_;
				if ( ( /<A NAME=\".*?\" HREF=\"(.*?)\">(.*?)<\/A>/i ) # like in cs_ovr.html
		  			|| ( /<TR><TD ALIGN=\"left\" VALIGN=\"TOP\"><A HREF=\"(.*?)\">(.*?)<\/A><\/TD><TD>&nbsp;&nbsp;<\/TD><TD ALIGN=\"left\" VALIGN=\"TOP\">/i ) ) { # every other file
		  			my $seclink = $1;
		  			(my $sectitle = $2) =~ s/(<.*?>)//g; # strip html code
		  			$sectitle = HTMLEntities::encode($sectitle);
  					print $TOCFILE <<EOTOCENTRY;
<LI> <OBJECT type="text/sitemap">
  <param name="Name" value="$sectitle">
  <param name="Local" value="$docroot\\$seclink">
</OBJECT>
EOTOCENTRY
						if ( $sectitle eq "Index" ) {
								print $TOCFILE "<UL>";
								(my $indexfile) = ($seclink =~ /(.*)\#.*/);
								doctoc_index($docroot, $indexfile, $TOCFILE);
								print $TOCFILE "<\/UL>\n";
						} elsif ( $seclink =~ /(.*)\#(.*)/ ) {
								print $TOCFILE "<UL>";
								doctoc_section($docroot, $1, $TOCFILE);
								print $TOCFILE "<\/UL>\n";
						}
				}
  	}
  	close($INDEXFILE);
}

#
# create csdocs TOC.
#
sub doctoc {
		my $docroot = shift @_;
	
	  open(my $TOCFILE, ">manualtoc.hhc");
	  
	  print $TOCFILE <<EOTOCHEAD;
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta name="GENERATOR" content="gendoctoc.pl">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<OBJECT type="text/site properties">
</OBJECT>
<UL>
<LI> <OBJECT type="text/sitemap">
  <param name="Name" value="Crystal Space Documentation">
  <param name="Local" value="$docroot\\index.html">
</OBJECT>
<UL>
<LI> <OBJECT type="text/sitemap">
  <param name="Name" value="Table of contents">
  <param name="Local" value="$docroot\\cs_toc.html">
</OBJECT>
<LI> <OBJECT type="text/sitemap">
  <param name="Name" value="About this document">
  <param name="Local" value="$docroot\\cs_abt.html">
</OBJECT>
EOTOCHEAD
  
	  doctoc_section($docroot, "cs_ovr.html", $TOCFILE);
	  
	  print $TOCFILE <<EOTOCEND;
</UL>
</UL>
</BODY></HTML>
EOTOCEND
  	close($TOCFILE);
}

#
# parse Compound List
#
sub api_compound_list {
  	(my $apiroot, my $srcfile, my $TOCFILE) = @_;

  	open(my $SOURCEFILE, "<$apiroot/$srcfile");
  	while (<$SOURCEFILE>) {
  			chomp $_;
				if ( /<li><a class=\"el\" href=\"(.*?)\">(.*?)<\/a>/ ) { 
		  			my $clink = $1;
		  			my $ctitle = HTMLEntities::encode($2);
  					print $TOCFILE <<EOTOCENTRY;
<LI> <OBJECT type="text/sitemap">
  <param name="Name" value="$ctitle">
  <param name="Local" value="$apiroot\\$clink">
</OBJECT>
EOTOCENTRY
				}
  	}
  	close($SOURCEFILE);
}

#
# parse Compound Members
#
sub api_compound_members {
  	(my $apiroot, my $srcfile, my $TOCFILE) = @_;

  	open(my $SOURCEFILE, "<$apiroot/$srcfile");
  	while (<$SOURCEFILE>) {
  			chomp $_;
				if ( /<a name=\"(.*?)\"><h3>- (.) -<\/h3><\/a>/ ) { 
  					print $TOCFILE <<EOTOCENTRY;
<LI> <OBJECT type="text/sitemap">
  <param name="Name" value="$2">
  <param name="Local" value="$apiroot\\$srcfile#$1">
</OBJECT>
EOTOCENTRY
				}
  	}
  	close($SOURCEFILE);
}

#
# parse File List
#
sub api_file_list {
  	(my $apiroot, my $srcfile, my $TOCFILE) = @_;

  	open(my $SOURCEFILE, "<$apiroot/$srcfile");
  	while (<$SOURCEFILE>) {
  			while ( /<li><b>(.*?)<\/b> <a href=\"(.*?)\">\[code\]<\/a>(.*)/ ) { 
  					print $TOCFILE <<EOTOCENTRY;
<LI> <OBJECT type="text/sitemap">
  <param name="Name" value="$1">
  <param name="Local" value="$apiroot\\$2">
</OBJECT>
EOTOCENTRY
						$_ = $3;
				}
  	}
  	close($SOURCEFILE);
}

#
# create csapi TOC.
#
sub apitoc {
		my $apiroot = shift @_;
	  open(my $TOCFILE, ">apitoc.hhc");
	  
	  print $TOCFILE <<EOTOCHEAD;
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta name="GENERATOR" content="gendoctoc.pl">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<OBJECT type="text/site properties">
</OBJECT>
<UL>
	<LI> <OBJECT type="text/sitemap">
		<param name="Name" value="Main Page">
		<param name="Local" value="$apiroot\\index.html">
		</OBJECT>
	<LI> <OBJECT type="text/sitemap">
		<param name="Name" value="Class Hierarchy">
		<param name="Local" value="$apiroot\\hierarchy.html">
		</OBJECT>
EOTOCHEAD

	  print $TOCFILE <<EOTOCHEAD;
	<LI> <OBJECT type="text/sitemap">
		<param name="Name" value="Compound List">
		<param name="Local" value="$apiroot\\annotated.html">
		</OBJECT>
		<UL>
EOTOCHEAD

		api_compound_list($apiroot, "annotated.html", $TOCFILE);

	  print $TOCFILE <<EOTOCHEAD;
	</UL>
	<LI> <OBJECT type="text/sitemap">
		<param name="Name" value="Compound Members">
		<param name="Local" value="$apiroot\\functions.html">
		</OBJECT>
		<UL>
EOTOCHEAD

		api_compound_members($apiroot, "functions.html", $TOCFILE);

	  print $TOCFILE <<EOTOCHEAD;
	</UL>
	<LI> <OBJECT type="text/sitemap">
		<param name="Name" value="Files">
		<param name="Local" value="$apiroot\\files.html">
		</OBJECT>
		<UL>
EOTOCHEAD
  
		api_file_list($apiroot, "files.html", $TOCFILE);
	  
	  print $TOCFILE <<EOTOCEND;
</UL>
</UL>
</UL>
</BODY></HTML>
EOTOCEND
  	close($TOCFILE);
}

if ($ARGV[0] eq "manual") {
  doctoc($ARGV[1]);
}
elsif ($ARGV[0] eq "api") {
  apitoc($ARGV[1]);
}
else {
  die "Unrecognized documentation selector: \"$ARGV[0]\"\n" .
    "Use either \"manual\" or \"api\"\n";
}
