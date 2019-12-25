//
//  LC_mouseHandler.c
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015-2019. All rights reserved.
//
//  VERSION 2.4.0 released 26/12/2019

#include "LC_mouseHandler.h"


mouseCommand_t m_mouseCommand[MAX_MOUSE_COMMANDS];			//max events that can be registered in this version

/*****************************************
 *
 * handleMouseDown - main entry point from Main
 *
 *****************************************/

void handleMouseDown(void)
{

	/* If the left button was pressed. */
	if (g_event.button.button == SDL_BUTTON_RIGHT)
	{
		//show where we are
		if(g_Debug == true) {
            fprintf(stderr, "Right Mouse Button was pressed at x:%d, y:%d \n",g_event.button.x,g_event.button.y);
        }
	}
	else if (g_event.button.button == SDL_BUTTON_LEFT)
	{
		//DEBUG - show where we are
		if(g_Debug == true){
            fprintf(stderr, "Left Mouse Button was pressed at x:%d, y:%d \n",g_event.button.x,g_event.button.y);
		}
		actionClick(g_event.button.x,g_event.button.y);


	}

}
/*****************************************
 *
 * actionClick - check all registered commands and action active ones.
 *
 *****************************************/
void actionClick(const int mx,const int my)
{
	//iterates through the active commands checking to see if mouse click happened within their rectangle
	//action all commands that match.

	int f = 0;

	for(f=0; f<MAX_MOUSE_COMMANDS; f++)
	{
		if(m_mouseCommand[f].IsActive)
		{

			if((m_mouseCommand[f].rect.x < mx)
            && (m_mouseCommand[f].rect.y < my)
            && ((m_mouseCommand[f].rect.w + m_mouseCommand[f].rect.x) > mx)
            && ((m_mouseCommand[f].rect.h + m_mouseCommand[f].rect.y) > my))
			{
				//mouse click was definitely inside an active area - go action its command
				SDL_PushEvent(&m_mouseCommand[f].event);

			}
		}

	}

}
/*****************************************
 *
 * registerCommand - allows buttons and active screen areas to register event actions
 *
 *****************************************/
int registerCommand(SDL_Rect *rect, const char *cmd )
{
	int f = 0;

	for(f=0; f<MAX_MOUSE_COMMANDS; f++)
	{
		if(!m_mouseCommand[f].IsActive)
		{
			//found a free one - populate it
			m_mouseCommand[f].IsActive = true;
			m_mouseCommand[f].rect.x = rect->x;
			m_mouseCommand[f].rect.y = rect->y;
			m_mouseCommand[f].rect.w = rect->w;
			m_mouseCommand[f].rect.h = rect->h;

			//Now create event construct from the cmd that will be used to action the 'click'
			m_mouseCommand[f].event.type = SDL_KEYDOWN;
			m_mouseCommand[f].event.key.keysym.sym = *cmd;
        	return 0;
		}
	}

	return 1;       //error - no free registration slots

}

/*****************************************
 *
 * unregisterCommand - unregister event actions when button no longer required
 *
 *****************************************/
int unregisterCommand(const int idx)
{
	//mark the specified command as inactive
	m_mouseCommand[idx].IsActive = false;
    return 0;
}
