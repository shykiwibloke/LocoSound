//
//  LC_utilities.h
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//
//  VERSION 1.0.2 released 11/04/2017

#ifndef __LocoControl__LC_utilities__
#define __LocoControl__LC_utilities__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <SDL2/SDL.h>
#include "LC_globals.h"     //definitions that are common across all MPU's in the Loco Sound system

/*********************************************
 *
 *  Global Variable Declarations
 *
 *********************************************/

/* Depricated: Data file paths now set in configReader
char					g_DataFilePath[100];   //Global data file path - defaults to current directory or overrided by commandline option
*/
char*					g_ProgramPath;		   //System Path this application and its files are running from


/*****************************************
 *
 * Function Prototypes
 *
 *****************************************/

int  setFilePath(char *);
int  setProgramFilePath(void);
void iniFilePaths(void);
char *LTrim(char *s);
char *RTrim(char *s);
char *Trim(char *s);

#endif /* defined(__LocoControl__LC_utilities__) */
