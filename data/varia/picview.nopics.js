/////////////////////////////////////////////////////
////////// No pictures  and/or error widget def  ////
/////////////////////////////////////////////////////


// Create a new widget with the picview controls in it.
function EmptyPicDir()
{
	label_thing = 	 	  	 
		 <widget type="ToolTip">
		 	<setup>widget.SetFocusPoint(controls.xmin + (controls.width>>1), controls.ymax);</setup>
		 	<title>No Pictures</title>
		 	<text>The directory that you started picview from does
not contain any pictures that this app
can recognize.</text>	 	
		 </widget>;
	
	return ParseXMLInterface(label_thing, null);
}

// Create a new controls widget
ll = EmptyPicDir();

// Show the tip
ll.Show();

