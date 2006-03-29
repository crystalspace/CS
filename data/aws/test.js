var p = new Pen;
var w = new Widget;

var w2 = new Widget;
var p2 = new Pen;

var fnt = new Font("VeraSans", 36);
var tx = new Texture("/lib/stdtex/parket.jpg");

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
	pen.DrawRect(0,0,hw,hh);
	
	pen.SetColor(0,1,0,0.5);
	pen.SetFlag(Pen.FLAG_FILL);
	pen.DrawMiteredRect(hw,hh,this.width,this.height,10);
	
	pen.SetColor(0,0,1,0.5);
	pen.DrawRoundedRect(hw,0,this.width,hh,10);
	
	pen.SetColor(1,1,0,0.5);
	pen.DrawArc(0, hh, hw, this.height, 0.14,2.23);
	
	pen.SetColor(1,0,1,0.5);
	pen.DrawTriangle(hw, 0, this.width, this.height, 0, this.height);	
	
	pen.SetFlag(Pen.FLAG_TEXTURE);	
	pen.SetTexture(tx);
	pen.DrawRect(hw>>1, hh>>1, hw+hw>>1, hh+hh>>1);
	
	pen.ClearFlag(Pen.FLAG_TEXTURE);
	pen.SetColor(1,1,1,1);
	pen.Write(fnt, 0,0, "Test");	
}

// Setup Child.
w2.onDraw = function(pen)
{		
	pen.Clear();
	
	pen.SetColor(0.5,0,0.5,0.5);
	pen.SetFlag(Pen.FLAG_FILL);
	pen.DrawRoundedRect(0,0,this.width,this.height,10);	
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

// Test a checkbox
chk = CheckBox("Enable some setting", Pen.ALIGN_LEFT);
win.AddChild(chk);
chk.MoveTo(5,5);

// Test the radio buttons
rb1 = RadioButton("This", Pen.ALIGN_LEFT);
rb2 = RadioButton("That", Pen.ALIGN_LEFT);
rb3 = RadioButton("The other thing.", Pen.ALIGN_LEFT);
win.AddChild(rb1);
win.AddChild(rb2);
win.AddChild(rb3);

rb1.MoveTo(5,30);
rb2.MoveTo(5,55);
rb3.MoveTo(5,80)

// Test the normal buttons
nb = Button();
win.AddChild(nb);

nb.ResizeTo(100,25);
nb.MoveTo(5,110);
nb.onDrawContent = function(pen) 
{ 
	pen.SetColor(0,0,0,1); 
	pen.WriteBoxed(Skin.current.Font, 0,0,this.width, this.height, Pen.ALIGN_CENTER, Pen.ALIGN_CENTER, "Press Me"); 
}