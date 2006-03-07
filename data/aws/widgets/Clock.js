/** Clock factory. */
function Clock()
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	// Give it a pen
	_widget.SetPen(new Pen);
	
	//  Set initial size.  
	_widget.Resize(100, 100);
	
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = Skin.current.Style.Clock;	
	
	return _widget;
}
