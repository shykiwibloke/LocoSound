//
//  LC_serial.h
//  LocoControl
//
//  Created by Chris Draper on 5/06/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//
//  VERSION 1.0.0 released 24/03/2017 in time for use at Keirunga Railways open weekend Easter 2017

#ifdef linux
#ifndef __LocoControl__LC_serial__
	#define __LocoControl__LC_serial__

	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <unistd.h>			//Used for UART
	#include <fcntl.h>			//Used for UART
	#include <termios.h>		//Used for UART
    #include <sys/ioctl.h>		//Used for UART
    #include "LC_configReader.h"
//	#include <SDL2/SDL.h>
//	#include "LC_globals.h"     //definitions that are common across all MPU's in the Loco Sound system


	/*****************************************
	 *
	 * Module Level Variables
	 *
	 *****************************************/
    #define MAX_CMD_LEN 255

    int m_serialhandle;
    char m_assyBuf[MAX_CMD_LEN];

	/*****************************************
	 *
	 * Function Prototypes
	 *
	 *****************************************/

	int initSerial(void);
	void closeSerial(void);
	int writeSerial( const char* buf,const int byteCount);
	int readSerial(char* buf,const int MaxBytes);
	int serviceSerial(char* cmdbuf,const int MaxBytes);
    unsigned int getConfigSpeed(void);

#endif /* defined(__LocoControl__LC_serial__) */

#endif // linux
