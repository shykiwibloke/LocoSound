//
//  LC_mouseHandler.h
//  LocoControl
//
//  Created by Chris Draper on 19/06/15.
//  Copyright (c) 2015-2019. All rights reserved.
//
//  VERSION 2.4.0 released 26/12/2019

#ifndef __LocoControl__LC_mouseHandler__
#define __LocoControl__LC_mouseHandler__



#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "LC_globals.h"     //definitions that are common across all MPU's in the Loco Sound system

#define MAX_MOUSE_COMMANDS 10			//maximum mouse commands that can be registered at any one time


/*********************************************
 *
 *  Global Variable Declarations
 *
 *********************************************/
typedef struct {
	bool		IsActive;
	SDL_Event	event;
	SDL_Rect	rect;
} mouseCommand_t;

/*****************************************
 *
 * Function Prototypes
 *
 *****************************************/

void handleMouseDown(void);
void actionClick(const int x,const int y);
int registerCommand(SDL_Rect *rect, const char *cmd);
int unregisterCommand(const int idx);

#endif /* defined(__LocoControl__LC_mouseHandler__) */
