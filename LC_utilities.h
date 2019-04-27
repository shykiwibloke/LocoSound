//
//  LC_utilities.h
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015-2019. All rights reserved.
//
//  VERSION 2.0.0 released 24/04/2019

#ifndef __LocoControl__LC_utilities__
#define __LocoControl__LC_utilities__

#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <SDL2/SDL.h>
#include "LC_globals.h"     //definitions that are common across all MPU's in the Loco Sound system
#include "LC_configReader.h"
#include <time.h>


/*********************************************
 *
 *  Global Variable Declarations
 *
 *********************************************/

char*					g_ProgramPath;		   //System Path this application and its files are running from
FILE                    *g_LogFileHandle;       //The file handle for this sessions log file

/*****************************************
 *
 * Function Prototypes
 *
 *****************************************/

int  setFilePath(const char *,bool forceCreate);
int  setProgramFilePath(void);
void iniFilePaths(void);
void openLogFile(void);
void closeLogFile(void);
void logMessage(const char *,bool AddNewline);
void logInt(const char *, const int);
void logString(const char *, const char *);
char *LTrim(char *s);
char *RTrim(char *s);
char *Trim(char *s);

#endif /* defined(__LocoControl__LC_utilities__) */
