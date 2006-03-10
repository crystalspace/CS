var p = new Pen;
var w = new Widget;

var w2 = new Widget;
var p2 = new Pen;

var fnt = new Font("VeraSans", 36);

w.MoveTo(100,100);
w.ResizeTo(200,200);

w2.MoveTo(50,50);
w2.ResizeTo(50,50);

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

w.SetPen(p);
w2.SetPen(p2);

w.AddChild(w2);

w.Invalidate();
w2.Invalidate();

// Test titlebar
tb = TitleBar("Test");
tb.Dock(w, Widget.DOCK_SOUTH);
tb.ResizeTo(200,tb.height);
tb.MoveTo(400,300);
tb.Invalidate();

// Test statusbar
stb = StatusBar();
w.Dock(stb, Widget.DOCK_SOUTH);

// Test resize knob
rk = ResizeKnob();
w.AddChild(rk);
rk.target=w;

// Test scrollbar
sb1 = ScrollBar(true);
w.AddChild(sb1);

sb2 = ScrollBar(false);
w.AddChild(sb2);

test_clock = Clock();
test_clock.MoveTo(580,20);

// Test the Window
win = Window("Test Window");
win.ResizeTo(300,300);
win.StatusBar.text = "Test complete.";

// Test a tool tip
tt = ToolTip("ToolTip", "A tooltip is designed to show\nshort, useful information.");
tt.SetFocusPoint(test_clock.xmin + (test_clock.width>>1), test_clock.ymax);