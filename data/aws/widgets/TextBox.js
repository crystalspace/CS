/** TextBox constructor. */
function TextBox(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	var sb = prefs.TextBox;
	
	if (settings==null) settings={};
	
	// Give it a pen
	_widget.SetPen(new Pen);
	
	// The text of the titlebar.		
	_widget._text = SafeDefault(settings.text, String(settings.text), "");
	_widget.border = SafeDefault(settings.border, Boolean(settings.border), true);
	_widget.cursor_pos = 0;
	
	// Some widget-specific settings.
	_widget.draw_init=false;
		
	// Initialize the cursor position property.
	_widget.cursor_pos = 0;
	
	// Resize the textbox to a basic size.
	_widget.ResizeTo(100,prefs.Font.GetTextHeight()+5);
	
	// Initialize the alignment
	_widget.halign = SafeDefault(settings.halign, Number(settings.halign), Pen.ALIGN_LEFT);
	_widget.valign = SafeDefault(settings.valign, Number(settings.valign), Pen.ALIGN_CENTER);
	
	// Invalidate and fire onChange when the text property is set.
	_widget.__defineSetter__("text", function(t) { this._text = t; this.Invalidate(); if (this.onChange) this.onChange(this); });		
	_widget.__defineGetter__("text", function()  { return this._text; });	
	
	// When text changes, this is fired if it's set.
	_widget.onChange = null;			
	
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = Skin.current.Style.TextBox;	
	
	// Handle keypresses
	_widget.onKeypress = function(key, modifiers)
	{
		switch(key)
		{
			case Keys.Backspace: this.text = this.text.slice(0, -1); break;
			
			case Keys.Left:  --this.cursor_pos; 
							 if (this.cursor_pos<0) this.cursor_pos=0;
				break;
				
			case Keys.Right: ++this.cursor_pos; 
							 if (this.cursor_pos>text.length) this.cursor_pos=this.text.length;
				break;
				
			default:
				this.text+=key;
				++this.cursor_pos;
				break;
		}	
	}
	
    return _widget;
}
