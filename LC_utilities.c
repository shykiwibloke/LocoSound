//
//  LC_utilities.c
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015-2019. All rights reserved.
//
//  VERSION 2.0.0 released 24/04/2019

#include "LC_utilities.h"

/*********************************************
 *
 * setDataFilePath: Sets Current Directory to the datafiles
 *
 *********************************************/

int setFilePath(const char *path, bool forceCreate)
{
    setProgramFilePath();   //file path could be relative to program path so set that first

	if(chdir(path) != 0)    //Change the current working directory to where we can find the sound samples
	{
        if(forceCreate)            //if directory does not exist - caller wants it created
        {
            #ifdef linux
            if(mkdir(path,S_IRWXU | S_IRWXG | S_IRWXO) != 0)
            #endif // linux         //Raspi version of codeblocks does not support #elifdef
            #ifdef WIN32
            if(mkdir(path) != 0)
            #endif // WIN32

            {
                fprintf(stderr,"Could not create or verify directory '%s'\n",path);
                return 1;
            }
            chdir(path);
        }
        else
        {
            fprintf(stderr,"Invalid or protected file path argument: '%s'\n", path);
            return 1;
        }
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
	if(g_ProgramPath)
    {
        if(chdir(g_ProgramPath) == 0)    //Change the current working directory to where we can find the applications files
        {
            return 0;
        }
    }

    fprintf(stderr,"Invalid or protected Program File Path: %s\n", g_ProgramPath);
    return 1;
}


/*********************************************
 *
 * initialize file paths
 *
 *********************************************/

void iniFilePaths(void)
{
    	//set defaults
    g_ProgramPath = SDL_GetBasePath();
    setProgramFilePath();                   //need to explicitly set the path for some env such as windows
}


/*********************************************
 *
 * open Log File
 *
 *********************************************/

void openLogFile(void)
{
    if(g_Debug)
    {
        if (setFilePath(getConfigStr("LOG_FILE_PATH"),true))
        {
            g_Debug = false;        //Cant open log file directory so cant debug
            return;
        }

        rename("Log.txt","Log_OLD.txt");            //keep one old log file about dont care if not already exist - try
        g_LogFileHandle = fopen("Log.txt","w");

        atexit(closeLogFile);

        fprintf(stderr,"Debug option Set. Log File created.\n");

	}

}

/*********************************************
 *
 * close Log File
 *
 *********************************************/

void closeLogFile(void)
{
   logMessage("Log File Closed",true);
   fprintf(stderr,"Log file closed");
   fclose(g_LogFileHandle);
}

/*********************************************
 *
 * Log a message to the Log File
 *
 *********************************************/

void logMessage(const char * msg,bool AddNewline)
{
 if(g_LogFileHandle)
 {
     /* get seconds since the Epoch */
     time_t secs = time(0);

     /* convert to localtime */
     struct tm *ltime = localtime(&secs);

    if (AddNewline)
        fprintf(g_LogFileHandle,"%02d:%02d:%02d - %s\n", ltime->tm_hour, ltime->tm_min, ltime->tm_sec,msg);
     else
        fprintf(g_LogFileHandle,"%02d:%02d:%02d - %s", ltime->tm_hour, ltime->tm_min, ltime->tm_sec,msg);
 }
}

/*********************************************
 *
 * Log Integer Variable and its associated message into the log file
 *
 *********************************************/

void logInt(const char *msg, const int i)
{
    if(g_LogFileHandle)
    {
        logMessage(msg,false);
        fprintf(g_LogFileHandle,"%d\n",i);
    }
}

/*********************************************
 *
 * Log String Variable and its associated message into the log file
 *
 *********************************************/

void logString(const char *msg, const char *s)
{
    if(g_LogFileHandle)
    {
        logMessage(msg,false);
        fprintf(g_LogFileHandle,"%s\n",s);
    }
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
