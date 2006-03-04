/** TitleBar constructor. */
function TitleBar(inittext)
{
	// Initialize the text property.
	this.text = new String(inittext);
		
	// Initialize the alignment
	this.hAlign = Pen.ALIGN_LEFT;
	this.vAlign = Pen.ALIGN_CENTER;
	
}

// Inherit from widget.
TitleBar.inherits(Widget);

// Setup the onDraw function for all textboxes.
TitleBar.prototype.onDraw = function(pen)
{
	var w = this.width, h = this.height;
	
	// Draw the frame.
	pen.SetColor(Skin.current["FillColor"]);
	pen.DrawArc(0,0,5,h, Math.PI, Math.PI*2, true);
	pen.DrawRect(6,0,w,h, true);
	
	
	pen.SetColor(Skin.current["HighlightColor"]);
	pen.DrawArc(0,0,5,h, Math.PI, Math.PI*2, false);
	
}
