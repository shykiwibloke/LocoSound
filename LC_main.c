//
//  LC_main.c
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015-2019. All rights reserved.
//
//  VERSION 2.0.0 released 24/04/2019
//

#include "LC_main.h"

/*********************************************
 *
 * Main Entry Point
 *
 *********************************************/

int main(int argc, char *argv[])
{

	int 	quit = 0;
	Uint32 	nextRefresh = 0;

    iniFilePaths();

	getCmdLineOptions(argc, argv);  //parse and process any supplied command line options
	loadConfig();					//load the users config file (used by many modules)
	initGlobals();					//Init the applications global variables
	openLogFile();                  //Open the log file if debug option set
 	initModules();					//Init the SDL systems we rely on for machine portability


	//all ready to begin the main loop

	addMessageLine("Application initialization completed OK");
    //addMessageLine("Press 'F1' for a list of keyboard commands");
	//Main Loop
	while(!quit) {


#ifdef linux

        // NOTE: Serial handling depends on features specific to Linux. This app will compile and run on windows
        //   without the serial comms - and can be used with keyboard commands in a sort of simulator mode

		actionArduinoCommand();

#endif  // linux

		while(SDL_PollEvent(&g_event)) {
			switch(g_event.type) {
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_KEYDOWN:               //Includes most Arduino received events
				//case SDL_KEYUP:
					quit = handleKey(g_event.key);
					break;

				case SDL_MOUSEBUTTONDOWN:       //Includes touch screen events
					handleMouseDown();
					break;
            }
        }

		soundService();								//Service Sound Subsystem - very frequently!

        //Service Screen Subsystem once per 1/4 second
		if SDL_TICKS_PASSED(SDL_GetTicks(),nextRefresh)
		{
 			screenService();
 			nextRefresh = SDL_GetTicks() + 250;
        }

	}  //End of Main Loop

	closeProgram();         //Go clean up
	if(quit==2)
        system ("sudo shutdown -h now");
	return 0;
}

/*********************************************
 *
 * handleKey (Event Handler)
 *
 * Allows keyboard to be used to control sound
 *
 *********************************************/
int handleKey(SDL_KeyboardEvent key) {

	int rtn = 0;

	switch(key.keysym.sym) {

		case SDLK_0:
		case SDLK_1:
		case SDLK_2:
		case SDLK_3:
		case SDLK_4:
		case SDLK_5:
		case SDLK_6:
		case SDLK_7:
		case SDLK_8:
			if (g_LC_ControlState.ThrottlePos < 0)
			{
				addMessageLine("Start the Engine first!");
				break;
			}
			if (g_LC_ControlState.ThrottleActive) {
				g_LC_ControlState.ThrottlePos = (int)key.keysym.sym-48;
				logInt("Setting Throttle to ", g_LC_ControlState.ThrottlePos);
			} else if (g_LC_ControlState.DynBrakeActive) {
				g_LC_ControlState.DynBrakePos = (int)key.keysym.sym-48;
				logInt( "Setting Dynamic Brake to ",g_LC_ControlState.DynBrakePos);
			} else {
				addMessageLine("Error: Cannot set notch. Throttle or Dynamic not set");
			}
			break;
		case SDLK_c:
 			addMessageLine("Clearing current sound queues");
            clearAllQueues();               //resets the sound
            break;
		case SDLK_d:
			g_LC_ControlState.DynBrakeActive = true;
			g_LC_ControlState.ThrottleActive = false;
			addMessageLine("Dynamic Brake Active");
			break;
		case SDLK_e:
			g_LC_ControlState.Emergency = true;
			addMessageLine("Emergency!");
			break;

		case SDLK_f:
			g_LC_ControlState.DirReverse = false;
			g_LC_ControlState.DirForward = true;
			logMessage("Direction set to Forward",true);
			break;

		case SDLK_h:
			if(!g_LC_ControlState.HornPressed)
			{
				g_LC_ControlState.HornPressed = true;    //horn press
				addMessageLine("Horn pressed");
			}
			break;

		case SDLK_j:
            if(g_LC_ControlState.HornPressed)
			{
				g_LC_ControlState.HornPressed = false;    //horn depresss
				addMessageLine("Horn released");
			}
			break;

		case SDLK_n:
			g_LC_ControlState.DirReverse = false;
			g_LC_ControlState.DirForward = false;
			logMessage("Direction set to Neutral",true);
			break;

		case SDLK_r:
			g_LC_ControlState.DirForward = false;
			g_LC_ControlState.DirReverse = true;
			logMessage("Direction set to Reverse",true);
			break;

		case SDLK_s:
		    // start / stop and cant stop until going properly so if we are busy starting or stopping - button ignored
            if (m_startBtn.IsPressed == false && g_LC_ControlState.MotorState == MOTOR_STOPPED)
            {
                g_LC_ControlState.MotorState = MOTOR_STARTING;
                m_startBtn.IsPressed = true;
                snprintf(m_startBtn.textPressed,BTN_TEXT_LEN,"  Wait...");

            }
            else if (m_startBtn.IsPressed == true && g_LC_ControlState.MotorState == MOTOR_RUNNING)
            {
                g_LC_ControlState.MotorState = MOTOR_STOPPING;
                m_startBtn.IsPressed = false;
                snprintf(m_startBtn.textPressed,BTN_TEXT_LEN,"  Wait...");
            }
			break;

		case SDLK_t:
			g_LC_ControlState.ThrottleActive = true;
			g_LC_ControlState.DynBrakeActive = false;
			addMessageLine("Throttle Active");
			break;

		case SDLK_u:
			showChannelSummary();
			break;

		case SDLK_q:  //quit application
		    logMessage("Quitting Application at user request",true);
			rtn = 1;
            break;

        case SDLK_x:  //quit and shut down raspi
            return 2;
            break;

		case SDLK_l:  //LOUD Volume

            m_volumeFullBtn.IsPressed = true;
            m_volumeOffBtn.IsPressed = false;
            m_volumeHalfBtn.IsPressed = false;
            setSoundVolume();
            break;

        case SDLK_m:  //MEDIUM Volume

            m_volumeFullBtn.IsPressed = false;
            m_volumeHalfBtn.IsPressed = true;
            m_volumeOffBtn.IsPressed = false;
            setSoundVolume();
            break;

        case SDLK_o:  //Off Volume

            m_volumeFullBtn.IsPressed = false;
            m_volumeHalfBtn.IsPressed = false;
            m_volumeOffBtn.IsPressed = true;
            setSoundVolume();
            break;

 		default:
		    logInt("Unknown command received ",key.keysym.sym);
			break;

	}
	return(rtn);
}

/*********************************************
 *
 * actionArduinoCommand - takes a string received from the arduino loco controller and actions it
 *
 *********************************************/
#ifdef linux
void actionArduinoCommand(void)
{
	//splits received arduino messages into component parts and populates the supplied structure

    int		    len = 0;
    char	    cmd_str[CMD_MAX_MSG_LEN] = "";
    char		cmd_class;
	char		cmd_arg;
	char*	    cmd_msg;
	int         idx;
	SDL_Event   event;


    len = readSerial(cmd_str,CMD_MAX_MSG_LEN-1); //Service Serial Port. Request one less chars than the buffer can hold
                                                //to guarantee the last char is always a null for terminating argument strings

	if(len < 5) //check bounds of expected message - may not be all here yet not not properly terminated
	{
		return;
	}


    if(cmd_str[1] != ':')   //no colon - could be a rogue log message
    {
        logString("Unformatted command:",cmd_str);;
        return;
    }
    else
    {
        logString("From Arduino: ",cmd_str);

    }

	//string determined to be complete - extract the three fields we want
	cmd_class = cmd_str[0];
	cmd_arg = cmd_str[2];
	cmd_msg = &cmd_str[4];       //point to start of message - note there may not be one with all message types


	switch(cmd_class)
	{
		case 'L':			//Log message received - pass raw input straight through
			logMessage(cmd_str,true);
			break;

		case 'S':			//Sound command
			logInt("Sound: ",cmd_arg);
			event.type = SDL_KEYDOWN;
			event.key.keysym.sym = cmd_arg;
			SDL_PushEvent(&event);
			break;

		case 'M':			//Motor amperage measurement - cmd-arg holds motor number and msg holds amps

            //ensure we dont have a null pointer or out-of-bounds motor number and cause a code exception
            idx = strtol(&cmd_arg,NULL,10);

			if(cmd_msg != NULL && idx >=0 && idx <=5)  //check for bounds of array index
            {
                g_LC_ControlState.motorAmps[idx] = strtol(cmd_msg,NULL,10);     //array is zero based , 0-5 for each motor

                //output debug log entry here showing raw received value
                logInt("Motor ",idx);
                logInt("Amps ",g_LC_ControlState.motorAmps[idx]);
            }

			break;
		case 'V':			//Voltage - battery voltage - cmd-arg will always be 1, msg holds volts
            //ensure we dont have a null pointer
			if(cmd_msg != NULL)
            {
                g_LC_ControlState.vbat = strtol(cmd_msg,NULL,10);     //get the vbat in millivolts
                if (g_LC_ControlState.vbat != 0)
                {
                    g_LC_ControlState.vbat = g_LC_ControlState.vbat/10; //get volts from tenths of a volt
                    logInt("Battery Voltage ",g_LC_ControlState.vbat);
                }
            }
			break;

		case 'E':            //Motors Enabled switch message from Arduino
            g_LC_ControlState.MotorsEnabled = strtol(&cmd_arg,NULL,10);
            break;

		case 'B':            //dynamic Brake switch message from Arduino
            g_LC_ControlState.DynamicEnabled = strtol(&cmd_arg,NULL,10);
            break;

		default:

			logString("Unrecognised command:",cmd_str);
			break;
	}


}
#endif // linux
/*********************************************
 *
 * initGlobals
 *
 * initializes the applications global variables
 *
 *********************************************/
void initGlobals()
{

	//Init control structure once we have our path variable and other initial directives.

	g_LC_ControlState.MotorState        = MOTOR_STOPPED;
	g_LC_ControlState.vbat              = 0;
	g_LC_ControlState.motorAmps[0]      = 10;
	g_LC_ControlState.motorAmps[1]      = 20;
	g_LC_ControlState.motorAmps[2]      = 30;
	g_LC_ControlState.motorAmps[3]      = 40;
	g_LC_ControlState.motorAmps[4]      = 50;
	g_LC_ControlState.motorAmps[5]      = 60;
	g_LC_ControlState.ConsoleHealthy    = false;
	g_LC_ControlState.DirForward        = false;
	g_LC_ControlState.DirReverse        = false;
	g_LC_ControlState.DynBrakeActive    = false;
	g_LC_ControlState.DynBrakePos       = 0;
	g_LC_ControlState.ThrottleActive    = false;
	g_LC_ControlState.ThrottlePos       = 0;
	g_LC_ControlState.Emergency         = false;
	g_LC_ControlState.HornPressed       = false;
	g_LC_ControlState.speed             = 0;

}

/*********************************************
 *
 * InitModules
 *
 * initializes the SDL Library and various modules of the application
 *
 *********************************************/
void initModules(void)
{
    addMessageLine("Winter Creek Loco Sound (c) 2017-2019 Initializing...");
    addMessageLine(PROGRAM_VERSION);

    #ifdef _WIN32
        SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING,"1");
    #endif // _WIN32

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "Unable to initialize SDL:  %s\n", SDL_GetError());
		return;
	}

    SDL_version linked;
    SDL_GetVersion(&linked);

    snprintf(m_msgTempLine,LC_MSGLINE_LEN,"Running SDL version %d.%d.%d",linked.major, linked.minor, linked.patch);
    addMessageLine(m_msgTempLine);
    fprintf(stderr,"%s\n",m_msgTempLine);

	atexit(closeProgram);  //setup the closedown callback


	if(initScreen() != 0)
	{
		fprintf(stderr, "Initalising Screen failed, program terminated\n");
		logMessage("Initalising Screen failed, program terminated", true);
		exit(EXIT_FAILURE);    //error setting up
	}

    snprintf(m_msgTempLine,LC_MSGLINE_LEN,"Running SDL TTF version %d.%d.%d",TTF_MAJOR_VERSION, TTF_MINOR_VERSION, TTF_PATCHLEVEL);
    addMessageLine(m_msgTempLine);
    fprintf(stderr,"%s\n",m_msgTempLine);


	if(initAudio() != 0)
	{
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"FATAL ERROR","Audio Initialization failed",m_mainWindow);
		logMessage("Initalising Audio failed, program terminated",true);
		exit(EXIT_FAILURE);    //error setting up
	}

	snprintf(m_msgTempLine,LC_MSGLINE_LEN,"Running SDL Mixer version %d.%d.%d", SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL);
    addMessageLine(m_msgTempLine);
    fprintf(stderr,"%s\n",m_msgTempLine);


#ifdef linux
	if (initSerial() != 0)    //not strictly SDL but safe place to put it
	{
        addMessageLine("Control stand comms failed to start");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"ERROR","Loco Control Stand Comms Failed to start",m_mainWindow);
		fprintf(stderr,"Warning: Error initializing comms to Loco Control Stand. Keyboard Control Only Mode\n");
	}
    addMessageLine("Control stand comms Initialized....OK");

#endif
#ifdef _WIN32
        addMessageLine("Comms currently disabled for windows version. Use keyboard");

#endif // _WIN32

	return;

}

/*********************************************
 *
 * closeProgram
 *
 * closes out any objects that need disposal
 *
 *********************************************/

void closeProgram(void)
{
	SDL_Quit();
}

/*********************************************
 *
 * unpacks commandline options and executes them
 *
 *********************************************/
void getCmdLineOptions(int argc, char * const argv[])
{
	int ich;

	g_Debug = false;

	//now see what overrides the user has sent
	while ((ich = getopt (argc, argv, "hds:p:")) != EOF) {
		switch (ich) {
			case 'd':           //debug flag - if set forces app to create log files
				g_Debug = true;
				fprintf(stderr,"Debug Option Set\n");
				break;
			case 'h':
				fprintf(stdout,"Usage: -d sets debug mode and creates log files"); //Depricated: , -p <pathname> sets the path for the data files");
				break;
			case '?':
					fprintf (stderr, "Unknown option -%c ignored\n", optopt);
				break;

			default: /* Code when there are no parameters */
				break;
		}
	}

}

