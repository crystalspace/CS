// Performs the function of transforming an XML UI into a set of nested widgets.
function ParseXMLInterface(node, parent_widget)
{	
	var widget;		
			
	Sys.Print("xml: ", node.@type);
			
	widget = eval(node.@type + "(node)");
	
	// Create the widget name, if it needs one.
	if (!(node.@id==undefined))
	{
		eval(node.@id + "=widget;");	
	}
	
	// Perform whatever setup is necessary.
	if (!(node.setup==undefined))
	{
		eval(node.setup.toString());	
	}
	
	// Parse event handlers
	for each (var ev in node.event)
	{						
		if (ev.@name==undefined || ev.@action==undefined)
		{
			Sys.Print("error: in parsing XML interface spec at section:");
			Sys.Print(ev);
			Sys.Print("Missing an event name or event action.  Both are REQUIRED parameters.");
			return;	
		}
		
		// Set the event up
		var set = "widget." + ev.@name + "=" + ev.@action + ";";				
		eval(set);	
		
		// Get any settings that need to modify the widget for this event.
		for (var s in ev.settings)
		{
			var setting = ev.settings[s];
			widget[setting.@name] = setting.@value;
		}
	}
					
	// Recursively descend and get the child layouts.
	for each(var cn in node.layout)
	{			
		ParseXMLInterface(cn, widget);						
	}
	
	// Recursively descend and get the child widgets.
	for each(var cn in node.widget)
	{						
		ParseXMLInterface(cn, widget);								
	}
			
	// Add it to the parent, if there is one.
	if (parent_widget!=null) parent_widget.AddChild(widget);
	
    return widget;
}