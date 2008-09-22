#!/usr/bin/perl -w
#==============================================================================
#
#    Copyright (C) 2002 by Eric Sunshine <sunshine@sunshineco.com>
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
#------------------------------------------------------------------------------
# cfg2xml.pl
#
#    A tool for converting simple property list configuration files into XML
#    documents.
#
#------------------------------------------------------------------------------
use strict;
use File::Basename;
use Getopt::Long;
$Getopt::Long::ignorecase = 1;

my $PROG_NAME = 'cfg2xml.pl';
my $PROG_VERSION = 1;
my $AUTHOR_NAME = 'Eric Sunshine';
my $AUTHOR_EMAIL = 'sunshine@sunshineco.com';
my $COPYRIGHT = "Copyright (C) 2002 by $AUTHOR_NAME <$AUTHOR_EMAIL>";

$main::opt_root_name = "config";
$main::opt_preserve_blank_lines = undef;
$main::opt_version = undef;
$main::opt_help = undef;

my @script_options = (
    'root-name=s',
    'preserve-blank-lines!',
    'version',
    'help'
);

my %MAP = ();
my @COMMENT = ();
my $LINE = 0;
my $INDENT = '  ';
my $KEYCHARS = 'A-Za-z0-9/*~.:_-';
my $KEYRX = "[$KEYCHARS]+ ?[$KEYCHARS]*";
my $NAMERX = '[A-Za-z_][A-Za-z0-9._-]*';
my $FILENAME = undef;
my $BOILERPLATE = "<?xml version=\"1.0\"?>";

#------------------------------------------------------------------------------
# Is this a blank line?
# Pre-condition: Leading and trailing whitespace has been stripped.
#------------------------------------------------------------------------------
sub is_white {
    my $line = shift;
    return $line =~ /^$/;
}

#------------------------------------------------------------------------------
# Is this a comment line?
# Pre-condition: Leading and trailing whitespace has been stripped.
#------------------------------------------------------------------------------
sub is_comment {
    my $line = shift;
    return $line =~ /^;/;
}

#------------------------------------------------------------------------------
# Is this a property relation line?
# Pre-condition: Leading and trailing whitespace has been stripped.
#------------------------------------------------------------------------------
sub is_relation {
    my $line = shift;
    return $line =~ /^;*\s*$KEYRX\s*=/;
}

#------------------------------------------------------------------------------
# Is this a valid XML node name?
#------------------------------------------------------------------------------
sub is_valid_name {
    my $name = shift;
    return $name =~ /^$NAMERX$/;
}

#------------------------------------------------------------------------------
# Extract the comment text from a comment line.
# Pre-condition: Leading and trailing whitespace has been stripped.
#------------------------------------------------------------------------------
sub parse_comment {
    my $line = shift;
    my ($comment) = $line =~ /^;+\s*(.*)$/;
    return $comment;
}

#------------------------------------------------------------------------------
# Extract the key, value, and "disabled" flag from a property relation line.
# Pre-condition: Leading and trailing whitespace has been stripped.
#------------------------------------------------------------------------------
sub parse_relation {
    my $line = shift;
    my ($disabled, $key, $value) = $line =~ /^(;*)\s*($KEYRX)\s*=\s*(.+)$/;
    $key =~ s/^\s+//;
    $key =~ s/\s+$//;
    return ($key, $value, $disabled);
}

#------------------------------------------------------------------------------
# Parse a key ("key.subkey.subsubkey") into components.
#------------------------------------------------------------------------------
sub parse_key {
    my $key = shift;
    my @parts = split(/\./, $key);
    return @parts;
}

#------------------------------------------------------------------------------
# Process a blank line from the input file by inserting it into the comment
# block for the upcoming node.  It is specially marked as "undef".
#------------------------------------------------------------------------------
sub do_white {
    my $line = shift;
    if (is_white($line)) {
	push(@COMMENT,undef);
	return 1;
    }
    return undef;
}

#------------------------------------------------------------------------------
# Process a comment line from the input file by inserting it into the comment
# block for the upcoming node.
#------------------------------------------------------------------------------
sub do_comment {
    my $line = shift;
    if (is_comment($line)) {
	push(@COMMENT,parse_comment($line));
	return 1;
    }
    return undef;
}

#------------------------------------------------------------------------------
# Process a property relation line from the input file by inserting the
# property into a hierarchical node map.  Node names are normalized to
# lower-case for graph navigation, however the original case is preserved when
# the node is emitted.  A node's weight is used to maintain order of properties
# from input to output files.  A node may contain child nodes as well as
# multiple values (all but one of which must be disabled).  If comments and
# blanks lines preceded this line, then attach them to the property.
#------------------------------------------------------------------------------
sub do_relation {
    my $line = shift;
    if (is_relation($line)) {
	my ($key, $value, $disabled) = parse_relation($line);
	$key = "$main::opt_root_name.$key";
    
	my $map = \%MAP;
	foreach my $k (parse_key($key)) {
	    print STDERR "WARNING: Invalid XML entity name \"$k\" " .
		"($FILENAME:$LINE)\n" unless is_valid_name($k);

	    my $klc = lc($k);
	    my $children = $map->{'children'};
	    $children->{$klc} = {
		'name' => $k,
		'weight' => $LINE,
		'comment' => [],
		'children' => {},
		'value' => []
		} if !exists $children->{$klc};
	    $map = $children->{$klc};
	}
    
	insert_value($map, $value, $disabled);
	push(@{$map->{'comment'}}, @COMMENT);
	@COMMENT = ();
	return 1;
    }
    return undef;
}

#------------------------------------------------------------------------------
# Insert a value into the value list for a given node.  A node may contain
# multiple values, but only one may be active.
#------------------------------------------------------------------------------
sub insert_value {
    my ($map, $value, $disabled) = @_;
    my $valary = $map->{'value'};

    if (!$disabled) {
	foreach my $v (@{$valary}) {
	    if ($v->{'enabled'}) {
		print STDERR "WARNING: Duplicate key ($FILENAME:$LINE)\n";
		$disabled = 1;
		last;
	    }
	}
    }

    push(@{$valary}, { 'text' => $value, 'enabled' => !$disabled });
}

#------------------------------------------------------------------------------
# Process each line from the input file.
#------------------------------------------------------------------------------
sub slurp_input {
    my $file = shift;
    while (my $line = <$file>) {
	$LINE++;
	chomp $line;
	$line =~ s/^\s*(.*)\s*$/$1/;
	do_relation($line) or do_comment($line) or do_white($line) or
	    print STDERR "WARNING: Malformed line ($FILENAME:$LINE): $line\n";
    }
}

#------------------------------------------------------------------------------
# Return a list of children of this node sorted by weight.  A node's weight
# represents its relative position in the input file and is used to preserve
# input file ordering in the output file.
#------------------------------------------------------------------------------
sub sorted_keys {
    my $m = shift;
    return sort { $m->{$a}->{'weight'} <=> $m->{$b}->{'weight'} } keys %{$m};
}

#------------------------------------------------------------------------------
# Protect unsafe character sequences in XML comment text.
#------------------------------------------------------------------------------
sub protect_comment {
    my $comment = shift;
    $comment =~ s/--/-=/g; # XML disallows repeated hyphen in comment.
    return $comment;
}

#------------------------------------------------------------------------------
# Protect unsafe characters in XML node text.
#------------------------------------------------------------------------------
sub protect_text {
    my $text = shift;
    $text =~ s/&/&amp;/g;
    $text =~ s/</&lt;/g;
    $text =~ s/>/&gt;/g;
    return $text;
}

#------------------------------------------------------------------------------
# Emit standard XML document boilerplate.
#------------------------------------------------------------------------------
sub emit_boilerplate {
    my $file = shift;
    print $file "$BOILERPLATE\n";
}

#------------------------------------------------------------------------------
# Emit a blank line into the output file.
#------------------------------------------------------------------------------
sub emit_white {
    my $file = shift;
    print $file "\n" if $main::opt_preserve_blank_lines;
}

#------------------------------------------------------------------------------
# Emit a comment block into the output file.  If the comment block contains
# only one line, then emit a one-line XML comment, otherwise emit an XML
# comment block.
#------------------------------------------------------------------------------
sub emit_comment_block {
    my ($file, $block, $depth) = @_;
    my $n = scalar(@{$block});
    if ($n > 0) {
	my $indent = $INDENT x $depth;
	if ($n == 1) {
	    print $file "$indent<!-- " . protect_comment(@{$block}[0]) .
		" -->\n";
	}
	else {
	    my $indent2 = $indent . $INDENT;
	    print $file "$indent<!--\n";
	    foreach my $line (@{$block}) {
		print $file "$indent2" . protect_comment($line) . "\n";
	    }
	    print $file "$indent-->\n";
	}
    }
}

#------------------------------------------------------------------------------
# Emit comments and blank links into output file.  Collect consecutive comment
# lines into a block so that they can be emitted as a single block rather than
# individual comment lines.
#------------------------------------------------------------------------------
sub emit_comments {
    my ($file, $comments, $depth) = @_;
    my @comment_block = ();
    foreach my $line (@{$comments}) {
	if (defined($line)) {
	    push(@comment_block, $line);
	}
	else {
	    emit_comment_block($file, \@comment_block, $depth);
	    @comment_block = ();
	    emit_white($file);
	}
    }
    emit_comment_block($file, \@comment_block, $depth);
}

#------------------------------------------------------------------------------
# Emit a value into the output file.  If the value is disabled, then it is
# placed inside an XML comment block.  A node may contain multiple values, but
# only a single enabled value.
#------------------------------------------------------------------------------
sub emit_value {
    my ($file, $value, $depth) = @_;
    my $text = protect_text($value->{'text'});
    my $enabled = $value->{'enabled'};
    my $comment = $value->{'comment'};
    my $indent = $INDENT x $depth;

    print $file "$indent$text\n" if $enabled;
    print $file "$indent<!-- " . protect_comment($text) . " -->\n"
	unless $enabled;
}

#------------------------------------------------------------------------------
# Emit a node's values into the output file.
#------------------------------------------------------------------------------
sub emit_values {
    my ($file, $node, $depth) = @_;
    my $valary = $node->{'value'};
    foreach my $v (@{$valary}) {
	emit_value($file, $v, $depth);
    }
}

#------------------------------------------------------------------------------
# Emit a node into the output file.  The node's comments and blank lines are
# emitted, followed by its values, and then its own children.
#------------------------------------------------------------------------------
sub emit_child {
    my ($file, $node, $depth) = @_;
    my $name = $node->{'name'};
    my $comment = $node->{'comment'};
    my $indent = $INDENT x $depth;

    emit_comments($file, $comment, $depth);
    print $file "$indent<$name>\n";
    emit_values($file, $node, $depth + 1);
    emit_children($file, $node, $depth + 1);
    print $file "$indent</$name>\n";
}

#------------------------------------------------------------------------------
# Emit the children of the given node into the output file.
#------------------------------------------------------------------------------
sub emit_children {
    my ($file, $map, $depth) = @_;
    my $children = $map->{'children'};
    my @keys = sorted_keys($children);
    foreach my $k (@keys) {
	emit_child($file, $children->{$k}, $depth);
    }
}

#------------------------------------------------------------------------------
# Prepare for processing another input file.
#------------------------------------------------------------------------------
sub clear_state {
    %MAP = ('children' => {});
    @COMMENT = ();
    $LINE = 0;
}

#------------------------------------------------------------------------------
# Convert an input property list file to an XML document.
#------------------------------------------------------------------------------
sub convert_file {
    my ($infile, $outfile) = @_;
    print STDOUT "Converting $FILENAME\n";
    clear_state();
    slurp_input($infile);
    emit_boilerplate($outfile);
    emit_children($outfile, \%MAP, 0);
    emit_comments($outfile, \@COMMENT, 0); # Trailing file comments.
}

#------------------------------------------------------------------------------
# Convert each property list files mentioned on the command line to an XML
# document.
#------------------------------------------------------------------------------
sub convert_files {
    foreach my $path (@ARGV) {
	my ($file, $dir, $ext) = fileparse($path, '\..*?');
	my $src = $dir . $file . (($ext ne '.bak') ? '.bak' : '.old');
	my $dst = $dir . $file . $ext;
	-r $dst or print(STDERR "$dst does not exist\n"), next;
	rename($dst, $src) or print(STDERR "rename($dst,$src) failed\n"), next;
	open INFILE, "<$src" or print(STDERR "open(<$src) failed\n"), next;
	open OUTFILE, ">$dst" or print(STDERR "open(>$dst) failed\n"),
	    close(INFILE), next;
	$FILENAME = $dst;
	convert_file(\*INFILE, \*OUTFILE);
	close(OUTFILE);
	close(INFILE);
    }
}

#------------------------------------------------------------------------------
# Display an opening banner.
#------------------------------------------------------------------------------
sub banner {
    my $stream = shift;
    $stream = \*STDOUT unless $stream;
    print $stream "\n$PROG_NAME version $PROG_VERSION\n$COPYRIGHT\n\n";
}

#------------------------------------------------------------------------------
# Display usage statement.
#------------------------------------------------------------------------------
sub print_usage {
    my $stream = shift;
    $stream = \*STDOUT unless $stream;
    banner($stream);
    print $stream <<"EOT";
Converts simple property-list based configuration files to XML format.  Input
files consist of blank lines, comment lines beginning with a semicolon (";"),
and property relation lines of the form "key.subkey.subsubkey = value".

Although property keys may contain most punctuation characters including
spaces, XML node names must be composed only of alphanumeric characters,
periods, underscores, and hyphens; and must begin with an alphanumeric
character or an underscore.  A warning message is emitted if an XML node name
derived from a property key contains or begins with invalid characters.  Node
names are case-insensitive, thus the input keys "Mouse.Buttons" and
"mouse.buttons" map to the same XML node names; and <Mouse> and <mouse> are
considered identical.

Property-list based configuration files may contain more than one top-level key
(the "key" part of "key.subkey.subsubkey"), however an XML document may contain
only a single root node.  For this reason, during conversion, the entire
configuration is wrapped inside a synthesized root node named "config".

An attempt is made to identify disabled properties which appear after
semicolons.  Special care is taken to preserve these properties by emitting
them as disabled XML nodes rather than as comments, however, since disabled
properties look a lot like comments, the heuristic which makes this
determination may be fooled on occasion, in which case manual editing of the
generated file is required.

Comments are preserved and translated to XML notation.  In XML, it is illegal
for runs of two or more hyphens ("--") to appear within comments, thus such
runs are translated to "-=" in order to preserve XML correctness.  A comment is
attached to the property which directly follows it.  Orphaned comments at the
end of file which are not associated with any property are also preserved.

The order of property declarations from the input file is preserved in the
output file whenever possible, however, because XML documents are hiearchically
organized, some re-ordering of properties may occur if properties with similar
keys are not listed continguously in the input file.

To illustrate points from the foregoing discussion, given the following input
file:

    ;;; My configuration file --- begin.
    ; Number of mouse buttons.
    Mouse.Buttons = 2
    ; Enable debugging?
    System.Debug = yes
    ;System.Debug = no
    ; Mouse-wheel present?
    Mouse.Wheel = yes
    ;;; End of file.

The resulting XML file will be:

    $BOILERPLATE
    <config>
      <Mouse>
        <!--
          My configuration file -=- begin.
          Number of mouse buttons.
        -->
        <Buttons>
          2
        </Buttons>
        <!-- Mouse-wheel present? -->
        <Wheel>
          yes
        </Wheel>
      </Mouse>
      <System>
        <!-- Enable debugging? -->
        <Debug>
          yes
          <!-- no -->
        </Debug>
      </System>
    </config>
    <!-- End of file. -->

Note that the opening file comment is grouped with the first property comment
in the output file.  This occurs since there is no way to correctly predict at
which hierarchical level a comment should be placed.  Consequently, the
heuristic of attaching a comment block to the property which immediately
follows it is employed since this seems to be the appropriate choice in most
cases.  However, there are some cases, as illustrated with this example, when
manual adjustment of comments in the output file may be warranted.

Usage: $PROG_NAME [options] <file> ...

<file> Path of the file to be converted.  The conversion process preserves the
       input file by renaming it and giving it the suffix ".bak" (or ".old" if
       the original suffix was already ".bak").  Any number of files may be
       specified.

Options:
    --root-name=<name>
               Name of root node.  This is the name of the node into which the
               entire configuration hierarchy is placed.  Default is "config".
    --preserve-blank-lines
               Preserve blank lines from the input file in the output file.
               Blank lines are grouped together with comments and attached to
               the following node.  This option is disabled by default since
               blank lines in the original flat input file typically do not
               translate well to the hierarchical organization within an XML
               document.  May be negated with --nopreserve-blank-lines.
               Default is --nopreserve-blank-lines.
    --version  Display the version number of $PROG_NAME.
    --help     Display this usage message.

EOT
}

#------------------------------------------------------------------------------
# Process command-line options.
#------------------------------------------------------------------------------
sub process_options {
    GetOptions(@script_options) or usage_error('');
    print_help() if $main::opt_help;
    print_version() if $main::opt_version;
    usage_error("Must specify at least one input file.") unless @ARGV;
    usage_error("Invalid XML node name: $main::opt_root_name")
	unless is_valid_name($main::opt_root_name);
    banner();
}

sub print_version {
    banner(\*STDOUT);
    exit(0);
}

sub print_help {
    print_usage(\*STDOUT);
    exit(0);
}

sub usage_error {
    my $msg = shift;
    print STDERR "\nERROR: $msg\n" if $msg;
    print_usage(\*STDERR);
    exit(1);
}

#------------------------------------------------------------------------------
# Perform the complete conversion process.
#------------------------------------------------------------------------------
process_options();
convert_files();
