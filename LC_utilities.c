//
//  LC_utilities.c
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//

#include "LC_utilities.h"

/*********************************************
 *
 * setDataFilePath: Sets Current Directory to the datafiles
 *
 *********************************************/
int setDataFilePath(void)
{
	if(chdir(g_DataFilePath) != 0)    //Change the current working directory to where we can find the sound samples
	{
		fprintf(stderr,"Invalid or protected file path specified with '-p' option: '%s', %s\n", g_DataFilePath, SDL_GetError());
		return 1;
	}
	return 0;
}
/*********************************************
 *
 * setProgramFilePath: Sets Current Directory to the Programfiles
 *
 *********************************************/
int setProgramFilePath(void)
{
	if(chdir(g_ProgramPath) != 0)    //Change the current working directory to where we can find the applications files
	{
		fprintf(stderr,"Invalid or protected file path retrieved as Data File Path: %s\n", g_DataFilePath);
		return 1;
	}
	return 0;
}

/*********************************************
 *
 * string trim utilities LTrim, RTrim and Trim
 *
 *********************************************/

char *LTrim(char *s)	 //returns pointer to first non-space character from the lefthand end or begining of provided string
{
	while(isspace(*s)) s++;
	return s;
}

char *RTrim(char *s)     //returns pointer to first non-space character from the righthand end or end of provided string
{
	char* back = s + strlen(s);
	while(isspace(*--back));
	*(back+1) = '\0';
	return s;
}

char *Trim(char *s)    //returns pointer to first non-space character from lefthand after removing all righthand end spaces.
{
	if (!s)
		return NULL;		//handle NULL string
	if(!*s)
		return s;		//handle empty string

	return RTrim(LTrim(s));
}
