<?php
//-----------------------------------------------------------------------------
// Module to allow users to annotate online documentation.
// Matthias Braun <matze@braunis.de>
//-----------------------------------------------------------------------------

$basedir="/home/groups/c/cr/crystal/htdocs/annotations";

$theme=preg_replace("\" \"","_",$theme);
$theme=preg_replace("[^a-zA-Z0-9_]","",$theme);
$file=$basedir."/$theme.xml";
$self=$PHP_SELF;

readXMLFile($file);
print "<hr><a name=\"comments\">\n";

if ($action=="") {
    printEntries();
    print "<a href=\"$self?action=showadd#comments\">Add a comment</a><br>\n";
} elseif ($action == "showadd") {
    print "<form action=\"$self?action=add#comments\" method=\"post\">\n";
    print "<b>Name:</b><input name=\"authorname\" value=\"\"><br>\n";
    print "<b>E-Mail:</b><input name=\"emailname\" value=\"\"><br>\n";
    print "<textarea cols=44 rows=8 name=\"texttext\" wrap=\"virtual\"></textarea><br>\n";
    print "<input type=\"submit\" value=\"Add Comment\"><br>\n";
    print "</form><br>\n";
    printEntries();
} elseif ($action == "add") {
    $newentry = new entry;
    $newentry->author=$authorname;
    $newentry->email=$emailname;
    if ($authorname=="" && $emailname=="")
	die ("<br><h2>Please give name or email!</h2>\n");
    $newentry->date=time();
    $newentry->text=nl2br($texttext);
    $entries[] = $newentry;
    $h= fopen ($file, "w");
    if (!$h) {
	die ("Couldn't write to disk!<br>\n");
    }
    writeXMLFile($h);
    include ("mail.php");
    foreach ($mail as $i) {
	mail ($i, "CS documentation annotated by $authorname ($emailname)",
		"Author: $authorname\n".
		"E-Mail: $emailname\n".
		"Topic:  $theme\n".
		"File:   $self\n".
		"Time:   ".strftime("%a, %d %b %G (%H:%M)")." ($date)\n".
		"Comment:\n".
		"$texttext");
    }

    fclose($h);
    chmod($file, 0666);
    print "<h3>Comment added!</h3>\n";
    unset($entries);
    readXMLFile($file);
    printEntries();
}   

class entry
{
    var $author;
    var $email;
    var $text;
    var $date;

    function entry()
    {
	$this->author="";
	$this->email="";
	$this->text="";
	$this->data=0;
    }

    function writeE($h)
    {
	if ($this->author=="" && $this->email=="") {
		return;
	}

	fputs ($h, "<comment>\n");
	fputs ($h, "<author>".$this->author."</author>\n");
	fputs ($h, "<email>".$this->email."</email>\n");
	fputs ($h, "<date>".$this->date."</date>\n");
	fputs ($h, "<?text ".$this->text ."?>\n");
	fputs ($h, "</comment>\n");
    }
}

function printEntries()
{
    global $entries;
    print "<br><br>\n";

    foreach ($entries as $e) {
	print "<table width=\"100%\" bgcolor=\"#88bbff\" celspacing=\"0\" celpadding=\"0\">\n";
	print "<tr><td>\n";
	if ($e->author != "") {
		print $e->author." ";
	} 
	if ($e->email != "") {
		print "<a href=\"mailto:".$e->email."\">".$e->email."</a>";
	}
	print "</td>\n";

	print "<td align=\"right\"><b>\n";
	if ($e->date != 0)
	{
	    print strftime("%a, %d %b %G (%H:%M)", $e->date) . " UTC<br>\n";
	}
	print "</b></td></tr>\n";
	print "</table>\n";
	print "<table width=\"100%\" bgcolor=\"#eeeeee\"><tr><td>";
	print $e->text;
	print "</td></tr></table>\n";
	print "<br>\n";
    }
}

function readXMLFile($file)
{
    global $status;
    global $entries;

    if (!file_exists($file)) {
	$newe = new entry;
	$newe->text="<h2>No user comments yet!</h2>\n";
	$entries[] = $newe;
	return;
    }

    $xml_parser = xml_parser_create();
    xml_set_element_handler($xml_parser, "startElement", "endElement");
    xml_set_character_data_handler($xml_parser, "characterData");
    xml_set_processing_instruction_handler($xml_parser, "piHandler");
    if (!($fp = fopen($file, "r"))) {
        die ("could not open XML input");
    }
    
    $status="global";
    
    while ($data = fread($fp, 4096)) {
        if (!xml_parse($xml_parser, $data, feof($fp))) {
	    die(sprintf("XML error: %s at line %d",
    		xml_error_string(xml_get_error_code($xml_parser)),
    		xml_get_current_line_number($xml_parser)));
        }
    }
    xml_parser_free($xml_parser);
}

function writeXMLFile($h)
{
    global $entries;

    fputs ($h, "<?xml version=\"1.0\"?>\n");
    fputs ($h, "<comments>\n");
    foreach ($entries as $e) {
	$e->writeE($h);
    }

    fputs ($h, "</comments>\n");
}

function startElement($parser, $name, $attrs) {
    global $status;
    global $aentry;
    switch ($status) {
	case "global":
	   if ($name == "COMMENT") {
		$status="comment";
		$aentry=new entry();
	    }
	    
	    break;
	case "comment":
	    if ($name == "AUTHOR") {
		$status="author";
		$aentry->author = $xmlglobdata;
	    }
	    if ($name == "EMAIL") {
		$status="email";
		$aentry->email = $xmlglobdata;
	    }
	    if ($name == "DATE") {
		$status="date";
		$aentry->date = (int)$xmlglobdata;
	    }
	    if ($name == "TEXT") {
		$status="text";
		$aentry->text = $xmlglobdata;
	    }
	    break;
    }
}

function endElement($parser, $name) {
    global $status;
    global $aentry;
    global $xmlglobdata;
    global $entries;

    switch ($status) {
	case "comment":
	    if ($name == "COMMENT") {
		$status="global";
		$entries[] = $aentry;
	    }
	    break;
	case "author":
	    $aentry->author = $xmlglobdata;
	    $status="comment";
	    break;
	case "email":
	    $aentry->email = $xmlglobdata;
	    $status="comment";
	    break;
	case "date":
	    $aentry->date = (int)$xmlglobdata;
	    $status="comment";
	    break;
	case "text":
	    if ($name != "TEXT") { break; }
	    $aentry->text = $xmlglobdata;
	    $status="comment";
	    break;
    }
}

function characterData($parser, $data) {
    global $xmlglobdata;
    $xmlglobdata=$data;
}

function piHandler($parser, $name, $data) {
    global $status;
    global $aentry;
    
    if ($status=="comment" && $name=="text") {
	$aentry->text=$data;
    }
}

?>
