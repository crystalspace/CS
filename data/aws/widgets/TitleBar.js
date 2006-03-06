/** TitleBar factory. */
function TitleBar(inittext)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
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
	
	// Create the buttons.
	_widget.Min = Button();
	_widget.Zoom = Button();
	_widget.Close = Button();
	
	// Add the children
	_widget.AddChild(_widget.Min);	
	_widget.AddChild(_widget.Zoom);
	_widget.AddChild(_widget.Close);
	
 	_widget.Min.ResizeTo(prefs.WindowMin.w, prefs.WindowMin.h);
 	_widget.Min.MoveTo(0, 5);
 	
 	_widget.Zoom.ResizeTo(prefs.WindowZoom.w, prefs.WindowZoom.h);
 	_widget.Zoom.MoveTo(0, 5);
 	
 	_widget.Close.ResizeTo(prefs.WindowClose.w, prefs.WindowClose.h);
 	_widget.Close.MoveTo(0, 5);
 	
 	_widget.Close.SetFrameAnchor(Widget.TRACK_EAST);
 	_widget.Close.Dock(_widget.Zoom, Widget.DOCK_WEST);
 	_widget.Zoom.Dock(_widget.Min, Widget.DOCK_WEST);	
	
	return _widget;
}
