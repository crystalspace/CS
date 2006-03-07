var p = new Pen;
var w = new Widget;

var w2 = new Widget;
var p2 = new Pen;

var w3 = new Widget;
var p3 = new Pen;

var fnt = new Font("VeraSans", 36);

w.MoveTo(100,100);
w.ResizeTo(200,200);

w2.MoveTo(50,50);
w2.ResizeTo(50,50);

// Just to give it some size.
w3.ResizeTo(50,100);

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
	
	pen.SetColor(1,1,1,1);
	pen.Write(fnt, 0,0, "Test");
	
}

// Setup Child.
w2.onDraw = function(pen)
{		
	pen.Clear();
	
	pen.SetColor(0.5,0,0.5,0.5);
	pen.DrawRoundedRect(0,0,this.width,this.height,0.25,true);	
}

// Setup Child.
w3.onDraw = function(pen)
{		
	pen.Clear();
	
	pen.SetColor(0.25,0,0.25,0.5);
	pen.DrawMiteredRect(0,0,this.width,this.height,0.25,true);	
}


w.SetPen(p);
w2.SetPen(p2);
w3.SetPen(p3);

w.AddChild(w2);
w.Dock(w3, Widget.DOCK_SOUTH);

w.Invalidate();
w2.Invalidate();
w3.Invalidate();

// Test titlebar
tb = TitleBar("Test");
tb.Dock(w, Widget.DOCK_SOUTH);
tb.ResizeTo(200,tb.height);
tb.MoveTo(400,300);
tb.Invalidate();

// Test scrollbar
sb1 = ScrollBar(true);
w.AddChild(sb1);
sb1.Invalidate();

sb2 = ScrollBar(false);
w.AddChild(sb2);
sb2.Invalidate();

test_clock = Clock();
test_clock.MoveTo(700,0);