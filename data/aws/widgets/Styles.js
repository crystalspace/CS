/******* Frame Styles *********
 *
 *  This file is automatically included
 * so that widgets can use these "predefined"
 * styles. 
 */

Frames = {  
	// Draw a square border
	Rect : function (pen, prefs)
	{
		pen.SetColor(prefs["Border"]);
		pen.DrawRect(0, 0, this.width, this.height, false);
		
		pen.SetColor(prefs["Fill"]);
		pen.DrawRect(0, 0, this.width, this.height, true);
	},
	
	// Draw a round border
	RoundedRect : function (pen, prefs)
	{
		pen.SetColor(prefs["Border"]);
		pen.DrawRoundRect(0, 0, this.width, this.height, prefs["Roundness"], false);
		
		pen.SetColor(prefs["Fill"]);
		pen.DrawRoundRect(0, 0, this.width, this.height, prefs["Roundness"],  true);
	}
};