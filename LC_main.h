//
//  LC_main.h
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//

#ifndef LocoControl_LC_main_h
#define LocoControl_LC_main_h

#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "LC_globals.h"     //definitions that are common across all MPU's in the Loco Sound system
#include "LC_utilities.h"
#include "LC_screen.h"
#include "LC_sound.h"
#include "LC_mouseHandler.h"
#include "LC_configReader.h"
#include "LC_serial.h"


/*********************************************
 *
 *  Module Variable Declarations
 *
 *********************************************/
#define CMD_MAX_MSG_LEN 255  //message as received from the arduino


/*****************************************
 *
 * Function Prototypes
 *
 *****************************************/

void actionCommand(char *str, int len);
void initGlobals(void);
void closeProgram(void);
int  initSDL(void);
void getCmdLineOptions(int argc, char * const argv[]);
int  handleKey(SDL_KeyboardEvent key);


#endif
