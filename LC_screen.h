//
//  LC_screen.h
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//

#ifndef __LocoControl__LC_screen__
#define __LocoControl__LC_screen__

#include <stdbool.h>     //Required so Mac is compatible with Raspi
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "LC_utilities.h"
#include "LC_mouseHandler.h"
#include "LC_configReader.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "LC_globals.h"     //definitions that are common across all MPU's in the Loco Sound system

// Font constants
//x,y,w,h
#ifdef linux
    #define TTF_VERT_OFFSET
#endif // linux
#ifdef WIN32
    #define TTF_VERT_OFFSET -11          //windows seems to offset the font downwards by a few pixels
#endif // windows

#define DYN_RECT_X 954
#define DYN_RECT_Y 226 + TTF_VERT_OFFSET

#define THR_RECT_X 954
#define THR_RECT_Y 146 + TTF_VERT_OFFSET

#define REV_RECT_X 794
#define REV_RECT_Y 315 + TTF_VERT_OFFSET

#define BAT_RECT_X 234
#define BAT_RECT_Y 173 + TTF_VERT_OFFSET

//Basic Colour Definitions

static const SDL_Color LC_BLACK		=	{0x00,0x00,0x00,0xff};
static const SDL_Color LC_DARK_GRAY	=	{0x55,0x55,0x55,0xff};
static const SDL_Color LC_GRAY		=	{0x80,0x80,0x80,0xff};
static const SDL_Color LC_LIGHTGRAY	=	{0xaa,0xaa,0xaa,0xff};
static const SDL_Color LC_WHITE		=	{0xff,0xff,0xff,0xff};
static const SDL_Color LC_RED		=	{0xff,0x00,0x00,0xff};
static const SDL_Color LC_GREEN		=	{0x00,0x64,0x00,0xff};
static const SDL_Color LC_BLUE		=	{0x00,0x00,0xff,0xff};
static const SDL_Color LC_CYAN		=	{0x00,0xff,0xff,0xff};
static const SDL_Color LC_YELLOW	=	{0xff,0xff,0x00,0xff};
static const SDL_Color LC_MAGENTA	=	{0xff,0x00,0xff,0xff};
static const SDL_Color LC_ORANGE	=	{0xff,0x80,0x00,0xff};
static const SDL_Color LC_PURPLE	=	{0x80,0x00,0x80,0xff};
static const SDL_Color LC_BROWN		=	{0x99,0x66,0x33,0xff};
static const SDL_Color LC_ALMOND	=   {0xff,0xde,0xad,0xff};

typedef struct {
	SDL_Rect		rect;			//x,y,w,h of the button
	SDL_Texture* 	image;			//bitmap or gif to display
	bool			pressed;	    //True if this button is 'on'

} LC_Button_t;

typedef struct {
	int				barmax;			//max height the bar can be (100%)
	float			onePercent;		//number of pixels that make up one percent
	int				anchor;			//used to xlate coords so bar anchored to same place
	SDL_Rect		bar;			//x,y,w,h of the actual bar of the bargraph
	SDL_Rect		background;		//x,y,w,h of the actual bar of the bargraph
	SDL_Color		backColour;		//Colour of the background
	SDL_Color		barColour;		//Colour of the bar on the bargraph
} LC_BarGraph_t;

/**********************************************
 *
 *  Module Variable Declarations
 *
 **********************************************/

LC_BarGraph_t			m_MotorGraph[6];	//Array to hold details of the 6 motor graphs
SDL_Window*				m_window;           //Pointer to the SDL window
SDL_Renderer*			m_renderer;			//main screen renderer used by all graphics objects
SDL_Texture*			m_background;		//background graphics for the main screen
LC_Button_t			    m_startBtn;			//graphic for the engine start button & related variables
TTF_Font*				m_MsgFont;		    //The font we use for all status messages
TTF_Font*				m_BigFont;			//Large font for throttle setting etc
int                     m_maxAmps;          //Maximum amperage per motor - used to determine 100% for bar graph
float                   m_onePercentAmps;   //One percent of the max amperage
/*****************************************
 *
 * Function Prototypes
 *
 *****************************************/

int  initScreen(void);
void closeScreen(void);
void screenService(void);
void initMotorGraph(void);
void initBarGraph(LC_BarGraph_t* graph, int xpos, int ypos, int height, int width, int border,  const SDL_Color backColour, const SDL_Color barColour);
int  initButton(LC_Button_t* button, const char * BMPfilename, int xpos, int ypos, int height, int width, const char *cmd );
void updateMotorGraphSet(void);
void updateBarGraph(LC_BarGraph_t* graph, int milliamps, const SDL_Color barColour);
void updateButton(LC_Button_t* button);
void updateThrottle(void);
void updateDynamic(void);
void updateReverser(void);
void updateSpeedo(void);
void updateBattery(void);
void renderText(const char* text, TTF_Font* font, const SDL_Color colour, SDL_Rect Message_rect);
SDL_Texture* loadTextureFromBMP(SDL_Renderer* renderer,const char* fileName);


#endif /* defined(__LocoControl__LC_screen__) */
