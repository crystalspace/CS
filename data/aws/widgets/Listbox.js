/** ListBox factory. */
function ListBox(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	if (settings==null) settings={};
	
	// Give it a pen
	_widget.SetPen(new Pen);
			
	// Setup the listbox
	_widget._state=false;
	_widget.over=false;
	_widget.draw_init=false;
	_widget.items=new Array();
	
	// Index of selected item
	_widget.sel_index=0;
	
	// Starting index for displaying list content
	_widget.start_index = 0;	
		
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("selected", function(v) { this.sel_index = v; this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("selected", function() { return this.sel_index; });	
	
	// Set the initial width and height
	_widget.Resize(prefs.ListBox.w, prefs.ListBox.h);	
			
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = Skin.current.Style.ListBox;
	
	// Add a function to help with adding list items.
	_widget.AddItem = function(list_item)
	{
		this.items.push(list_item);	
		this.Invalidate();			
	}
	
	// Add a function to help with adding list items.
	_widget.AddTextItem = function(_text)
	{
		this.items.push(ListBoxTextItem({text:_text}));				
		this.Invalidate();
	}
	
	// Clear all the items in the list box.
	_widget.Clear = function()
	{
		this.items = new Array();
		this.sel_index=0;
		this.start_index=0;
		this.Invalidate();
	}
		
	// If we get a mouse down, find out where.
	_widget.onMouseDown = function(buttons, x, y)
	{
		this.state=true;
		this._active=true;
		this.CaptureMouse();		
		
		var iy=0;
		
		// Find out which item has been selected
		for(var i=this.start_index; i<this.items.length; ++i)
		{
			iy+=this.items[i].GetHeight();
			
			if (y>iy) continue;
			
			this.selected=i;
			break;
		}
	}
	
	// If the mouse is up, release the mouse
	_widget.onMouseUp = function(buttons, x, y)
	{
		this.state=false;
		this._active=false;	
		this.ReleaseMouse();		
	}
	
	_widget.onMouseEnter = function()
	{
		this.over=true;
		this.Invalidate();	
	}
	
	_widget.onMouseExit = function()
	{
		this.over=false;
		this.Invalidate();	
	}
		
	return _widget;
}

function ListBoxTextItem(settings)
{
	var l = new Object;
	
	l.text = settings.text;
	
	if (settings.color==undefined) l.color = Skin.current.TextForeColor;
	else l.color=settings.color;
	
	if (settings.halign==undefined) l.halign = Pen.ALIGN_LEFT;
	else l.halign=settings.halign;
	
	if (settings.valign==undefined) l.valign = Pen.ALIGN_CENTER;
	else l.valign=settings.valign;
	
	l.onDrawItem = function(pen, listbox, x1, y1, x2, y2)
	{
		var prefs = Skin.current;
		var lb = prefs.ListBox;
	
		pen.SetColor(this.color);
		pen.WriteBoxed(prefs.Font, x1, y1, x2, y2, this.halign, this.valign, this.text);							
	}
	
	l.GetHeight = function()
	{
		return Skin.current.Font.GetTextHeight(this.text)+5;
	}	
	
	return l;
}

function ListBoxTextandIconItem(settings)
{
	var l = new Object;
	
	l.text = settings.text;
	l.icon = settings.icon;
	
	//Sys.Print("item: ", l.text, " icon: ", l.icon);
	
	if (settings.color==undefined) l.color = Skin.current.TextForeColor;
	else l.color=settings.color;
	
	if (settings.halign==undefined) l.halign = Pen.ALIGN_LEFT;
	else l.halign=settings.halign;
	
	if (settings.valign==undefined) l.valign = Pen.ALIGN_CENTER;
	else l.valign=settings.valign;
	
	l.onDrawItem = function(pen, listbox, x1, y1, x2, y2)
	{
		var prefs = Skin.current;
		var lb = prefs.ListBox;
						
		var h = this.GetHeight();
		
		pen.SetColor(1,1,1,1);
		pen.SetFlag(Pen.FLAG_TEXTURE);
		pen.SetTexture(this.icon);
		pen.PushTransform();
		pen.Translate(x1+2,y1+2,0);
		pen.DrawRect(0,0, h-5, h-5);
		pen.PopTransform();
		pen.ClearFlag(Pen.FLAG_TEXTURE);
	
		pen.SetColor(this.color);
		pen.WriteBoxed(prefs.Font, x1+h+5, y1, x2, y2, this.halign, this.valign, this.text);							
	}
	
	l.GetHeight = function()
	{
		return Skin.current.Font.GetTextHeight(this.text)+5;
	}	
	
	return l;
}
