var p = new Pen;
var w = new Widget;

w.MoveTo(100,100);
w.ResizeTo(100,100);

w.onDraw = function(pen)
{	
	var hw = this.width>>1;
	var hh = this.height>>1;
	
	pen.Clear();
	
	pen.SetColor(1,1,1,0.5);
	pen.DrawLine(0,0,this.width,this.height);
	
	pen.SetColor(1,0,0,0.5);
	pen.DrawRect(0,0,hw,hh,false);
	
	pen.SetColor(0,1,0,0.5);
	pen.DrawMiteredRect(hw,hh,this.width,this.height,0.5,true);
	
	pen.SetColor(0,0,1,0.5);
	pen.DrawRoundedRect(hw,0,this.width,hh,0.5,true);
	
	pen.SetColor(1,1,0,0.5);
	pen.DrawArc(0, hh, hw, this.height, 0.14,2.23, true);
	
	pen.SetColor(1,0,1,0.5);
	pen.DrawTriangle(hw, 0, this.width, this.height, 0, this.height, true);	
	
}

w.SetPen(p);
w.Invalidate();
