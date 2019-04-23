//
//  LC_configReader.h
//  LocoSoundv2
//
//  Created by Chris Draper on 9/02/17.
//  Copyright (c) 2015-2019. All rights reserved.
//
//  VERSION 2.0.0 released 24/04/2019

#ifndef LocoControl_LC_configReader_h
#define LocoControl_LC_configReader_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "LC_utilities.h"

#define CONFIG_FILE_NAME "LocoSound.conf"
#define MAX_BUF 255
#define MAX_LABEL 25
#define MAX_VALUE 230

typedef struct {
	char	label[MAX_LABEL];
	char	value[MAX_VALUE];
} config_t;


//Function Prototypes
int loadConfig(void);
int saveConfig(void);
void putConfigVal(const char * label, const char * value);
char * getConfigStr(const char * label);
int getConfigVal(const char * label);
#endif
