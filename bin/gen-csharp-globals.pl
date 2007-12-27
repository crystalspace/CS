#!/usr/bin/perl
#------------------------------------------------------------------------------
#   C# Global Class Gennerator
#   Copyright (C) 2007 Ronie Salgado <roniesalg@gmail.com>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2, or (at your option)
#   any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor,
#   Boston, MA 02110-1301, USA.
#------------------------------------------------------------------------------
# This script takes C# files, read his public static members, public constants,
# and writes stubs for its in a file, with the end of have all that method in a
# single class. This is useful for simulate globals functions and constants.

# Defaults values and constants
use  strict;

my $ClassName = "Globals";
my $Namespace = "Dummy";
my $Verbose = 0;
my $AppName = "C# Global Class Generator";
my $AppFileName = "gen-csharp-globals.pl";
my $Modifier = "internal";
my $BaseClass = 0;
my $CurrentClass;
my $OutputFile;
my @ExcludeList;
my @InputFiles;
my @Attributes;
my @UsingNamespace = ("System");

# Prints the help information
sub PrintHelp
{

print "Usage: $AppFileName [options] <input files>...";
print STDOUT<<"EOT";
Options:
  -h
  --help                    Display this information
  -v
  --verbose                 Display more information on the work
  -o
  --output                  The output file
  -x
  --exclude                 Functions that must be excluded from the globals
  -c
  --classname               The name of the class that will be generated
  -n
  --namespace               The namespace that will contain the generated class
  -u
  --using-namespace         Additionals using namespaces
  -m
  --modifier                The class modifier. By default is set to "internal"
  -a
  --attribute               Class attribute. By default there aren't any attributes
  -b
  --baseclass               The base class of the generated one
NOTE: The output file is required
EOT
}

# Prints the version information
sub PrintVersion
{
print STDOUT<<"EOT";

EOT
}

# If we are in verbose mode, prints the text, otherwise no
sub PrintVerbose
{
  if($Verbose)
  {
    print $_[0];
  }
}

# Parse the command line
sub ParseCommandLine
{
  my $count;
  my $i;

  $count = @ARGV;

  for($i = 0; $i < $count; $i++)
  {
    if($ARGV[$i] eq '-h' || $ARGV[$i] eq '--help')
    {
      &PrintHelp;
      exit 0;
    }
    elsif($ARGV[$i] eq '--version')
    {
      &PrintVersion;
      exit 0;
    }
    elsif($ARGV[$i] eq '-v' || $ARGV[$i] eq '--verbose')
    {
      $Verbose = 1;
    }
    elsif($ARGV[$i] eq '-o' || $ARGV[$i] eq "--output")
    {
      $i++;
      $OutputFile = $ARGV[$i];
    }
    elsif($ARGV[$i] eq '-x' || $ARGV[$i] eq '--exclude')
    {
      $i++;
      push(@ExcludeList, $ARGV[$i]);
    }
    elsif($ARGV[$i] eq '-c' || $ARGV[$i] eq '--classname')
    {
      $i++;
      $ClassName = $ARGV[$i];
    }
    elsif($ARGV[$i] eq '-n' || $ARGV[$i] eq "--namespace")
    {
      $i++;
      $Namespace = $ARGV[$i];
    }
    elsif($ARGV[$i] eq '-u' || $ARGV[$i] eq '--using-namespace')
    {
      $i++;
      push(@UsingNamespace, $ARGV[$i]);
    }
    elsif($ARGV[$i] eq '-m' || $ARGV[$i] eq '--modifier')
    {
      $i++;
      $Modifier = $ARGV[$i];
    }
    elsif($ARGV[$i] eq '-a' || $ARGV[$i] eq '--attribute')
    {
      $i++;
      push(@Attributes, $ARGV[$i]);
    }
    elsif($ARGV[$i] eq '-b' || $ARGV[$i] eq '--baseclass')
    {
      $i++;
      $BaseClass = $ARGV[$i];
    }
    else
    {
      push(@InputFiles, $ARGV[$i]);
    }
  }
}

# Writes the header data in the output.
sub WriteHeader
{
  my $namespace;
  my $attrnum;
  my $i;

  # Write license
  print OUTPUTFILE <<"EOL";
/* Skeleton for a C# Globals Functions Class.

   Copyright (C) 2007 by Ronie Salgado <roniesalg\@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the C# Global Class Generator skeleton and distribute
   that work under terms of your choice, so long as that work isn't
   itself a C# Global Class Generator using the skeleton or a modified
   version thereof as a parser skeleton.  Alternatively, if you modify
   or redistribute the parser skeleton itself, you may (at your option)
   remove this special exception, which will cause the skeleton and the
   resulting C# Global Class Generator output files to be licensed under
   the GNU General Public License without this special exception.

   This special exception was added by the Ronie Salgado in the first
   release of the C# Global Class Generator.  */

EOL

  # Write generated comment
  print OUTPUTFILE "// WARNING: This file was autommatically generated by $AppName\n";

  # Write using <namespace> directive

  foreach $namespace (@UsingNamespace)
  {
    print OUTPUTFILE "using $namespace;\n";
  }
  print OUTPUTFILE "\n";

  # Write namespace
  print OUTPUTFILE "namespace $Namespace\n";
  print OUTPUTFILE "{\n";

  # Write attributes if there are available
  if($attrnum > 0)
  {
    print OUTPUTFILE "[";
    for($i = 0; $i < $attrnum; $i++)
    {
      print OUTPUTFILE $Attributes[$i];
      if($i+1 < $attrnum )
      {
	print OUTPUTFILE "\n";
      }
    }
    print OUTPUTFILE "]\n";
  }

  # Write class header
  print OUTPUTFILE "\t$Modifier class $ClassName";
  if($BaseClass)
  {
    print OUTPUTFILE ": $BaseClass\n";
  }
  else
  {
    print OUTPUTFILE "\n";
  }
  print OUTPUTFILE "\t{\n";
}

# Writes the foot of the output.
sub WriteFoot
{
  print OUTPUTFILE "\t}; // class $ClassName\n";
  print OUTPUTFILE "}; // namespace $Namespace\n";
}

# Writes a comment about the class which contains this "globals"
sub WriteClassComment
{
  print OUTPUTFILE "\n";
  print OUTPUTFILE "\t\t//=====================================================================\n";
  print OUTPUTFILE "\t\t// $CurrentClass globals\n";
  print OUTPUTFILE "\n";
}

# Outputs a stub of a functions, knowing he's prototype.
sub OutputFunction
{
  my $line;
  my @tokens;
  my ($function_name, $type);
  my ($i, $exclude);

  $line = $_[0];

  # Preprocess the line
  $line =~ s/\).*$/ ) /;
  $line =~ s/\(/ ( /;
  $line =~ s/,/ , /g;
  $line =~ s/[ \t]+/ /g;
  $line =~ s/^[ \t]*//;
  $line =~ s/[ \t]*$//;

  # Separate the line into tokens
  @tokens = split(/ /, $line);
  $type = $tokens[2];

  $function_name = $tokens[3];

  # If the function is on the exclude list, skip it
  foreach $exclude (@ExcludeList)
  {
    if($exclude eq $function_name)
    {
      return;
    }
  }

  # Write prototype
  print OUTPUTFILE "\t\tpublic static $type $function_name(";

  for($i = 5; $i < $#tokens; $i++)
  {
    print OUTPUTFILE "$tokens[$i]";
    if($i < $#tokens-1 && $tokens[$i+1] ne "," )
    {
      print OUTPUTFILE " ";
    }
  }
  print OUTPUTFILE ")\n";
  
  # Write body
  print OUTPUTFILE "\t\t{\n"; # Begin block

  print OUTPUTFILE "\t\t\t";

  if ($type ne "void") # If the function have to return, add a return keyword
  {
    print OUTPUTFILE "return ";
  }
  print OUTPUTFILE "$CurrentClass.$function_name("; # Function name

  # Write arguments
  for($i = 6; $i < $#tokens; $i+=3)
  {
    print OUTPUTFILE "$tokens[$i]";
    if($i+3 < $#tokens)
    {
      print OUTPUTFILE ", ";
    }
  }
  print OUTPUTFILE ");\n"; # End function call
  print OUTPUTFILE "\t\t}\n\n"; # End block
}

# Outputs a static var.
sub OutputStaticConstant
{
  my $line;

  $line = $_[0];
  $line =~ s/;.*$//;
  $line =~ s/[ \t]+/ /g;
  $line =~ s/^[ \t]*//;

  print OUTPUTFILE "\t\t$line;\n";
}

# Outputs a constants, simplely copy it.
sub OutputConstant
{
  my $line;
  $line = $_[0];
  $line =~ s/;.*$//;
  $line =~ s/[ \t]+/ /g;
  $line =~ s/^[ \t]*//;

  print OUTPUTFILE "\t\t$line;\n";
}

sub OutputStaticVariable
{
  my $line;
  my @tokens;
  my $type;
  my $name;

  $line = $_[0];
  $line =~ s/;.*$//;
  $line =~ s/[ \t]+/ /g;
  $line =~ s/^[ \t]*//;

  @tokens = split(/ /, $line);
  $type = $tokens[2];
  $name = $tokens[3];

  # Outputs a static get and set property
  print OUTPUTFILE "\n";
  print OUTPUTFILE "\t\tpublic static $name\n";
  print OUTPUTFILE "\t\t{\n";
  print OUTPUTFILE "\t\t\tget\n";
  print OUTPUTFILE "\t\t\t{\n";
  print OUTPUTFILE "\t\t\t\treturn $CurrentClass.$name;\n";
  print OUTPUTFILE "\t\t\t}\n";
  print OUTPUTFILE "\n";
  print OUTPUTFILE "\t\t\tset\n";
  print OUTPUTFILE "\t\t\t{\n";
  print OUTPUTFILE "\t\t\t\t$CurrentClass.$name = value;\n";
  print OUTPUTFILE "\t\t\t}\n";
  print OUTPUTFILE "\t\t}\n";
  print OUTPUTFILE "\n";
}

sub IsStaticFunction
{
  my $line;
  $line = $_[0];
  $line =~ s/[ \t]+/ /g;
  return $line =~ m/^ ?public static [A-Za-z_]+ [A-Za-z_][A-Za-z0-9_]* ?\(.*\).*$/ ;
}

sub IsConstant
{
  my $line;
  $line = $_[0];
  $line =~ s/=/ = /;
  $line =~ s/[ \t]+/ /g;
  return $line =~ m/^ ?public const [A-Za-z_]+ [A-Za-z_][A-Za-z0-9_]* =.+;.*$/ ;
}

sub IsClass
{
  my $line;
  $line = $_[0];
  return $line =~ m/^[ \t]*public[ \t]+class/;
}

sub IsStaticConstant
{
  my $line;
  $line = $_[0];
  $line =~ s/=/ = /;
  $line =~ s/[ \t]+/ /g;
  return $line =~ m/^ ?public static readonly [A-Za-z_]+ [A-Za-z_][A-Za-z0-9_]* =.+;.*$/ ;
}

sub IsStaticVariable
{
  my $line;
  $line = $_[0];
  $line =~ s/=/ = /;
  $line =~ s/[ \t]+/ /g;
  return $line =~ m/^ ?public static [A-Za-z_]+ [A-Za-z_][A-Za-z0-9_]* =.+;.*$/ ;
}

sub Generate
{
  my $line;
  my $input;

  PrintVerbose "Writting header\n";
  &WriteHeader;

  foreach $input (@InputFiles)
  {
    PrintVerbose "Parsing $input\n";
    open(INPUTFILE, "<${input}");
    while($line = <INPUTFILE>)
    {
      chop $line;
      if(IsClass $line)
      {
	$CurrentClass = $line;
	$CurrentClass =~ s/[ \t]+/ /g;
	$CurrentClass =~ s/^ ?(public )?class //;
	$CurrentClass =~ s/ ?(:[^{]+)?{?$//;
	PrintVerbose "Parsing class \"$CurrentClass\"\n";
	&WriteClassComment;
      }
      elsif(IsStaticFunction $line)
      {
	OutputFunction $line;
      }
      elsif(IsStaticConstant $line)
      {
	OutputStaticConstant $line;
      }
      elsif(IsStaticVariable $line)
      {
	OutputStaticVariable $line;
      }
      elsif(IsConstant $line)
      {
	OutputConstant $line;
      }
    }
    close(INPUTFILE);
  }

  PrintVerbose "Writting foot\n";
  &WriteFoot;
  PrintVerbose "Generation completed\n";
  close(OUTPUTFILE);
}

sub ValidateCommandLine
{
  if($#InputFiles == -1)
  {
    &PrintHelp;
    die "Input files must be specified!\n";
  }

  if($OutputFile)
  {
    open(OUTPUTFILE, ">${OutputFile}");
  }
  else
  {
    die "An output file must be specified!\n";
  }
}

&ParseCommandLine;
&ValidateCommandLine;
&Generate;

