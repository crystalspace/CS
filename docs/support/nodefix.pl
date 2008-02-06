#!/usr/bin/perl -w
#==============================================================================
#
#    Texinfo @node and @menu Repair Script
#    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>
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
# nodefix.pl
#
#    A tool for repairing broken and out-of-date @node directives and @menu
#    blocks.  Understands @include and @import{} directives and will correctly
#    repair an entire Texinfo tree when given a root Texinfo file which
#    directly or indirectly includes all other files within the tree.
#
#    (The @import{} directive is a special macro developed by Eric Sunshine to
#    deal gracefully with differences in pathname syntax between platforms.)
#
# To-Do List
#    * Add support for @detailmenu.
#
#------------------------------------------------------------------------------
use strict;
use Getopt::Long;
$Getopt::Long::ignorecase = 0;

my $PROG_NAME = 'nodefix.pl';
my $PROG_VERSION = '1.3';
my $AUTHOR_NAME = 'Eric Sunshine';
my $AUTHOR_EMAIL = 'sunshine@sunshineco.com';
my $COPYRIGHT = "Copyright (C) 2000 by $AUTHOR_NAME <$AUTHOR_EMAIL>";

@main::opt_include_dir = ();
$main::opt_verbose = 0;
$main::opt_debug = 0;
$main::opt_help = 0;

my @script_options = ('include-dir|I=s@', 'verbose!', 'debug!', 'help' );

my $broken_nodes = 0;
my $broken_menus = 0;
my $broken_files = 0;

$main::current_file = undef;
$main::line_number = 0;
$main::scan_depth = 0;

my $current_node = undef;
my $current_depth = 0;
my $depth_adjustment = 0;

my %node_map;
my @node_list;
my @menu_list;
my @file_list;

my %ignored_blocks = (
    'iftex'     => 1,
    'ifhtml'    => 1,
    'ifnotinfo' => 1,
    'ignore'    => 1,
    'macro'     => 1,
    'rmacro'    => 1,
);

my $depth_min = 1; # Limit for @raisesections.
my $depth_max = 4; # Limit for @lowersections.

my %depth_map = (
    'chapter'             => 1,
    'section'             => 2,
    'subsection'          => 3,
    'subsubsection'       => 4,
    'top'                 => 0,
    'unnumbered'          => 1,
    'unnumberedsec'       => 2,
    'unnumberedsubsec'    => 3,
    'unnumberedsubsubsec' => 4,
    'appendix'            => 1,
    'appendixsec'         => 2,
    'appendixsection'     => 2,
    'appendixsubsec'      => 3,
    'appendixsubsubsec'   => 4,
);

#------------------------------------------------------------------------------
# Emit an error message and terminate abnormally.
#------------------------------------------------------------------------------
sub fatal {
    my $msg = shift;
    die "ERROR: $msg\n";
}

#------------------------------------------------------------------------------
# Emit a warning message.
#------------------------------------------------------------------------------
sub warning {
    my $msg = shift;
    warn "WARNING: $msg\n";
}

#------------------------------------------------------------------------------
# Print an underlined title.
#------------------------------------------------------------------------------
sub print_title {
    my $msg = shift;
    print "$msg\n", '-' x length($msg), "\n";
}

#------------------------------------------------------------------------------
# Find a file, possibly searching a list of alternate directories specified
# on the command line.
#------------------------------------------------------------------------------
sub find_include_file {
    my $file = shift;
    return $file if -f "$file";
    my $dir;
    foreach $dir (@main::opt_include_dir) {
	return "$dir/$file" if -f "$dir/$file";
    }
    return undef;
}

#------------------------------------------------------------------------------
# For older versions of Perl which lack FileHandle, we fake up a unique file
# handle type object.
#------------------------------------------------------------------------------
*main::file_handle_template = 'FH0000';
sub new_file_handle {
    return $main::file_handle_template++;
}

#------------------------------------------------------------------------------
# Open a Texinfo file for input and construct a file record which records the
# contained nodes and menus.
#------------------------------------------------------------------------------
sub open_input_file {
    my $file = shift;
    my $path = find_include_file($file)
	or fatal "Unable to locate file: $file";
    local *handle = new_file_handle();
    open(*handle, $path) or fatal "Failed to open file: $path";
    $main::current_file = {
	'path'  => $path,
	'nodes' => [],
	'menus' => [],
    };
    push(@file_list, $main::current_file);
    $main::line_number = 0;
    return *handle;
}

#------------------------------------------------------------------------------
# Read an input line from a Texinfo file.  If the line begins with a Texinfo
# directive, parse it and strip the trailing @comment if present.
#------------------------------------------------------------------------------
sub input_line {
    my $handle = shift;
    my $line = <$handle>;
    close($handle), return () unless $line;
    my ($key, $option) = ('', '');
    if ($line =~ /^\@(\w+)\s*(.*)$/) {		# Got a @directive?
	($key, $option) = ($1, $2);		# Extract components.
	$option =~ s/\@c(?:omment)?\b.*$//;	# Strip trailing comment.
	$option =~ s/\s+$//;			# Strip trailing whitespace.
    }
    return ($line, $key, $option);
}

#------------------------------------------------------------------------------
# Canonicalize a node name.
#------------------------------------------------------------------------------
sub canonicalize {
    local $_ = $_[0];
    $_ = '' unless $_;
    s/\s+/ /g;
    s/^ //;
    s/ $//;
    $_[0] = $_;
}

#------------------------------------------------------------------------------
# Parse the contents of a @node line ("Name, Next, Previous, Up").
#------------------------------------------------------------------------------
sub parse_node {
    my $node = shift;
    my ($nname, $nnext, $nprev, $nup) = split(/,/, $node);
    canonicalize($nname);
    canonicalize($nnext);
    canonicalize($nprev);
    canonicalize($nup  );
    return ($nname, $nnext, $nprev, $nup);
}

#------------------------------------------------------------------------------
# Get a node's depth. Internally, a negative depth is used as a flag to
# indicate that a node's depth has not been changed by one of the sectioning
# commands. The first sectioning directive following a @node is allowed to
# change the node's depth; subsequent sectioning directives do not. This
# function always returns the node's real (positive) depth.
#------------------------------------------------------------------------------
sub node_depth {
    my $node = shift;
    my $depth = $node->{'depth'};
    return $depth < 0 ? -$depth - 1 : $depth;
}

#------------------------------------------------------------------------------
# Scan a @node line and construct a node record.
#------------------------------------------------------------------------------
sub scan_node {
    my $node_line = shift;
    my ($nname, $nnext, $nprev, $nup) = parse_node($node_line);
    fatal "Malformed \@node at line $main::line_number in " .
	"$main::current_file->{'path'}." unless $nname;
    $current_node = {
	'name'  => $nname,
	'old'   => { 'next' => $nnext, 'prev' => $nprev, 'up' => $nup, },
	'new'   => { 'next' => '',     'prev' => '',     'up' => '',   },
	'title' => $nname,
	'file'  => $main::current_file,
	'depth' => -$current_depth - 1,
	'menu'  => undef,
	'dirty' => 0,
    };
    $node_map{$nname} = $current_node;
    push(@node_list, $current_node);
    push(@{$main::current_file->{'nodes'}}, $current_node);
}

#------------------------------------------------------------------------------
# Add a menu item to a menu record's item list.
#------------------------------------------------------------------------------
sub add_menu_item {
    my ($item_list, $node, $title) = @_;
    $title = '' if $node eq $title;
    my $item = {
	'node'  => $node,
	'title' => $title,
    };
    push(@{$item_list}, $item);
}

#------------------------------------------------------------------------------
# Parse a menu item ("Entry:Node. Title" or "Node:: Title").
#------------------------------------------------------------------------------
sub parse_menu_item {
    my $item = shift;
    return () unless $item =~ /^\s*\*\s+(.+?):(.*?)[:,.\t\n]\s*(.*)\s*$/;
    my ($entry, $node, $title) = ($1, $2, $3);
    $node = $entry unless $node;
    return ($node, $title);
}

#------------------------------------------------------------------------------
# Scan a menu.  Build a new menu record and then iteratively scan each item
# contained within the menu.
#------------------------------------------------------------------------------
sub scan_menu {
    fatal "Orphaned \@menu outside of \@node block at line " .
	"$main::line_number in $main::current_file->{'path'}"
	unless $current_node;
    fatal "Mutiple \@menus in node \"$current_node->{'name'}\" at line " .
	"$main::line_number in $main::current_file->{'path'}"
	if $current_node->{'menu'};

    my $handle = shift;
    my $start_line = $main::line_number;
    my $menu = {
	'node'   => $current_node,
	'file'   => $main::current_file,
	'items'  => [],
	'detail' => undef,
	'dirty'  => 0,
    };
    $current_node->{'menu'} = $menu;
    push(@menu_list, $menu);
    push(@{$main::current_file->{'menus'}}, $menu);

    my ($line, $key, $option) = (undef, undef, undef);
    while (($line, $key, $option) = input_line($handle)) {
	$main::line_number++;
	return if $key eq 'end' && $option eq 'menu';
	next unless my ($node, $title) = parse_menu_item($line);
	add_menu_item($menu->{'items'}, $node, $title);
    }
    fatal "Missing \"\@end menu\" near line $start_line in " .
	"$main::current_file->{'path'}.";
}

#------------------------------------------------------------------------------
# Ignore a section of text enclosed within a specified "@foo...@end foo" block.
#------------------------------------------------------------------------------
sub scan_ignored {
    my ($key, $handle) = @_;
    my $start_line = $main::line_number;
    while (<$handle>) {
	$main::line_number++;
	next unless /\@end\s+(\w+)\b/;
	return if $key eq $1;
    }
    fatal "Missing \"\@end $key\" near line $start_line in " .
	"$main::current_file->{'path'}.";
}

#------------------------------------------------------------------------------
# Scan a sectioning command which indicate a node's depth.
#------------------------------------------------------------------------------
sub scan_depth {
    my ($key, $title) = @_;
    $current_depth = $depth_map{$key};
    $current_depth += $depth_adjustment if
	($depth_adjustment < 0 && $current_depth > $depth_min) ||
	($depth_adjustment > 0 && $current_depth < $depth_max);
    if ($current_node && $current_node->{'depth'} < 0) {
	$current_node->{'depth'} = $current_depth;
	$current_node->{'title'} = $title;
    }
}

#------------------------------------------------------------------------------
# Recursively scan included files.  Understands the standard @include directive
# as well as @import{} which is a special Texinfo macro designed by Eric
# Sunshine to deal with differences in pathname syntax between platorms.
#------------------------------------------------------------------------------
sub scan_include {
    my $line = shift;
    my $child;
    $line =~ /^\@include\s+(.+)$/ && do { $child = $1; };
    $line =~ /^\@imports*\s*\{(.+)\}/ && do { $child = $1; $child =~ tr|,|/| };
    fatal "Missing filename following \@include or \@import{} at line " .
	"$main::line_number in $main::current_file->{'path'}" unless $child;
    return scan_file($child);
}

#------------------------------------------------------------------------------
# Scan a file and build a list of nodes and menus.  Handle sectioning
# directives which indicate a node's depth (and implicitly which other nodes
# are its siblings, children, and parent).  Recursively scan included files.
#------------------------------------------------------------------------------
sub scan_file {
    local ($main::current_file, $main::line_number);
    my $file = shift;
    my $handle = open_input_file($file);
    my $path = $main::current_file->{'path'};

    local $main::scan_depth = $main::scan_depth + 1;
    print ' ' x ($main::scan_depth - 1) . "$path\n" if $main::opt_verbose;

    my ($line, $key, $option) = (undef, undef, undef);
    LINE: while (($line, $key, $option) = input_line($handle)) {
	$main::line_number++;
	next LINE unless $key;				# Got a @directive?
	next LINE if $key eq 'c' || $key eq 'comment';	# Ignore @comments.
	return undef if $key eq 'bye';			# @bye terminates.
	$depth_adjustment++, next LINE if $key eq 'raisesections';
	$depth_adjustment--, next LINE if $key eq 'lowersections';
	scan_depth($key, $option), next LINE if exists $depth_map{$key};
	scan_ignored($key, $handle), next LINE if exists $ignored_blocks{$key};
	scan_node($option), next LINE if $key eq 'node';
	scan_menu($handle), next LINE if $key eq 'menu';
	warning("Orphaned \@detailmenu at line $main::line_number in $path."),
	    next LINE if $key eq 'detailmenu';

	if ($key =~ /include|imports*/) {
	    scan_include($line) or return undef;
	    next LINE;
	}
    }
    return 1;
}

#------------------------------------------------------------------------------
# Scan the root Texinfo file and all files it includes directly or indirectly.
#------------------------------------------------------------------------------
sub scan_root_file {
    my $file = shift;
    print_title("Scanning") if $main::opt_verbose;
    scan_file($file) and warning "Missing \"\@bye\" directive.";
    print "\n" if $main::opt_verbose;
}

#------------------------------------------------------------------------------
# Update a node pointer and mark the node as dirty if necessary.
#------------------------------------------------------------------------------
sub node_set {
    my ($node, $key, $value) = @_;
    $node->{'new'}{$key} = ($value ? $value->{'name'} : '');
}

sub node_set_next { node_set(shift, 'next', shift); }
sub node_set_prev { node_set(shift, 'prev', shift); }
sub node_set_up   { node_set(shift, 'up'  , shift); }

#------------------------------------------------------------------------------
# Repair links of all @nodes lines.  Set up the links in the customary Texinfo
# fashion where "next" and "previous" point at sibling nodes at the same
# "depth" (chapter, section, subsection, etc.), rather than merely pointing at
# physical neighboring nodes.  The "up" link points at the immediate parent
# node.  The "top" node is handled specially.  Its "next" link points at its
# first child node (rather than a sibling), and its "up" link points at the
# literal name "(dir)".
#------------------------------------------------------------------------------
sub repair_nodes {
    my @prev_at_depth;
    my $last_depth = -1;
    my $node;
    foreach $node (@node_list) {
	my $depth = node_depth($node);
	my $parent_depth = ($depth > 0 ? $depth - 1 : 0);

	if ($depth < $last_depth) {
	    my $i;
	    foreach $i ($depth + 1..$last_depth) {
		$prev_at_depth[$i] = undef;
	    }
	}

	my $prev_node = $prev_at_depth[$depth];
	node_set_next($prev_node, $node) if $prev_node;

	my $up_node = $prev_at_depth[$parent_depth];
	node_set_up($node, $up_node) unless $depth == 0; # 'top' special case.

	$prev_node = $up_node unless $prev_node;
	node_set_prev($node, $prev_node);

	if ($last_depth == 0) { # 'top' special case.
	    my $top_node = $prev_at_depth[$last_depth];
	    node_set_next($top_node, $node);
	    node_set_up($top_node, { 'name' => '(dir)' });
	}

	$prev_at_depth[$depth] = $node;
	$last_depth = $depth;
    }

    foreach $node (@node_list) {
	$node->{'dirty'} = 1 if
	    $node->{'new'}{'next'} ne $node->{'old'}{'next'} ||
	    $node->{'new'}{'prev'} ne $node->{'old'}{'prev'} ||
	    $node->{'new'}{'up'  } ne $node->{'old'}{'up'  };
    }
}

#------------------------------------------------------------------------------
# Perform a deep comparision of two menu item lists to see if they differ.
#------------------------------------------------------------------------------
sub menus_differ {
    my ($old_items, $new_items) = @_;
    return 1 unless scalar(@{$old_items}) == scalar(@{$new_items});
    my $i;
    for ($i = 0; $i < @{$new_items}; $i++) {
	my $old_item = @{$old_items}[$i];
	my $new_item = @{$new_items}[$i];
	return 1 if ($old_item->{'node'} ne $new_item->{'node'} ||
	    $old_item->{'title'} ne $new_item->{'title'});
    }
    return 0;
}

#------------------------------------------------------------------------------
# Repair all menus in the entire document hierarchy.  For each node, if it
# has an existing menu, build a brand new menu from scratch and replace the
# old menu with the new one if the two menus differ in any way.
#------------------------------------------------------------------------------
sub repair_menus {
    my $i;
    for ($i = 0; $i < @node_list; $i++) {
	my $menu_node = $node_list[$i];
	my $menu = $menu_node->{'menu'};
	next unless $menu;

	my $item_depth = node_depth($menu_node) + 1; # Node depth of items.
	my $items = [];
	my $j;
	for ($j = $i + 1; $j < @node_list; $j++) {
	    my $node = $node_list[$j];
	    my $node_depth = node_depth($node);
	    next if $node_depth > $item_depth;
	    last if $node_depth < $item_depth;
	    add_menu_item($items, $node->{'name'}, $node->{'title'});
	}

	if (menus_differ($menu->{'items'}, $items)) {
	    $menu->{'items'} = $items;
	    $menu->{'dirty'} = 1;
	}
    }
}

#------------------------------------------------------------------------------
# Patch a @node by emitting the correct links if the old links are broken.
#------------------------------------------------------------------------------
sub patch_node {
    my ($lines, $node_line) = @_;
    my ($nname, $nnext, $nprev, $nup) = parse_node($node_line);
    if (exists $node_map{$nname}) {
	my $node = $node_map{$nname};
	if ($node->{'dirty'}) {
	    $node_line = "$nname, " .
		"$node->{'new'}{'next'}, " .
		"$node->{'new'}{'prev'}, " .
		"$node->{'new'}{'up'}";
	    $broken_nodes++;
	}
    }
    push(@{$lines}, "\@node $node_line\n");
}

#------------------------------------------------------------------------------
# Patch a @menu by emitting an entire new menu if the old one is broken.
# Always emits items of form "Node:: Title", or "Node::" if Title is identical
# to Node.  Never emits item of form "Entry:Node. Title" since Entry can not
# be automatically gleaned from any source, whereas Node and Title can.
#------------------------------------------------------------------------------
sub patch_menu {
    my ($lines, $menu, $handle) = @_;
    push(@{$lines}, "\@menu\n"); # Always emit whether dirty or not.
    if ($menu->{'dirty'}) {
	scan_ignored('menu', $handle);		 # Skip the old menu.
	my $item;
	foreach $item (@{$menu->{'items'}}) { # Emit the new menu.
	    my $line = "* $item->{'node'}::" .
		($item->{'title'} ? " $item->{'title'}" : '') . "\n";
	    push(@{$lines}, $line);
	}
	push(@{$lines}, "\@end menu\n");
	$broken_menus++;
    }
}

#------------------------------------------------------------------------------
# Patch a file by repairing each broken node and menu.
#------------------------------------------------------------------------------
sub patch_file {
    my $file = shift;
    my $path = $file->{'path'};
    my $menu_number = 0;
    print "$path\n" if $main::opt_verbose;
    $broken_files++;

    local *handle = new_file_handle();
    open(*handle, $path) or fatal "Failed to open file: $path";
    my $lines = [];
    my ($line, $key, $option) = (undef, undef, undef);
    while (($line, $key, $option) = input_line(*handle)) {
	patch_node($lines, $option), next if $key eq 'node';
	patch_menu($lines, $file->{'menus'}[$menu_number++], *handle), next
	    if $key eq 'menu';
	push(@{$lines}, $line);
    }
    close(*handle);

    unless ($main::opt_debug) {
	open(*handle, ">$path") or fatal "Failed to truncate file: $path";
	print {*handle} @{$lines} or fatal "Failed to write file: $path";
        close(*handle);
    }
}

#------------------------------------------------------------------------------
# Patch all files which contain broken nodes or menus.
#------------------------------------------------------------------------------
sub patch_files {
    print_title("Repairing") if $main::opt_verbose;
    my $file;
    FILE: foreach $file (@file_list) {
	my $node;
	foreach $node (@{$file->{'nodes'}}) {
	    patch_file($file), next FILE if $node->{'dirty'};
	}
	my $menu;
	foreach $menu (@{$file->{'menus'}}) {
	    patch_file($file), next FILE if $menu->{'dirty'};
	}
    }
    print "\n" if $main::opt_verbose;
}

#------------------------------------------------------------------------------
# Display repair summary.
#------------------------------------------------------------------------------
sub summary {
    print_title("Repair Summary");
    print <<EOS;
Nodes: $broken_nodes of ${\scalar(@node_list)}
Menus: $broken_menus of ${\scalar(@menu_list)}
Files: $broken_files of ${\scalar(@file_list)}
EOS
}

#------------------------------------------------------------------------------
# Display usage statement.
#------------------------------------------------------------------------------
sub print_usage {
    my $stream = shift;
    $stream = \*STDOUT unless $stream;
    print $stream <<EOT;
$PROG_NAME version $PROG_VERSION
$COPYRIGHT

Repairs out-of-date and broken \@node directives and \@menu blocks.
Understands \@include and \@import{} directives and will correctly repair an
entire Texinfo tree when given a root Texinfo file which directly or
indirectly includes all other files within the tree.

Usage: $PROG_NAME [options] <file>

<file> is the root file of the entire Texinfo hierarchy.  This file is
       repaired along with all files which it directly or indirectly includes.

Options:
    -I <dir>
    --include-dir=<dir>
                 Search directory <dir> when looking for files included by
                 \@include or \@import{}.  May be used multiple times to
                 specify additional search directories.
    -v --verbose Emit informational messages as processing proceeds.
    -d --debug   Perform all repair work but do not actually modify any files.
    -h --help    Print this usage message.
EOT
}

#------------------------------------------------------------------------------
# Process command-line options.
#------------------------------------------------------------------------------
sub process_options {
    GetOptions(@script_options) or usage_error('');
    print_help() if $main::opt_help;
    usage_error("Must specify exactly one input file.\n") unless @ARGV == 1;
    print "$PROG_NAME version $PROG_VERSION\n$COPYRIGHT\n\n";
    print "Debugging enabled.\n\n" if $main::opt_debug;
}

sub print_help {
    print_usage(\*STDOUT);
    exit(0);
}

sub usage_error {
    my $msg = shift;
    print STDERR "$msg\n";
    print_usage(\*STDERR);
    exit(1);
}

#------------------------------------------------------------------------------
# Perform the complete repair process.
#------------------------------------------------------------------------------
process_options();
scan_root_file(shift @ARGV);
repair_nodes();
repair_menus();
patch_files();
summary();
