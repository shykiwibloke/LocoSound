//
//  LC_mouseHandler.c
//  LocoControl
//
//  Created by Chris Draper on 19/06/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//

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
		fprintf(stderr, "Right Mouse Button was pressed at x:%d, y:%d \n",g_event.button.x,g_event.button.y);
	}
	else if (g_event.button.button == SDL_BUTTON_LEFT)
	{
		//show where we are
		fprintf(stderr, "Left Mouse Button was pressed at x:%d, y:%d \n",g_event.button.x,g_event.button.y);

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
			//found a free one
			m_mouseCommand[f].IsActive = true;
			m_mouseCommand[f].rect.x = rect->x;

            //WORKING HERE
			//Now create event construct from the cmd
		}
	}

	return 0;

}

/*****************************************
 *
 * unregisterCommand - unregister event actions when button no longer required
 *
 *****************************************/
int unregisterCommand(int idx)
{

    return 0;
}
