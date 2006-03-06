/** ScrollBar factory. */
function ScrollBar(orientation_vertical)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	// Give it a pen
	_widget.SetPen(new Pen);
		
	// Set the orientation of the ScrollBar
	_widget.orientation_vertical = orientation_vertical;
	
	// Setup the scroll bar
	_widget.bar_size=50;
	_widget.value=0;
	_widget.max=100;
	_widget.min=0;
		
	//  Set initial size
	if (orientation_vertical)
	{	// vertical		
		_widget.Resize(prefs.ScrollBarWidth, 20);
		_widget.SetFrameAnchor(Widget.STICK_NORTH | Widget.STICK_SOUTH | Widget.TRACK_EAST);
	}
	else
	{	// horizontal
		_widget.Resize(20, prefs.ScrollBarHeight);		
		_widget.SetFrameAnchor(Widget.STICK_EAST | Widget.STICK_WEST | Widget.TRACK_SOUTH);
	}
	
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = Skin.current.Style.ScrollBar;
	
	return _widget;
}
