<?php
//=============================================================================
//
//    Virtual-Directory Listing & Pretty-Printing Script
//    Copyright (C) 2000,2002 by Eric Sunshine <sunshine@sunshineco.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//=============================================================================
//-----------------------------------------------------------------------------
// spoofdir.php
//
// This script allows HTTP access to directories and files which are contained
// both within and outside of the normal Apache 'htdocs' directory tree.  It
// does so by treating such URIs as virtual paths and resolves them into
// actual physical paths at run-time.  A common reason for allowing access to
// directories and files which reside outside of the 'htdocs' tree is to allow
// HTTP access to an FTP archive hierarchy.
//
// In addition to the obvious benefit of being able to browse locations
// outside of 'htdocs', this script also pretty-prints directory listings and
// can optionally annotate them with descriptive information contained within
// a configuration file residing in the directory being browsed (in much the
// same manner as some FTP servers automatically send the contents of a
// README file along with a directory listing).
// 
// Many aspects of the script's operation can be controlled by both a global
// configuration file as well as configuration files in each of the browsed
// directories.  These options are described below.
//
// Typical configuration involves use of Apache's URI rewriting mechanism in
// order to invoke this script whenever a file is requested within the
// pseudo-directory, or when a listing of the directory itself is requested.
//
// This script should be accompanied by an .htaccess file in the same
// directory.  Here is an example .htaccess file which defines two virtual
// directories; 'docs' and 'download':
//
//    Options FollowSymLinks
//    RewriteEngine on
//    RewriteRule "^(docs|download)(/)?$"  "/spoofdir.php?/$1$2"  [L]
//    RewriteRule "^(docs|download)/(.+)$" "/spoofdir.php?/$1/$2" [L]
//
// In this example, the first rule catches requests for the directory itself
// (either 'docs' or 'download'), with or without the trailing '/'.  The
// second rule catches requests for any file within either of the virtual
// directories and passes the requests on to this script.
//
// When the script is invoked, it takes the input virtual path argument and
// tries to locate a matching physical path by searching through a list of
// potential target directories.  The search list (see $dirlist) can be
// configured by the global configuration file (see $globalconfig).  If the
// configuration file fails to initialize the search list, then all searching
// is performed relative to the "current" directory, which is denoted by ".".
//
// One of the following actions is taken depending upon the outcome of the
// search:
//
//    - If a corresponding physical path does not exist, then a standard HTTP
//      "404 Not Found" exception is raised.
//
//    - If the path is a file, then it is returned to the client's browser.
//      If the file can be viewed directly by the browser (see $disposition)
//      then it is sent "inline"; otherwise it is sent as an "attachment".
//
//    - If the path is a directory but is not in canonical format (that is,
//      if it is missing the final '/'), then the browser is redirected to
//      the same URI with the final '/' added.  This ensures that the browser
//      will treat relative URIs correctly later on.
//
//    - If the path is a directory in canonical format and the directory
//      contains an index-like file (such as "index.html"), then that file is
//      returned to the client browser.  This emulates the standard behavior
//      of most HTTP servers where they prefer returning an index-like file,
//      if present, rather than a directory listing.  The candidate list of
//      index-like files can be configured (see $indexfile).
//
//    - If the path is a directory in canonical format and the directory does
//      not contain an index-like file, then the contents of the directory
//      are nicely formatted and returned to the client browser.  Several
//      customizations to the directory listing are possible, including
//      colors, omission of certain files (see $ignore), title (see $title),
//      and descriptive information (see $annotation).  These options may be
//      specified in the global configuration file (see $globalconfig) or the
//      configuration file local to the directory itself (see $localconfig).
//
// An explanation of the various configuration options can be found in the
// "Configuration Options" section below.
//
// spoofdir.php was written by Eric Sunshine <sunshine@sunshineco.com> on
// September 3, 2000, and is Copyright (C)2000,2002 by Eric Sunshine.
//-----------------------------------------------------------------------------

$prog_name = 'spoofdir.php';
$prog_version = '4';
$author_name = 'Eric Sunshine';
$author_email = 'sunshine@sunshineco.com';

//-----------------------------------------------------------------------------
// Configuration Options
//
// The default values for the following options can be overriden via the
// global configuration file (see $globalconfig) or the configuration file
// local to the directory being browsed (see $localconfig), unless otherwise
// noted.
//
// $dirlist
//    An array of physical paths which will be searched for the given virtual
//    path.  If $dirlist is not initialized by the global conifguration file
//    (see $globalconfig), then the search list will default to the "current"
//    directory (also known as ".").  Setting this value only makes sense in
//    the global configuration file.
//
// $ignore
//    An associative array where the keys are regular-expresions which
//    constitute the set of files to omit from the generated directory
//    listing, and the values are boolean flags which control whether or not
//    the match is case-sensitive; 1 (true) specifies case-sensitive matching,
//    and 0 (false) specifies case-insensitive matching.  By default, the
//    $ignore array is seeded with patterns matching the names of this script
//    and its configuration files, thus the directory listing will
//    automatically be purged of these administrative entries.
//
// $title
//    The human-readable title for the directory.  The title is displayed in
//    the <head> and <body> portions of the document.  It should not contain
//    any HTML mark-up.  It generally only makes sense to specify this value
//    in the local configuration file (see $localconfig).
//
// $annotation
//    Descriptive text used to annotate the directory contents.  This
//    information is presented after the opening banner but before the actual
//    directory listing.  It may contain HTML mark-up.  It generally only
//    makes sense to specify this value in the local configuration file.
//
// $globalconfig
//    The name of the global configuration file.  If this file exists, then it
//    must be a valid PHP file and should contain statements which override or
//    initialize various options.  Remember to surround PHP expressions within
//    an appropriate '<?php...>' block within this file.  It is not possible
//    to override the value of $globalconfig without actually modifying this
//    script.
//
// $localconfig
//    The name of the local configuration file.  If this file exists within
//    the directory being browsed, then it must be a valid PHP file and should
//    contain statements which override various options.  Remember to surround
//    PHP expressions within an appropriate '<?php...>' block within this
//    file.  Overriding the default value of $localconfig only makes sense in
//    the global configuration file.
//
// $banner_bgcolor, $banner_fgcolor, $banner_linkcolor
//    Specify the various colors of the opening and closing banners.
//
// $row_colors
//    An array of two colors.  The rows of the directory listing alternate
//    between these two colors.
//
// $mimetype, $default_mimetype
//    When sending files to the client's browser, the file's MIME-type (also
//    known as content-type) is derived from the file's extension.  $mimetype
//    is an associative array where the keys are file extensions and the
//    values are the MIME-types.  The file extensions are canonicalized by
//    conversion to lower-case and removal the leading period (if present)
//    before being used as keys.  $default_mimetype will be used if the file's
//    extension does not appear in $mimetype or if the file has no extension.
//
// $disposition, $default_disposition
//    When sending files to the client's browser, the file's MIME-type is used
//    to determine its disposition.  The two most common dispositions are
//    "inline" for when the browser can (probably) display the file directly,
//    and "attachment" for when the browser should save the file to disk or
//    send it to an external program.  $disposition is an associative array
//    where the keys are MIME-types and the values are disposition identifiers.
//    $default_disposition is used if the MIME-type does not appear in
//    $disposition.
//
// $cacheable
//    When sending files to the client's browser, the file's MIME-type is used
//    to determine if the browser should be able to cache the file.  Typically
//    files with disposition type "inline" are cacheable, whereas other types
//    are not.  $cacheable is an associative array where the keys are
//    MIME-types and the values are boolean values; 1 for true, or 0 for false.
//    By default, if a file's MIME-type does not appear in $cacheable, then it
//    is considered uncacheable.
//
// $indexfile
//    An associative array where the keys are regular-expressions which
//    constitute the set of potential index-like filenames, and the values are
//    boolean flags which control whether or not the match is case-sensitive;
//    1 (true) specifies case-sensitive matching, and 0 (false) specifies
//    case-insensitive matching.  If a filename in the directory being browsed
//    matches one of these patterns then that file will be sent to the
//    client's browser instead of a directory listing.  This emulates the
//    behavior of most HTTP servers where the presence of an index-like file
//    (such as 'index.html') within a directory results in the transmission of
//    that file rather than a listing of the directory.
//-----------------------------------------------------------------------------

$globalconfig = 'spoofdir.cfg';
$localconfig = 'spoofdir.info';

$banner_bgcolor = '#5544ff';
$banner_fgcolor = '#ffff00';
$banner_linkcolor = '#ffffff';
$row_colors = array('#ccccee', '#ffffff');

$default_mimetype = 'application/octet-stream';
$mimetype['htm'  ] = 'text/html';
$mimetype['html' ] = 'text/html';
$mimetype['html3'] = 'text/html';
$mimetype['ht3'  ] = 'text/html';
$mimetype['txt'  ] = 'text/plain';
$mimetype['text' ] = 'text/plain';

$default_disposition = 'attachment';
$disposition['text/html' ] = 'inline';
$disposition['text/plain'] = 'inline';

$cacheable['text/html' ] = 1;
$cacheable['text/plain'] = 1;

$ignore['^index\..+$'     ] = 1;
$ignore["^$prog_name\$"   ] = 1;
$ignore["^$globalconfig\$"] = 1;
$ignore["^$localconfig\$" ] = 1;

$indexfile['^index.s?html?$'  ] = 1; // index.htm, index.html, index.shtml
$indexfile['^index.php[1-9]?$'] = 1; // index.php, index.php3, index.php4

//-----------------------------------------------------------------------------
// Private Configuration
//-----------------------------------------------------------------------------
if (file_exists($globalconfig))
    include($globalconfig);
if (count($dirlist) == 0)
    $dirlist[] = '.';

$copyright = "Copyright &copy;2000,2002 by $author_name " .
    "&lt;<a href=\"mailto:$author_email\">" .
    "<font color=\"$banner_linkcolor\">$author_email</font></a>&gt;";

//-----------------------------------------------------------------------------
// Utility Functions
//-----------------------------------------------------------------------------
function solidify($s)
{
    return strtr($s, array(' ' => '&nbsp;'));
}

function pretty_date($stamp)
{
    return gmdate('d M Y H:i:s', $stamp) . ' UTC';
}

function pretty_size($bytes)
{
    $KB = 1024;
    $MB = $KB * 1024;
    $GB = $MB * 1024;
    if ($bytes > $GB)
	$s = sprintf('%.1f GB', (float)$bytes / $GB);
    elseif ($bytes > $MB)
	$s = sprintf('%.1f MB', (float)$bytes / $MB);
    elseif ($bytes > $KB)
	$s = sprintf('%.1f KB', (float)$bytes / $KB); 
    else
	$s = "$bytes";
    return $s;
}

function sort_array(&$a)
{
    if (count($a) > 1)
	usort($a, strcasecmp);
}

function array_pattern_match($target, &$patterns)
{
    $n = count($patterns);
    reset($patterns);
    for ($i = 0; $i < $n; $i++, next($patterns))
    {
	$matched = 0;
	$pattern = key($patterns);
	$case_sensitive = current($patterns);
	if ($case_sensitive)
	    $matched = ereg ($pattern, $target);
	else
	    $matched = eregi($pattern, $target);
	if ($matched)
	    return 1;
    }
    return 0;
}

function find_mimetype($file)
{
    global $mimetype, $default_mimetype;
    $type = '';
    $ext = strrchr($file, '.');
    if ($ext) // Canonicalize; strip leading '.', down-case.
	$ext = strtolower(substr($ext, 1));
    $type = $mimetype[$ext];
    if (!$type)
	$type = $default_mimetype;
    return $type;
}

function find_disposition($type)
{
    global $disposition, $default_disposition;
    $s = $disposition[$type];
    if (!$s)
	$s = $default_disposition;
    return $s;
}

function is_cacheable($type)
{
    global $cacheable;
    return $cacheable[$type];
}

function is_ignored($file)
{
    global $ignore;
    return array_pattern_match($file, $ignore);
}

function is_indexfile($file)
{
    global $indexfile;
    return array_pattern_match(basename($file), $indexfile);
}

//-----------------------------------------------------------------------------
// Emit the sign-off banner containing copyright notice, etc.
//-----------------------------------------------------------------------------
function signoff()
{
    global $prog_name, $prog_version, $copyright,
	$banner_bgcolor, $banner_fgcolor;
    print("<p><table width=\"100%\" border=0 cellspacing=0 cellpadding=2>\n" .
	"<tr bgcolor=\"$banner_bgcolor\">\n" .
	"<td><font color=\"$banner_fgcolor\">&nbsp;&nbsp;" .
	'Generated ' . pretty_date(time()) . " by <tt>$prog_name</tt> " .
	"version $prog_version.<br>&nbsp;&nbsp;$copyright</font></td>" .
	"</tr></table>\n");
}

//-----------------------------------------------------------------------------
// Enumerate over the search list ($dirlist) to find a corresponding physical
// path for the given virtual path.
//-----------------------------------------------------------------------------
function find_path($find)
{
    global $dirlist;
    if (strlen($find) > 0 && $find[0] == '/')
	$find = substr($find, 1);
    for ($s = reset($dirlist); $s; $s = next($dirlist))
    {
	$n = strlen($s);
	if ($n > 0 && $s[n - 1] != '/')
	    $s .= '/';
	$s .= $find;
	if (file_exists($s))
	    return $s;
    }
    return '';
}

//-----------------------------------------------------------------------------
// Re-direct client's browser to given URI.  Note that PHP treats "Location:"
// header specially and does all the work for us.
//-----------------------------------------------------------------------------
function redirect($uri)
{
    global $HTTP_HOST;
    header("Location: http://$HTTP_HOST$uri");
    exit();
}

//-----------------------------------------------------------------------------
// Send a file to the client's browser using appropriate content-type,
// disposition, etc.
//
// NOTE: Unfortunately, PHP files must be handled specially.  A simple
// re-direct will not execute the file.  Instead, the PHP file is loaded into
// the currently running script via include().  This is a poor solution since
// the script might cause name clashes, among other problems.  A better
// solution is needed.
//-----------------------------------------------------------------------------
function send_file($path)
{
    if (!eregi('\.php[0-9]?$', $path))
    {
	$name = basename($path);
	$type = find_mimetype($name);
        $disposition = find_disposition($type);
        header("Content-Disposition: $disposition; filename=$name");
        header("Content-Type: $type; file=$name");
        header('Content-Length: ' . filesize($path));
        if (!is_cacheable($type))
        {
	    header('Pragma: no-cache');
	    header('Expires: 0');
        }
        readfile($path);
    }
    else
    {
	chdir(dirname($path));
	include(basename($path));
    }
    exit();
}

//-----------------------------------------------------------------------------
// Emit standard HTML boilerplate to begin the page.
//-----------------------------------------------------------------------------
function open_doc($title)
{
    print("<html>\n<head>\n<title>$title</title>\n</head>\n<body>\n");
}

//-----------------------------------------------------------------------------
// Emit standard HTML boilerplate to end the page.
//-----------------------------------------------------------------------------
function close_doc()
{
    signoff();
    print("</body>\n</html>\n");
    exit();
}

//-----------------------------------------------------------------------------
// Chop a pathname into directory-sized components and provide an HREF to
// each.  Given the input "/one/two/three/four", the output HTML will be
// "/<one>/<two>/<three>/<four>" where '<' and '>' signify the presence of an
// HREF link.  (The '<' and '>' are for illustration purposes only and do not
// appear in the output string.)
//-----------------------------------------------------------------------------
function parent_link($path, $link = '../')
{
    global $banner_linkcolor;
    $s = '/';
    if ($path && $path != '/')
    {
	$s = parent_link(dirname($path), "../$link");
	$f = basename($path);
	$s .= "<a href=\"$link\"><font color=\"$banner_linkcolor\">" .
	    "$f</font></a>/";
    }
    return $s;    
}

//-----------------------------------------------------------------------------
// Emit HTML to announce a directory.  Elements include the actual pathname
// with embedded links pointing at parent directories, an optional title,
// and optional annotation.  $title and $annotation can be set in the
// $localconfig file.
//-----------------------------------------------------------------------------
function announce_directory($path, $title, $annotation)
{
    global $banner_bgcolor, $banner_fgcolor;
    if ($path && $path[strlen($path) - 1] == '/')
	$path = substr($path, 0, -1);

    print("<p><table width=\"100%\" border=0 cellspacing=0 cellpadding=2>\n" .
	"<tr bgcolor=\"$banner_bgcolor\">\n" .
	"<td><font color=\"$banner_fgcolor\">&nbsp;&nbsp;" .
	'<strong>Directory</strong>: <code>' .
	parent_link(dirname($path)) . basename($path) .
	'</code></font></td>');
    if ($title)
	print("<td align=\"right\"><font color=\"$banner_fgcolor\">\n" .
	    "<em>$title</em>&nbsp;&nbsp;</td>\n");
    print("</tr></table><p>\n");

    if ($annotation)
      print("$annotation<p>\n");
}

//-----------------------------------------------------------------------------
// Emit a single directory entry.
//-----------------------------------------------------------------------------
function print_entry($link, $info1, $info2)
{
    global $row_colors;
    static $color = 0;
    print("<tr bgcolor=\"$row_colors[$color]\">\n" .
	"<td><a href=\"$link\">$link</a></td>\n" .
	"<td align=\"right\">$info1</td>\n" .
	"<td>$info2</td>\n</tr>\n");
    $color = !$color;
}

//-----------------------------------------------------------------------------
// Emit a directory listing.  If a $localconfig file exists, then invoke it.
// It may override any of the options mentioned in the 'global' statement
// below.  If the directory contains an index-like file, then send that file
// back to the client browser rather than a directory listing.
//-----------------------------------------------------------------------------
function list_directory($display_name, $path)
{
    global $localconfig;
    // Allow local configuration file to override any of these globals.
    global $mimetype, $disposition, $cacheable, $ignore, $indexfile;
    // And these locals.
    $title = '';
    $annotation = '';
    if (file_exists("$path/$localconfig"))
	include("$path/$localconfig");

    $handle = @opendir($path) or die("Directory $display_name not found.");
    $files = array();
    $subdirs = array();
    while ($entry = readdir($handle))
    {
	if (is_indexfile($entry))
	{
	    closedir($handle);
	    send_file("$path/$entry"); // Never returns
	}
	elseif ($entry != '..' && $entry != '.' && !is_ignored($entry))
	{
	    if (is_dir("$path/$entry"))
		$subdirs[] = $entry;
	    else
		$files[] = $entry;
	}
    }
    closedir($handle);
    sort_array($files);
    sort_array($subdirs);

    open_doc($title);
    announce_directory($display_name, $title, $annotation);
    print("<table width=\"100%\" border=0 cellspacing=1 cellpadding=2>\n");

    for ($s = reset($subdirs); $s; $s = next($subdirs))
	print_entry("$s/", '(dir)', '&nbsp');

    for ($s = reset($files); $s; $s = next($files))
	print_entry($s, solidify(pretty_size(filesize("$path/$s"))),
	    solidify(pretty_date(filemtime("$path/$s"))));

    print("</table>\n");
    close_doc();
}

//-----------------------------------------------------------------------------
// Raise a standard HTTP "404 Not Found" exception.
//-----------------------------------------------------------------------------
function path_not_found($uri)
{
    header("http/1.0 404 Not Found");
    open_doc("404 Not Found");
    print("<h1>Not Found</h1>The requested URL ($uri) was not found on " .
	'this server.');
    close_doc();
}

//-----------------------------------------------------------------------------
// Dispatch the given URI.
//-----------------------------------------------------------------------------
function dispatch_uri($uri)
{
    $path = find_path($uri);
    if (strlen($path) == 0)
	path_not_found($uri);
    elseif (!is_dir($path))
	send_file($path);
    elseif ($path[strlen($path) - 1] != '/')
	redirect("$uri/"); // Enforce correct directory syntax (append '/').
    else
	list_directory($uri, $path);
}

//-----------------------------------------------------------------------------
// Main: Dispatch the input URI.
//-----------------------------------------------------------------------------
$uri = $argv[0];
if (strlen($uri) == 0)
    $uri = $REQUEST_URI;
dispatch_uri($uri);
?>
