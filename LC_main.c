//
//  LC_main.c
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//
//  VERSION 1.0.0 released 24/03/2017 in time for use at Keirunga Railways open weekend Easter 2017

#include "LC_main.h"


/*********************************************
 *
 * Main Entry Point
 *
 *********************************************/

int main(int argc, char *argv[]) {

	int 	quit = 0;
	Uint32 	lastrun = 0;


    iniFilePaths();

	getCmdLineOptions(argc, argv);  //parse and process any supplied command line options
	loadConfig();					//load the users config file (used by many modules)
	initGlobals();					//Init the applications global variables
	initModules();						//Init the SDL systems we rely on for machine portability

	if(g_Debug)
        addMessageLine("Debug option set by command line");

	//all ready to begin the main loop

	addMessageLine("Initialization complete. Press 'F1' for a list of keyboard commands");
	//Main Loop
	while(!quit) {

		//check for SDL Events - and process accordingly

		while(SDL_PollEvent(&g_event)) {
			switch(g_event.type) {
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_KEYDOWN:
				//case SDL_KEYUP:
					quit = handleKey(g_event.key);
					break;

				case SDL_MOUSEBUTTONDOWN:
					handleMouseDown();
					break;
			}
		}

		soundService();								//Service Sound Subsystem - very frequently!
		SDL_Delay(10);								//relinquish cpu for 10ms to allow other SDL threads to execute.
                                                    //Found this to stabilize SDL_mixer

// NOTE: Serial handling depends on features specific to Linux. This app will compile and run on windows
//   without the serial comms - and can be used with keyboard commands in a sort of simulator mode

#ifdef linux
        int		len = 0;
        char	cmd_str[CMD_MAX_MSG_LEN];

        len = readSerial(cmd_staddMessageLiner,CMD_MAX_MSG_LEN); //Service Serial Port
		if(len > 0)
		{
			actionCommand(cmd_str,len);
		}
#endif  // linux

        //Service Screen & comms Subsystems once per second
		if SDL_TICKS_PASSED(SDL_GetTicks(),lastrun)
		{
			screenService();
			lastrun = SDL_GetTicks()+250;
		}

	}  //End of Main Loop

	closeProgram();									//Go clean up
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
				fprintf(stderr, "Setting Throttle to %d\n", g_LC_ControlState.ThrottlePos);
			} else if (g_LC_ControlState.DynBrakeActive) {
				g_LC_ControlState.DynBrakePos = (int)key.keysym.sym-48;
				fprintf(stderr, "Setting Dynamic Brake to: %d\n",g_LC_ControlState.DynBrakePos);
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
			fprintf(stderr, "Emergency!\n");
			break;

		case SDLK_f:
			g_LC_ControlState.DirReverse = false;
			g_LC_ControlState.DirForward = true;
			fprintf(stderr, "Direction set to Forward\n");
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
			fprintf(stderr, "Direction set to Neutral\n");
			break;

		case SDLK_r:
			g_LC_ControlState.DirForward = false;
			g_LC_ControlState.DirReverse = true;
			fprintf(stderr, "Direction set to Reverse\n");
			break;

		case SDLK_s:
			g_LC_ControlState.ThrottleActive = true;		//make throttle active and trigger start
			g_LC_ControlState.DynBrakeActive = false;
			g_LC_ControlState.ThrottlePos = 0;
			addMessageLine("Starting Prime Mover...");
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
		    fprintf(stderr,"Quitting Application at user request\n");
			rtn = 1;
            break;

		case SDLK_F1:
          //  snprintf(m_msgTempLine,MSG_RECT_LINE_LENGTH,"",);
            addMessageLine(" ");
            addMessageLine("Available Commands List: (all lower case)");
            addMessageLine("-----------------------------------------");
            addMessageLine("s = Start Engine (or use mouse to click the 'Engine Start' Button");
            addMessageLine("t = Throttle mode");
            addMessageLine("d = Dynamic mode");
            addMessageLine("0-8 = Idle and notches 1 to 8. Sound changes for Throttle or Dynamic mode");
            addMessageLine("h = start sounding the horn");
            addMessageLine("j = stop sounding the horn");
            addMessageLine("q = quit the application gracefully");
            addMessageLine("u = update on sound queue activity to log - a debug command");
            addMessageLine("c = cancel all currently playing sounds - debug command");
            addMessageLine("-----------------------------------------");
            break;
		default:
		    fprintf(stderr, "Unknown command '%c' received \n",key.keysym.sym);
			break;

	}
	return(rtn);
}

/*********************************************
 *
 * actionCommand - takes a string received from the arduino loco controller and actions it
 *
 *********************************************/

void actionCommand(char *str,const int len)
{
	//splits received arduino messages into component parts and populates the supplied structure

	char		cmd_class = ' ';
	char		cmd_arg =  ' ';
	char		cmd_msg[len];
	SDL_Event   event;

    memset(cmd_msg,0,len);    //explicitly zero the buffer

	if(str == NULL)
	{
		return;
	}

	if(len < 5 || len >= CMD_MAX_MSG_LEN )    //check bounds of message
	{
		return;
	}

	//string determined to be safe - extract the three fields we want
	cmd_class = str[0];
	cmd_arg = str[2];
	strncpy(cmd_msg, &str[4], len);


	switch(cmd_class)
	{
		case 'L':			//Log message received - pass straight on to std err (which can be redirected)
		    //TODO - examine log level and decide if should be logged based on g_Debug setting
			fprintf(stderr,"Log: %c: %s\n",cmd_arg, cmd_msg);
			break;

		case 'S':			//Sound command
			fprintf(stderr,"XXXXXXXXXXXXXXXXXXXXXXXXXXX\n\n\n");
			fprintf(stderr,"Sound: %c\n",cmd_arg);
			event.type = SDL_KEYDOWN;
			event.key.keysym.sym = cmd_arg;
			SDL_PushEvent(&event);
			break;

		case 'M':			//Motor amperage measurement - cmd-arg holds motor number and msg holds amps

            //ensure we dont have a null pointer or out-of-bounds motor number and cause a code exception
			if(cmd_msg != NULL || cmd_arg >0 || cmd_arg <7)
            {
                g_LC_ControlState.motorAmps[cmd_arg-1] = atoi(cmd_msg);     //array is zero based , 0-5 for each motor
            }
			break;
		case 'V':			//Voltage - battery voltage - cmd-arg will always be 1, msg holds volts
            //ensure we dont have a null pointer

			if(cmd_msg != NULL)
            {
                g_LC_ControlState.vbat = atoi(cmd_msg);     //get the vbat in milliamps
                if (g_LC_ControlState.vbat != 0)
                    g_LC_ControlState.vbat = g_LC_ControlState.vbat/1000; //get amps from milliamps
            }
			break;
		default:
			fprintf(stderr, "Unrecognised command string: %s \n", str);
			break;
	}


}

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

	g_LC_ControlState.vbat = 0;
	g_LC_ControlState.motorAmps[0] = 1;
	g_LC_ControlState.motorAmps[1] = 1;
	g_LC_ControlState.motorAmps[2] = 1;
	g_LC_ControlState.motorAmps[3] = 1;
	g_LC_ControlState.motorAmps[4] = 1;
	g_LC_ControlState.motorAmps[5] = 1;
	g_LC_ControlState.ConsoleHealthy = false;
	g_LC_ControlState.DirForward = false;
	g_LC_ControlState.DirReverse = false;
	g_LC_ControlState.DynBrakeActive = false;
	g_LC_ControlState.DynBrakePos = 0;
	g_LC_ControlState.ThrottleActive = false;
	g_LC_ControlState.ThrottlePos = -1;
	g_LC_ControlState.Emergency = false;
	g_LC_ControlState.HornPressed = false;
	g_LC_ControlState.speed = 0;

}

/*********************************************
 *
 * InitModules
 *
 * initializes the SDL Library and various modules of the application
 *
 *********************************************/
int initModules()
{
    addMessageLine("Winter Creek Loco Sound (c) 2017 Initializing.....");

    #ifdef _WIN32
        SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING,"1");
    #endif // _WIN32

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "Unable to initialize SDL:  %s\n", SDL_GetError());
		return 1;
	}

    SDL_version linked;
    SDL_GetVersion(&linked);

    snprintf(m_msgTempLine,MSG_RECT_LINE_LENGTH,"Using SDL ver:%d.%d.%d",linked.major, linked.minor, linked.patch);
    addMessageLine(m_msgTempLine);

	atexit(closeProgram);  //setup the closedown callback


	if(initScreen() != 0)
	{
		fprintf(stderr, "Initalising Screen failed, program terminated\n");
		exit(1);    //error setting up
	}
    addMessageLine("Screen & Fonts Initialized....OK");
	if(initAudio() != 0)
	{
		fprintf(stderr, "Initalising Audio failed, program terminated\n");
		exit(1);    //error setting up
	}

    addMessageLine("Audio Generator & Mixer Initialized....OK");


#ifdef linux
	if (initSerial() != 0)    //not strictly SDL but safe place to put it
	{
		fprintf(stderr,"Warning: Error initializing comms to Loco Control Stand. Keyboard Control Only Mode\n");
	}
    addMessageLine("Comms Initialized....OK");

#endif
#ifdef _WIN32
        addMessageLine("Comms currently disabled for windows version");
#endif // _WIN32

	return 0; //success

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

	fprintf(stderr,"about to close SDL\n");
	SDL_Quit();
	fprintf(stderr,"Program Closed OK\n");

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
			case 'd':           //debug flag
				g_Debug = true;
				fprintf(stderr,"Debug Option Set\n");
				break;
			case 'p':           //path to Data files specified
				strcpy(g_DataFilePath,optarg);
				fprintf (stderr, "Data filepath set to %s.\n", g_DataFilePath);
				break;
			case 'h':
				fprintf(stdout,"Usage: -d sets debug mode, -p <pathname> sets the path for the data files");
				break;
			case '?':
				if (optopt == 'p')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else
					fprintf (stderr, "Unknown option -%c ignored\n", optopt);
				break;

			default: /* Code when there are no parameters */
				break;
		}
	}

}

