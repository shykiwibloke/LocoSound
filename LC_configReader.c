//
//  LC_configReader.c
//  LocoSoundv2
//
//  Created by Chris Draper on 9/02/17.
//  Copyright (c) 2017 Winter Creek. All rights reserved.
//

#include "LC_configReader.h"

//the main configuration store
config_t m_config[] =
	{
		{"SCREEN_WIDTH", "1024" },
		{"SCREEN_HEIGHT", "600" },
		{"FONT_FILE","FreeSans.ttf" },
		{"LOCO_NAME","DFT7361" },
		{"BAUD_RATE","9600" },
		{"SERIAL_DEVICE","/dev/ttyAMA0" },
		{"VOL_ENGINE_LEFT","127"},
        {"VOL_ENGINE_RIGHT","127"},
        {"VOL_DYNAMIC_LEFT","127"},
        {"VOL_DYNAMIC_RIGHT","127"},
        {"VOL_AIRCOMPRESSOR_LEFT","127"},
        {"VOL_AIRCOMPRESSOR_RIGHT","127"},
        {"VOL_TRACTION_LEFT","127"},
        {"VOL_TRACTION_RIGHT","127"},
        {"VOL_HORN_LEFT","127"},
        {"VOL_HORN_RIGHT","127"},
		{"REV_UP_IDLE", "0" },
		{"REV_UP_NOTCH1", "200000" },
		{"REV_UP_NOTCH2", "385000" },
		{"REV_UP_NOTCH3", "530000" },
		{"REV_UP_NOTCH4", "700000" },
		{"REV_UP_NOTCH5", "875000" },
		{"REV_UP_NOTCH6", "1010000" },
		{"REV_UP_NOTCH7", "1225000" },
		{"REV_UP_NOTCH8", "99999999" },
		{"REV_DN_NOTCH8", "0" },
		{"REV_DN_NOTCH7", "60000" },
		{"REV_DN_NOTCH6", "120000" },
		{"REV_DN_NOTCH5", "180000" },
		{"REV_DN_NOTCH4", "240000" },
		{"REV_DN_NOTCH3", "310000" },
		{"REV_DN_NOTCH2", "340000" },
		{"REV_DN_NOTCH1", "480000" },
		{"REV_DN_IDLE", "710000" },
		{"FADE_SHORT","500"},
		{"FADE_STD", "1500" },
		{"FADE_LONG", "10000" },
		{"MAX_AMPS","25"}


	};

const int m_CONFIG_SIZE = 39;           //alter this if you add any more config lines above

int loadConfig()
{

	//Opens the user config file and loads in any values as an overlay to the defaults listed above
	//This way - if a user forgets to insert or misspells a value - we fall back on the default value

	setDataFilePath();		//config file must be in the data file directory

    FILE *file = fopen (CONFIG_FILE_NAME, "r");

	if (file != NULL)
	{
		char line[MAX_BUF] = {0};

		while(fgets(line, sizeof(line), file) != NULL)
		{
			char *hash = NULL, *equ = NULL, *s = NULL;
            char label[MAX_LABEL] = {0};
            char value[MAX_VALUE] = {0};

			//check for # in the line = comment so chop off all characters after that
			//by placing a NULL character in place of the # - effectively truncating the line
			hash = strstr((char *)line,"#");
			if(hash)
                *hash = 0;

			//find the equals
			equ = strstr((char *)line,"=");
			if(equ)
			{
				*equ = 0;
				//remove leading and trailing whitespace from the label and value and store them away
				s = Trim(line);
				if(s)
                    memcpy((char*)label,s,strlen(s));
				equ++;
				s = Trim(equ);
				if(s)
                    memcpy((char *)value,s,strlen(s));

				//have a valid label-value pair here to put away
				putConfigVal(label,value);
			}

 		} // End while
		fclose(file);

	} // End of file

    return 0;
}

/****************************************
*
*  putConfigVal
*
*****************************************/
void putConfigVal(const char * label, const char * value)
{
		//iterate through the config m_config[] looking for label, when found - load value in.
		//if not found - fprintf an error message

	int f = 0;
	int len = strlen(label);

	while(f < m_CONFIG_SIZE)
	{
		if(strncmp(m_config[f].label,label,len)==0)   //if the label matches the stored label strncmp returns 0
        {
			memcpy(m_config[f].value,value, strlen(value));
			return;
        }
		f++;
	}

	fprintf(stderr,"Invalid or unknown config variable in file: %s=%s\n",label,value);

}

/****************************************
*
*  getConfigStr
*
*****************************************/

char * getConfigStr(const char * label)
{
	int f = 0;
	int len = strlen(label);

	while(f < m_CONFIG_SIZE)
	{
		if(strncmp(m_config[f].label,label,len)==0)   //if the label matches the stored label strncmp returns 0
			return m_config[f].value;

		f++;
	}

	fprintf(stderr,"Invalid or unknown config variable requested: %s\n",label);
	return NULL;	//specified label not found

}

/****************************************
*
*  getConfigVal
*
*****************************************/
int getConfigVal(const char * label)
{

    //gets the string value, and coverts to integer - BUT will test for nulls to avoid a segmentation fault
    char * str = getConfigStr(label);

    if (str == NULL)
        return 0;
    else
        return atoi(str);

}
