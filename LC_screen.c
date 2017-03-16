//
//  LC_screen.c
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//

#include "LC_screen.h"

/*********************************************
 *
 * ScreenService
 *
 * Updates the screen every second...
 *
 *********************************************/
void screenService(void)
{
	//Regular servicing of the screen - but call only every second or so.

	if (m_window)
	{

		if(m_renderer)
		{
			SDL_RenderClear(m_renderer);

            // render background, NULL for source and destination rectangles just means "use the default"
            SDL_RenderCopy(m_renderer, m_background, NULL, NULL);

			updateThrottle();
			updateDynamic();
			updateReverser();
			updateSpeedo();
			updateBattery();
			if(g_LC_ControlState.ThrottlePos == -1)
				updateButton(&m_startBtn);					//only want to show this button if the engine is not yet started.
            else
                updateMotorGraphSet();

			SDL_RenderPresent(m_renderer);		//go refresh the screen and return
		}
	}
}

/*********************************************
*
* InitScreen
*
* initializes the screen, shows the splash screen and sets to default configuration
*
*********************************************/
int initScreen()
{

	//Init Module Variables

	m_window = NULL;
	m_renderer = NULL;
	m_background = NULL;


	atexit(closeScreen);  //setup exit disposal of memory hungry objects and resources

	// Create an application window with the following settings:
	m_window = SDL_CreateWindow(
								"Loco Control",                    // window title
								SDL_WINDOWPOS_UNDEFINED,           // initial x position
								SDL_WINDOWPOS_UNDEFINED,           // initial y position
								getConfigVal("SCREEN_WIDTH"),                     // width, in pixels
								getConfigVal("SCREEN_HEIGHT"),                               // height, in pixels
								SDL_WINDOW_SHOWN
								//SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE  // flags - see below
								);

	// Check that the window was successfully created
	if (m_window == NULL)
    {
		// In the event that the window could not be made...
		fprintf(stderr,"Could not create window: %s\n", SDL_GetError());
		return 1;
	}
	else
    {

        //SDL_SetWindowFullscreen(m_window,SDL_WINDOW_FULLSCREEN_DESKTOP);

		//Create renderer for window
		m_renderer = SDL_CreateRenderer( m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE );
		if( m_renderer == NULL )
		{
			fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
			return 1;
		}

		//Initialize default renderer colour
        SDL_SetRenderDrawColor( m_renderer, 0xFF, 0xFF, 0xFF, 0xFF );


		//Now load the background image
		m_background = loadTextureFromBMP(m_renderer, "BACKGROUND.BMP");

		//Load any buttons or other overlay images here
		initButton(&m_startBtn,"START_BTN.BMP",180,340,150,150,"s");    //commands must be lower case!


		//Now initialize the fonts we wish to use
		if(TTF_Init()==-1)
        {
			printf("Error Initializing SDL True Type Font Module: %s\n", TTF_GetError());
			exit(2);
		}

        const SDL_version *link_version=TTF_Linked_Version();
        printf("SDL_ttf opened OK version: %d.%d.%d\n", link_version->major, link_version->minor, link_version->patch);

        //Now open the fonts we need
		m_MsgFont = TTF_OpenFont( getConfigStr("FONT_FILE"), 12); //this opens a font style and sets a point size
		if (m_MsgFont == NULL)
        {
			fprintf(stderr,"Could not open message font: %s\n",SDL_GetError());
		}

		m_BigFont = TTF_OpenFont( getConfigStr("FONT_FILE"), 40); //this opens a font style and sets a point size
		if (m_BigFont == NULL)
        {
			fprintf(stderr,"Could not open big font: %s\n",SDL_GetError());
		}


		//todo - testing. move somewhere else
        const SDL_Rect	Message_rect  = {486,406,500,12};	  //Rectangle for text messages
		renderText("initializing - please wait", m_MsgFont, LC_LIGHTGRAY,Message_rect);

		initMotorGraph();
	}

	return 0;   //success

}

/*****************************
 *
 * closeScreen()
 *
 *****************************/

void closeScreen(void)
{
	//Called automatically at exit DO NOT CALL directly FROM YOUR CODE

	SDL_DestroyWindow(m_window);
	SDL_DestroyRenderer(m_renderer);

	fprintf(stderr,"Screen Closed OK\n");

}

/*****************************
 *
 * Create Motor Graph - a collection of graph objects
 *
 *****************************/
void initMotorGraph(void)
{
	//initialize the six motor bar graph. Note this is not painted until next screen refresh

    m_maxAmps = getConfigVal("MAX_AMPS");
    if(m_maxAmps < 1) m_maxAmps = 1;        //just in case some wally sets the value wrong and causes a divide by zero!!
    m_onePercentAmps = (float) (m_maxAmps / 100);

	initBarGraph(&m_MotorGraph[0], 80 ,270,280,30,1,LC_ALMOND,LC_GREEN);
	initBarGraph(&m_MotorGraph[1], 142,270,280,30,1,LC_ALMOND,LC_GREEN);
	initBarGraph(&m_MotorGraph[2], 200,270,280,30,1,LC_ALMOND,LC_GREEN);
	initBarGraph(&m_MotorGraph[3], 258,270,280,30,1,LC_ALMOND,LC_GREEN);
	initBarGraph(&m_MotorGraph[4], 320,270,280,30,1,LC_ALMOND,LC_GREEN);
	initBarGraph(&m_MotorGraph[5], 382,270,280,30,1,LC_ALMOND,LC_GREEN);

}

/*****************************
 *
 *
 * Init a single Bar Graph
 *
 *
 ****************************/
void initBarGraph(LC_BarGraph_t* graph, int xpos, int ypos, int height, int width, int border, const SDL_Color backColour, const SDL_Color barColour)
{

	//creates single vertical bargraph object at the specified coordinates
	//note - the actual graph is not painted on screen until the next screen refresh

	graph->background.x = xpos;
	graph->background.y = ypos;
	graph->background.h = height;
	graph->background.w = width;
	graph->backColour   = backColour;
	graph->barColour    = barColour;

	graph->bar.x       = xpos + border;
	graph->bar.y       = ypos + height - border;
	graph->bar.w       = width - (border * 2);
	graph->bar.h       = height - (border * 2);

	graph->anchor = ypos + height - border;

	graph->barmax = graph->bar.h;
	if (graph->barmax < 100)
	{
		fprintf(stderr,"Bar graph less than 100 pixels high and risks later div/0 - height adjusted");
		graph->barmax = 100;
	}
	graph->onePercent  = (float) graph->barmax / 100;

}

/*****************************
 *
 *  UpdateMotorGraphSet
 *
 *****************************/
void updateMotorGraphSet(void)
{
	//updates each Motor's graph from global value.
	//Selects RED as bar colour for drawing from motor, GREEN if battery is being charged

    int f = 0;

    for(f=0; f<6; f++)
	{
        if(g_LC_ControlState.motorAmps[f] < 0)
            updateBarGraph(&m_MotorGraph[f], g_LC_ControlState.motorAmps[f],LC_GREEN);
        else
            updateBarGraph(&m_MotorGraph[f], g_LC_ControlState.motorAmps[f],LC_RED);
	}
}


/*****************************
 *
 *
 * Update a Bar Graph
 *
 *
 ****************************/
void updateBarGraph(LC_BarGraph_t* graph, int milliamps, const SDL_Color barColour)
{

	//takes an already initialised bar graph and redraws it to specified bar height and bar colour

	//redraw background
	SDL_SetRenderDrawColor( m_renderer, graph->backColour.r, graph->backColour.g, graph->backColour.b, graph->backColour.a );
	SDL_RenderFillRect( m_renderer, &graph->background);

	//update bar size and colour
	graph->barColour = barColour;
	graph->bar.h = (int) graph->onePercent * abs((float)milliamps/m_onePercentAmps);        //calculates percentage times pixels per percent
	graph->bar.y = graph->anchor - graph->bar.h;

	//redraw bar
	SDL_SetRenderDrawColor( m_renderer, barColour.r, barColour.g, barColour.b, barColour.a );
	SDL_RenderFillRect( m_renderer, &graph->bar);

}

/*****************************
 *
 *  Init a button
 *
 *****************************/
int initButton(LC_Button_t* button, const char * BMPfilename, int xpos, int ypos, int height, int width, const char *cmd )
{
	//Loads the specified button structure with the bitmap and positional information
	//returns 0 if OK

	button->rect.x = xpos;
	button->rect.y = ypos;
	button->rect.h = height;
	button->rect.w = width;
	button->image =  loadTextureFromBMP(m_renderer, BMPfilename);

	if(button->image == NULL)
	{
		fprintf(stderr,"Button Image file %s failed to load\n",BMPfilename);
		return 1;
	}

	registerCommand(&button->rect, cmd);			//tell the mouse handler about our button and what command
	return 0;
}

/*****************************
 *
 *  Update a button
 *
 *****************************/
void updateButton(LC_Button_t* button)
{
	SDL_RenderCopy(m_renderer, button->image, NULL, &button->rect);
}

/*****************************
 *
 *  Update Throttle Notch
 *
 *****************************/
void updateThrottle(void)
{
    static const SDL_Rect	Throttle_rect = {THR_RECT_X,THR_RECT_Y,25,25};    //custom numbers to match bmp

    if(g_LC_ControlState.ThrottlePos > -1)
    {
        char str[2] = {'0' + g_LC_ControlState.ThrottlePos,0};
        renderText(str, m_BigFont, LC_BLACK, Throttle_rect);
    }
}

/*****************************
 *
 *  Update
 *
 *****************************/
void updateDynamic()
{

    static const SDL_Rect	Dynamic_rect = {DYN_RECT_X,DYN_RECT_Y,25,25};    //custom numbers to match bmp

    if(g_LC_ControlState.DynBrakePos > -1)
    {
        char str[2] = {'0' + g_LC_ControlState.DynBrakePos,0};
        renderText(str, m_BigFont, LC_BLACK,Dynamic_rect);
    }

}

/*****************************
 *
 *  Update
 *
 *****************************/
void updateReverser(void)
{
    static const SDL_Rect	Dynamic_rect = {REV_RECT_X,REV_RECT_Y,25,25};    //custom numbers to match bmp

	if(g_LC_ControlState.DirForward)
	{
		renderText("FORWARD", m_BigFont, LC_RED,Dynamic_rect);
	}
	else if(g_LC_ControlState.DirReverse)
	{
		renderText("REVERSE", m_BigFont, LC_RED,Dynamic_rect);
	}
	else if(!g_LC_ControlState.DirForward && !g_LC_ControlState.DirReverse)
	{
		renderText("NEUTRAL", m_BigFont, LC_GREEN,Dynamic_rect);
	}
}

/*****************************
 *
 *  Update Speedo
 *
 *****************************/
void updateSpeedo(void)
{
	//TODO
}

/*****************************
 *
 *  Update Battery
 *
 *****************************/
void updateBattery(void)
{
    static const SDL_Rect	Battery_rect = {BAT_RECT_X,BAT_RECT_Y,200,25};    //custom numbers to match bmp

    char str[15];

    sprintf(str,"%.2f volts", g_LC_ControlState.vbat);
    renderText(str, m_BigFont, LC_BLACK, Battery_rect);
}

/*****************************
 *
 *
 * Load specified Bitmap file from the data directory and convert it to a texture
 *
 ****************************/

SDL_Texture* loadTextureFromBMP(SDL_Renderer* renderer, const char* fileName)
{

	//	SDL_Surface* optimizedSurface = NULL;
	SDL_Texture* newTexture = NULL;

	setDataFilePath();   //change directory to the data file path

	SDL_Surface* loadedSurface = SDL_LoadBMP(fileName);

	if (loadedSurface == NULL)
	{

		fprintf(stderr, "Unable to load image %s\n",fileName);

	}

	newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);

	SDL_FreeSurface( loadedSurface );

	return newTexture;


}

void renderText(const char* text, TTF_Font* font, const SDL_Color colour, SDL_Rect Message_rect)
{

	// as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
	SDL_Surface* surfaceMessage = TTF_RenderUTF8_Blended(font, text, colour);

    //now you can convert it into a texture
	SDL_Texture* Message = SDL_CreateTextureFromSurface(m_renderer, surfaceMessage);

	//TODO - have to put RenderCopy in your main loop area, the area where the whole code executes

	Message_rect.w = surfaceMessage->w; // controls the width of the rect
	Message_rect.h = surfaceMessage->h ; // controls the height of the rect

	SDL_RenderCopy(m_renderer, Message, NULL, &Message_rect); //you put the renderer's name first, the Message, the crop size(you can ignore this if you don't want to dabble with cropping), and the rect which is the size and coordinate of your texture

	SDL_FreeSurface(surfaceMessage);
	SDL_DestroyTexture(Message);

}


