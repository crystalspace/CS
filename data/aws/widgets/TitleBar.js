/** TitleBar constructor. */
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
	
	_widget.Resize(100, Skin.current.TitleBarHeight);
	
	_widget.onDraw = Skin.current.Style.TitleBar;
	
	return _widget;
}
