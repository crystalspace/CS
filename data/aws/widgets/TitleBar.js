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
	
	_widget.onDraw = function(pen)
	{
		var w = this.width, h = this.height;
		
		// Draw the frame.
		pen.SetColor(Skin.current["FillColor"]);
		pen.DrawArc(0,0,5,h, Math.PI, Math.PI*2, true);
		pen.DrawRect(6,0,w,h, true);
		
		
		pen.SetColor(Skin.current["HighlightColor"]);
		pen.DrawArc(0,0,5,h, Math.PI, Math.PI*2, false);
		
	}
	
	return _widget;
}
