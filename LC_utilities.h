//
//  LC_utilities.h
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//

#ifndef __LocoControl__LC_utilities__
#define __LocoControl__LC_utilities__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "LC_globals.h"     //definitions that are common across all MPU's in the Loco Sound system

/*********************************************
 *
 *  Global Variable Declarations
 *
 *********************************************/

char					g_DataFilePath[100];   //Global data file path - defaults to current directory or overrided by commandline option
char*					g_ProgramPath;		   //System Path this application and its files are running from


/*****************************************
 *
 * Function Prototypes
 *
 *****************************************/

int  setDataFilePath(void);
int  setProgramFilePath(void);
char *LTrim(char *s);
char *RTrim(char *s);
char *Trim(char *s);

#endif /* defined(__LocoControl__LC_utilities__) */
