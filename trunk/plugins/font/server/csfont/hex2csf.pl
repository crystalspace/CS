#!/usr/bin/perl
#----------------------------------------------------------------------------
#
# hex2csf.pl
# Converter from the unifont hex to the CrystalSpace font format 
# Heavily modified version of hex2bdf
#
#----------------------------------------------------------------------------


while (<>) { $glyph{$1} = $2 if /(....):([0-9A-Fa-f]{32,64})\n/; }
$glyph{"FFFF"} = "00007e424242424242424242427e0000";

@chars = sort keys %glyph; 
# dbmopen (%charname, "/usr/share/unicode/unidata/charname.db", 0);

binmode stdout, ":raw";
print stdout "CSF [Font=gnu-unifont.16 Height=16 Ascent=14 Descent=2 HasCharRanges=1]\n";

$ranges = "";
$widths = "";
$bitdata = "";

$firstConsecChar = 0;
$numConsecChars = 0;

foreach $character (@chars)
{
	$encoding = hex($character); $glyph = $glyph{$character};
	
	if ($encoding != ($firstConsecChar + $numConsecChars))
	{
	  if ($numConsecChars != 0)
	  {
	    $ranges .= pack 'VV', $firstConsecChar, $numConsecChars;
	  }
	  $firstConsecChar = $encoding;
	  $numConsecChars = 0;
	}
	
	$dlen = length ($glyph);
	$width = $dlen > 32 ? 2 : 1;
	$dwidth = $width * 8; 
	$widths .= pack 'C', $dwidth;
	
	$glyphdata = pack ("H$dlen", $glyph);
	$bitdata .= $glyphdata;

	$numConsecChars++;
}

if ($numConsecChars != 0)
{
  $ranges .= pack 'VV', $firstConsecChar, $numConsecChars;
}
$ranges .= pack 'VV', 0, 0;

print stdout "$ranges$widths$bitdata";
binmode stdout, ":crlf";

