//
//  LC_serial.h
//  LocoControl
//
//  Created by Chris Draper on 5/06/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//

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
	#include <SDL2/SDL.h>
	#include "LC_globals.h"     //definitions that are common across all MPU's in the Loco Sound system

	#define SERIAL_DEV "/dev/ttyAMA0"						//default device is for Raspi /todo - move to conf file

	/*****************************************
	 *
	 * Module Level Variables
	 *
	 *****************************************/

//None at this time

	/*****************************************
	 *
	 * Function Prototypes
	 *
	 *****************************************/

	int initSerial(void);
	void closeSerial(void);
	int writeSerial( const void* buf, int byteCount);
	int readSerial(void* buf,const int MaxBytes);


#endif /* defined(__LocoControl__LC_serial__) */
