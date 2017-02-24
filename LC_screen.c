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
			if(g_LC_ControlState.ThrottlePos == -1)
				updateButton(&m_startBtn);					//only want to show this button if the engine is not yet started.
			updateMotorGraph(&m_MotorGraph[0],50);
			updateMotorGraph(&m_MotorGraph[1],40);
			updateMotorGraph(&m_MotorGraph[2],30);
			updateMotorGraph(&m_MotorGraph[3],60);
			updateMotorGraph(&m_MotorGraph[4],70);
			updateMotorGraph(&m_MotorGraph[5],50);

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

	//Init Globals

	m_window = NULL;
	m_renderer = NULL;
	m_background = NULL;
	m_screenWidth = atoi(getConfigVal("SCREEN_WIDTH"));
	m_screenHeight = atoi(getConfigVal("SCREEN_HEIGHT"));

	atexit(closeScreen);  //setup exit disposal of memory hungry objects and resources

	// Create an application window with the following settings:
	m_window = SDL_CreateWindow(
								"Loco Control",                    // window title
								SDL_WINDOWPOS_UNDEFINED,           // initial x position
								SDL_WINDOWPOS_UNDEFINED,           // initial y position
								m_screenWidth,                     // width, in pixels
								m_screenHeight,                               // height, in pixels
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
		initButton(&m_startBtn,"START_BTN.BMP",35,100,150,150,"S");


		//Now initialize the fonts we wish to use
		if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
		}

		m_MsgFont = TTF_OpenFont( LC_FONT_FILE, 12); //this opens a font style and sets a point size
		if (m_MsgFont == NULL) {
			fprintf(stderr,"Could not open message font: %s\n",SDL_GetError());
		}

		m_BigFont = TTF_OpenFont( LC_FONT_FILE, 40); //this opens a font style and sets a point size
		if (m_BigFont == NULL) {
			fprintf(stderr,"Could not open big font: %s\n",SDL_GetError());
		}

		//todo - testing. move somewhere else
        const SDL_Rect	Message_rect  = {486,406,500,12};	  //Rectangle for text messages
		renderText("initializing - please wait", m_MsgFont, LC_LIGHTGRAY,Message_rect);

		initMotorGraph();    //todo - tidy this up - placeholder for testing

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

	initBarGraph(&m_MotorGraph[0], 80 ,270,280,30,1,LC_ALMOND,LC_GREEN);
	initBarGraph(&m_MotorGraph[1], 142,270,280,30,1,LC_ALMOND,LC_GREEN);
	initBarGraph(&m_MotorGraph[2], 200,270,280,30,1,LC_ALMOND,LC_GREEN);
	initBarGraph(&m_MotorGraph[3], 258,270,280,30,1,LC_ALMOND,LC_GREEN);
	initBarGraph(&m_MotorGraph[4], 320,270,280,30,1,LC_ALMOND,LC_GREEN);
	initBarGraph(&m_MotorGraph[5], 382,270,280,30,1,LC_ALMOND,LC_GREEN);

}

/*****************************
 *
 *  UpdateMotorGraphSet
 *
 *****************************/
void updateMotorGraphSet(void)
{
	//updates each Motor's graph from global value IF IT HAS CHANGED
	//Selects RED as bar colour for drawing from motor, GREEN if battery is being charged

	static int	lastm1 = 0,		//how we keep track of any changes since our last update
				lastm2 = 0,
				lastm3 = 0,
				lastm4 = 0,
				lastm5 = 0,
				lastm6 = 0;

	if(g_LC_ControlState.ampsM1 != lastm1)
	{
			updateMotorGraph(&m_MotorGraph[0],g_LC_ControlState.ampsM1);
			lastm1 = g_LC_ControlState.ampsM1;
	}
	if(g_LC_ControlState.ampsM2 != lastm2)
	{
			updateMotorGraph(&m_MotorGraph[1],g_LC_ControlState.ampsM2);
			lastm1 = g_LC_ControlState.ampsM1;
	}
	if(g_LC_ControlState.ampsM3 != lastm3)
	{
			updateMotorGraph(&m_MotorGraph[2],g_LC_ControlState.ampsM3);
			lastm1 = g_LC_ControlState.ampsM1;
	}
	if(g_LC_ControlState.ampsM4 != lastm4)
	{
			updateMotorGraph(&m_MotorGraph[3],g_LC_ControlState.ampsM4);
			lastm1 = g_LC_ControlState.ampsM1;
	}
	if(g_LC_ControlState.ampsM5 != lastm5)
	{
			updateMotorGraph(&m_MotorGraph[4],g_LC_ControlState.ampsM5);
			lastm1 = g_LC_ControlState.ampsM1;
	}
	if(g_LC_ControlState.ampsM6 != lastm6)
	{
			updateMotorGraph(&m_MotorGraph[5],g_LC_ControlState.ampsM6);
			lastm1 = g_LC_ControlState.ampsM1;
	}
}

/*****************************
 *
 *  UpdateMotorGraph
 *
 *****************************/
void updateMotorGraph(LC_BarGraph_t* graph,int percent)
{
	//determines if the bar should be shown in red (discharge) or green (charging)

	if(percent < 0)
		updateBarGraph(graph,percent,LC_GREEN);
	else
		updateBarGraph(graph,percent,LC_RED);

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
 *
 * Update a Bar Graph
 *
 *
 ****************************/
void updateBarGraph(LC_BarGraph_t* graph, int percent, const SDL_Color barColour)
{

	//takes an already initialised bar graph and redraws it to specified bar height and bar colour

	//redraw background
	SDL_SetRenderDrawColor( m_renderer, graph->backColour.r, graph->backColour.g, graph->backColour.b, graph->backColour.a );
	SDL_RenderFillRect( m_renderer, &graph->background);

	//update bar size and colour
	graph->barColour = barColour;
	graph->bar.h = graph->onePercent * abs(percent);
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
    static const SDL_Rect	Throttle_rect = {954,146,25,25};    //custom numbers to match bmp

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
    static const SDL_Rect	Dynamic_rect = {954,226,25,25};    //custom numbers to match bmp

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
    static const SDL_Rect	Dynamic_rect = {794,315,25,25};    //custom numbers to match bmp

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
		renderText("NEUTRAL", m_BigFont, LC_RED,Dynamic_rect);
	}
}

/*****************************
 *
 *  Update
 *
 *****************************/
void updateSpeedo(void)
{
	//todo
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


