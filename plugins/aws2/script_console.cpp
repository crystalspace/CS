#include "cssysdef.h"
#include "script_manager.h"
#include "script_console.h"
#include "csutil/csuctransform.h"

extern "C" {
#include "js/jsapi.h"
}

static scriptConsole _con_;

scriptConsole *ScriptCon()
{
	return &_con_;
}

////////////////

void scriptConsole::Message(const csString &txt)
{
	size_t 	pos=0;
			
	// Break up newlines in message.
	do 
	{
		size_t old_pos=pos;
		pos = txt.FindFirst("\n\r", pos);
		
		if (pos!=(size_t)-1)
		{
			msgs.Push(txt.Slice(old_pos, pos-old_pos));
		}
		else
		{
			msgs.Push(txt.Slice(old_pos));
			break;	
		}				
		
		// Skip the character we found.
		++pos;
		
	} while(pos!=(size_t)-1 && pos<txt.Length());	
	
	// Post it to the logfile too.
	csString tmp=txt;
	tmp+="\n";
	csPrintf(tmp.GetDataSafe());
	
	// Make sure that the console only maintains the history_size specified
	while(msgs.Length()>history_size)
	{
		msgs.DeleteIndex(0);	
	}
}

void scriptConsole::Initialize(iObjectRegistry *obj_reg)
{
	csRef<iKeyboardDriver> currentKbd = 
	CS_QUERY_REGISTRY (obj_reg, iKeyboardDriver);
	
	if (currentKbd == 0)
	{
		return;
	}
	
	composer = currentKbd->CreateKeyComposer ();	
	
	cmd = "Press CTRL+ALT+F1 to activate the console.";
}

void scriptConsole::OnKeypress(csKeyEventData &eventData)
{
	switch(eventData.codeCooked)
	{
		case CSKEY_UP:
			if (cmd_ptr>0)
			{
			  if (cmd_ptr==cmds.Length() && cmd.Length()>0 && (cmds.Length()==0 || cmds.Top() != cmd)) { cmds.Push(cmd); }
				
			  --cmd_ptr;		  
			  
			  cmd = cmds[cmd_ptr];
			  
			  if (cursor_pos>cmd.Length()) cursor_pos=cmd.Length();
			}
			break;
			
		case CSKEY_DOWN:
			if (cmd_ptr<cmds.Length())
			{
			  
				++cmd_ptr;						  
			  	if (cmd_ptr<cmds.Length()) cmd = cmds[cmd_ptr];
			  	else cmd.Clear();			  
			  	
			  	if (cursor_pos>cmd.Length()) cursor_pos=cmd.Length();			  
			}
			break;
			
		case CSKEY_LEFT:
			if (cursor_pos) --cursor_pos;
			break;
			
		case CSKEY_RIGHT:
			if (cursor_pos<cmd.Length()) ++cursor_pos;
			break;			
		
		case CSKEY_BACKSPACE:
			if (cursor_pos)
			{
				cmd.DeleteAt(cursor_pos-1);
				--cursor_pos;
			}
				
			break;
			
		case CSKEY_ENTER:
			if (cmd.Length())
			{
				JSScript *sc;
				jsval tmp;
				
				Message(cmd);
				cmds.Push(cmd);
				cmd_ptr=cmds.Length();
				
				if((sc=JS_CompileScript(ScriptMgr()->GetContext(), ScriptMgr()->GetGlobalObject(),
						cmd.GetData(), cmd.Length(),
						"command line", 1))!=0)
				{
					if (JS_ExecuteScript(ScriptMgr()->GetContext(), ScriptMgr()->GetGlobalObject(), sc, &tmp)==JS_FALSE)
					{						  					
						
						Message("error: command execution failed.");
					}	
					else  // Return the results of the expression.
					{
						csString msg;
						
						msg="[";
						msg+=JS_GetTypeName(ScriptMgr()->GetContext(), JS_TypeOfValue(ScriptMgr()->GetContext(), tmp));
						msg+="] : ";
						
						JSString *result = JS_ValueToString(ScriptMgr()->GetContext(), tmp);
						
						msg+= JS_GetStringBytes(result);
						
						Message(msg);							
					}				    				
											 
					JS_DestroyScript(ScriptMgr()->GetContext(), sc);
				}
				else
				{
					Message("error: command compilation failed.");			
				}	  	
			}
			
			cmd.Clear();
			cursor_pos=0;
			
			break;
		
		default:
		{
			if (CSKEY_IS_SPECIAL (eventData.codeCooked))
	  			break;	
	  			
	  		utf32_char composedCh[2];
			int composedCount;
			
			if (composer->HandleKey (eventData, composedCh, 
	  								 sizeof (composedCh) / sizeof (utf32_char), 
	  								 &composedCount) != csComposeNoChar)
			{			 
			  for (int n = 0; n < composedCount; n++)
			  {
			    utf8_char ch[CS_UC_MAX_UTF8_ENCODED + 1];
			    size_t chSize = csUnicodeTransform::EncodeUTF8 (composedCh[n], ch, sizeof (ch) / sizeof (utf8_char));
			    ch[chSize] = 0;
			   
			    if (cursor_pos == cmd.Length())
			    { 
			    	cmd.Append ((char*)ch);
		    	}
		    	else
		    	{
			    	cmd.Insert(cursor_pos, (char*)ch);	
		    	}
			    ++cursor_pos;
		      }		      
			    
		    }  			
	  		
	  			
  		} break;	
	  			
	  		
	}	
}

void scriptConsole::Redraw(iGraphics2D * g2d)
	{
				
		int thd = font->GetTextHeight();
		int y=g2d->GetHeight()-thd;
		int tw=0,th=thd;
		
		size_t i=msgs.Length();
		
		int fg = (active ? g2d->FindRGB(192,192,192,255) : g2d->FindRGB(128,128,128,128));
		
		// Write the current command line.
		g2d->Write(font, 5, y, fg, -1, cmd.GetData());
				
		// Draw the cursor		
		if (cursor_pos!=0)
		{				
			csString tmp = cmd.Slice(0, cursor_pos);
			font->GetDimensions(tmp.GetData(), tw, th);
		}
		
		// The cursor color.
		g2d->DrawBox(5+tw, y, 1, th, fg);		
		
		// Adjust for command-line.
		y-=thd;
		
		while(y>0 && i>0)
		{
			--i;
			y-=thd;
			g2d->Write(font, 5, y, fg, -1, msgs[i].GetData());
		}				
	}
	
// newline
