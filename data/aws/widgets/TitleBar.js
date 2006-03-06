/** TitleBar factory. */
function TitleBar(inittext)
{
	var _widget = new Widget;
	
	// Give it a pen
	_widget.SetPen(new Pen);
	
	// Initialize the text property.
	_widget.text = new String(inittext);
		
	// Initialize the alignment
	_widget.hAlign = Pen.ALIGN_LEFT;
	_widget.vAlign = Pen.ALIGN_CENTER;
	
	//  Set initial size.  When you dock this bar, or stick it
	// in the top of a window as a child, it will resize. (Assuming
	// you use anchors if it's a child.)
	_widget.Resize(100, Skin.current.TitleBarHeight);
	
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = Skin.current.Style.TitleBar;
	
	return _widget;
}
