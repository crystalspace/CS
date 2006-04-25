function Horizontal(padding)
{
	l = new Widget;
	
	l._current_width = 0;
	l._current_height = 0;
	
	l.padding = padding;
	l.kids = new Array();
	
	l.onDraw = Skin.current.Style.EmptyLayout;
		
	// When a child is added, grow the box.
	l.onAddChild = function(child)
	{
		var new_h = child.height + (this.padding*2);
		
		child.MoveTo(this._current_width + this.padding, this.padding);
						
		this._current_width += child.width + this.padding;
		
		if (new_h > this._current_height) 
		{	
			this._current_height = new_h;
			var adjusted_h = child.height;
			
			// Resize the height of all children.
			for(var kid in this.kids)
			{				
				kid.ResizeTo(kid.width,adjusted_h);									
			}
		}
		else
		{
			// Make it the same height as the box.
			child.ResizeTo(child.width,this._current_height - (this.padding*2));			
		}
		
				
		this.ResizeTo(this._current_width, this._current_height);			
						
		this.kids.push(child);
	}	
	
	return l;
}