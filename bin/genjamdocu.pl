#!/usr/bin/perl -w

# Scan jam files
$find=`find -name "*.jam"`;
@files=split("\n", $find);

print <<'_END_' ;
@node Jam
@chapter Jam Macros


_END_

foreach $f (@files)
{
  open (IN, $f) or die "Couldn't open $f\n";

  while (defined ($i = <IN>))
  {
    if ($i =~ m/##  (\w+)\s*(.*)$/)
    {
      if (defined $section)
      {
	print "$section\n";
      }
      $section = "";

      print "\@strong{$1} $2\n";
      print "\n";
    }
    if ($i =~ m/##    (\w+.*)$/)
    {
      $section.="$1\n";
    }
  }

  close (IN);
}

if (defined $section)
{
  print "$section\n";
}

