<?php
//-----------------------------------------------------------------------------
// Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>
//
// This script allows HTTP access to Crystal Space's CVS snapshots directory
// (which is normally only accessible via FTP).
//
// Typical configuration simulates a snapshot directory by making use of
// Apache's URI rewriting mechanism to invoke this script whenever a file is
// requested within the pseudo-directory, or when a listing of the directory
// itself is requested.
//
// This script should be accompanied by an .htaccess file in the same
// directory.  Typical contents of .htaccess are:
//
//     Options FollowSymLinks
//     RewriteEngine on
//     RewriteRule "^cvs-snapshots(/)?$" "cvssnap.php?$1" [L]
//     RewriteRule "^cvs-snapshots/(.+)$" "cvssnap.php?$1" [L]
//
// This sample .htaccess file specifies a pseudo-directory named
// "cvs-snapshots" and invokes this script (cvssnap.php) with various input
// arguments depending upon the URL requested by the client.  The following
// list summarizes the input arguments:
//
//     http://.../cvs-snapshots	      Invokes script with "" as argument.
//     http://.../cvs-snapshots/      Invokes script with "/" as argument.
//     http://.../cvs-snapshots/file  Invokes script with "file" as argument.
//
// When the argument is a filename, the requested file is returned to the
// client.  When the argument is "/", a formatted, recursive listing of the
// snapshot directory is returned.  When the argument is "", the script
// redirects the client to the same URL, but with a "/" appended.  This forces
// the client browser to believe that it is viewing the contents of a
// directory, rather than a file.
//-----------------------------------------------------------------------------

$prog_name = 'cvssnap.php';
$prog_version = '1.1';
$author_name = 'Eric Sunshine';
$author_email = 'sunshine@sunshineco.com';
$copyright = "Copyright &copy; 2000 by $author_name " .
    "&lt;<a href=\"mailto:$author_email\">$author_email</a>&gt;";

$snapdir = '/home/groups/ftp/pub/crystal/cvs-snapshots';

function pretty_date($stamp)
{
    return gmdate('d M Y H:i:s', $stamp);
}

function pretty_size($bytes)
{
    $KB = 1024;
    $MB = $KB * 1024;
    $GB = $MB * 1024;
    if ($bytes > $GB) { return sprintf('%.1f GB', (float)$bytes / $GB); }
    elseif ($bytes > $MB) { return sprintf('%.1f MB', (float)$bytes / $MB); }
    elseif ($bytes > $KB) { return sprintf('%.1f KB', (float)$bytes / $KB); }
    else { return "$bytes"; }
}

function list_recursive($prefix, $sub)
{
    $dir = strlen($sub) ? "$prefix/$sub" : $prefix;
    $handle = @opendir($dir) or die("Directory \"$dir\" not found.");

    $dir_files = array();
    $dir_subdirs = array();
    while ($entry = readdir($handle))
    {
	if ($entry != '..' && $entry != '.')
	{
	    if (is_dir("$dir/$entry"))
		$dir_subdirs[] = $entry;
	    else
		$dir_files[] = $entry;
	}
    }
    sort($dir_files);
    sort($dir_subdirs);

    if (count($dir_files) > 0) // List all files.
    {
	print("<table border=0 cellspacing=0>\n");
	for ($i=0; $i < count($dir_files); $i++)
	{
	    $file = $dir_files[$i];
	    print('<tr><td align="left" valign="top">' .
		"<a href=\"$sub/$file\">$file</a>&nbsp&nbsp</td>" .
		'<td align="right" valign="top">' .
		pretty_size(filesize("$dir/$file")) . '&nbsp&nbsp</td>' .
		'<td align="left" valign="top">' .
		pretty_date(filemtime("$dir/$file")) . "</td></tr>\n");
	}
	print("</table>\n");
    }

    for ($i=0; $i < count($dir_subdirs); $i++) // Recurse into subdirectories.
    {
	$subdir = $dir_subdirs[$i];
	print("Contents: <tt>$subdir</tt>\n<blockquote>\n");
	list_recursive($prefix, strlen($sub) ? "$sub/$subdir" : $subdir);
	print("</blockquote>\n");
    }

    closedir($handle);
}

function list_root($dir)
{
?>
<html>
<head>
<title>Crystal Space: CVS Snapshots</title>
</head>
<body>
<center><h3>Crystal Space: CVS Snapshots</h3></center><p>
This is a list of the <em>bleeding-edge</em> snapshots and `diffs' of
the Crystal Space CVS repository which are available for download.
Time stamps refer to Universal Coordinated Time (UTC),
which is also known as Greenwich Mean Time (GMT).<p>
<?php
    global $prog_name, $prog_version, $copyright, $snapdir;
    list_recursive($snapdir, $dir);
    print('<hr>Generated ' . pretty_date(time()) . ' UTC ' .
	"by <tt>$prog_name</tt> version $prog_version.<br>\n$copyright\n");
?>
</body>
</html>
<?php
}

function send_file($file)
{
    global $snapdir;
    $base = basename($file);
    $path = "$snapdir/$file";
    file_exists($path) or die ("File \"$file\" not found.");

    header("Content-Disposition: attachment; filename=$base");
    header("Content-Type: application/octet-stream; file=$base");
    header('Content-Length: ' . filesize($path));
    header('Pragma: no-cache');
    header('Expires: 0');
    readfile($path);
}

function redirect()
{
    // Append a "/" to the URI so the browser thinks this is a directory.
    global $HTTP_HOST, $REQUEST_URI;
    header("Location: http://$HTTP_HOST$REQUEST_URI/");
}

$file = $argv[0];
if ($file == '')	// Redirect browser to directory (with trailing '/').
    redirect();
elseif ($file == '/')	// List the pseudo-directory itself.
    list_root('');
else			// Send a file.
    send_file($argv[0]);
?>
