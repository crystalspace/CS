/** TextBox constructor. */
function TextBox(inittext)
{
	// Initialize the text property.
	this.text = new String(inittext);
	
	// Initialize the cursor position property.
	this.cursor_pos = 0;
	
	// Initialize the alignment
	this.hAlign = Pen.ALIGN_LEFT;
	this.vAlign = Pen.ALIGN_CENTER;
	
	// Invalidate and fire onChange when the text property is set.
	this.__defineSetter__("text", function(t) { this.text = t; this.Invalidate(); if (this.onChange) this.onChange(this); });		
	
	// When text changes, this is fired if it's set.
	this.onChange = null;			
}

// Inherit from widget.
TextBox.prototype = new Widget;

// Set the default border method.
TextBox.prototype.DrawFrame = Frames.Rect;

// Setup the onDraw function for all textboxes.
TextBox.prototype.onDraw = function(pen, prefs)
{
	var tw, th;
	
	// Draw the frame.
	this.DrawFrame(pen, prefs);
	
	// Draw the text.
	pen.SetColor(prefs["TextColor"]);
	pen.WriteBoxed(x, y, x+width, y+height, hAlign, vAlign, text);
	
	// Get the width of the text where the cursor is.
	pen.GetTextSize(text.slice(0, cursor_pos), tw, th);
	
	// Draw the cursor position.
	pen.DrawLine(x+tw, y+1, x+tw+1, y+th-2);
}

TextBox.prototype.onKeypress = function(key, modifiers)
{
	switch(key)
	{
		case Keys.Backspace: text = text.slice(0, -1); break;
		
		case Keys.Left:  --cursor_pos; 
						 if (cursor_pos<0) cursor_pos=0;
			break;
			
		case Keys.Right: ++cursor_pos; 
						 if (cursor_pos>text.length) cursor_pos=text.length;
			break;
			
		default:
			text+=key;
			++cursor_pos;
			break;
	}	
}

