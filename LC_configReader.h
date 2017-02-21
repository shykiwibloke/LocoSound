//
//  LC_configReader.h
//  LocoSoundv2
//
//  Created by Chris Draper on 9/02/17.
//  Copyright (c) 2017 Winter Creek. All rights reserved.
//

#ifndef LocoControl_LC_configReader_h
#define LocoControl_LC_configReader_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "LC_globals.h"     //definitions that are common across all MPU's in the Loco Sound system
#include "LC_utilities.h"


#define CONFIG_FILE_NAME "lococtl.conf"
#define MAX_BUF 255
#define MAX_LABEL 25
#define MAX_VALUE 230

typedef struct {
	char	label[MAX_LABEL];
	char	value[MAX_VALUE];
} config_t;


//Function Prototypes
int loadConfig(void);
void putConfigVal(const char * label, const char * value);
char * getConfigVal(const char * label);

#endif