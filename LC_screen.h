//
//  LC_screen.h
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//
//  VERSION 1.0.2 released 11/04/2017

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
    #define TTF_VERT_OFFSET 0
#endif // linux
#ifdef WIN32
    #define TTF_VERT_OFFSET -11          //windows seems to offset the font downwards by a few pixels compared to Linux - compensate
#endif // windows

#define SCREEN_LOGICAL_W 1024           //used for internally laying out the screen
#define SCREEN_LOGICAL_H 600            //irresepcetive of what the screen size really is

#define MOTOR_AREA_X 40
#define MOTOR_AREA_Y 230
#define MOTOR_AREA_H 355
#define MOTOR_AREA_W 404
#define MOTOR_BAR0_X 75
#define MOTOR_BAR1_X 135
#define MOTOR_BAR2_X 195
#define MOTOR_BAR3_X 255
#define MOTOR_BAR4_X 315
#define MOTOR_BAR5_X 375
#define MOTOR_BAR_Y  270
#define MOTOR_BAR_H  280
#define MOTOR_BAR_W  30
#define MOTOR_BAR_B  1                  //Border width of 1
#define MOTOR_BAR_TEXT_Y MOTOR_BAR_Y + MOTOR_BAR_H + 10 + TTF_VERT_OFFSET //Text vert offset from BAR Y
#define MOTOR_BAR_TEXT_H 12
#define MOTOR_BAR_TEXT_W 30

#define MSG_RECT_X 10                  //Message rectangle coords in logical screen units
#define MSG_RECT_Y 120
#define MSG_RECT_H 130
#define MSG_RECT_LINES 16               //Max number of lines that will fit on the screen
#define MSG_RECT_LINE_LENGTH 80         //max number of characters that will fit on one line
#define MSG_RECT_FONT_HEIGHT 22
#define MSG_RECT_LINE_HEIGHT 28

#define LARGE_FONT_HEIGHT 40

#define DYN_RECT_X 954                  //Dynamic Control rectangle
#define DYN_RECT_Y 226 + TTF_VERT_OFFSET

#define THR_RECT_X 954                  //Throttle Control rectangle
#define THR_RECT_Y 146 + TTF_VERT_OFFSET

#define REV_RECT_X 794                  //Reverser Control rectangle
#define REV_RECT_Y 315 + TTF_VERT_OFFSET

#define BAT_RECT_X 236                  //Battery voltage rectangle
#define BAT_RECT_Y 180 + TTF_VERT_OFFSET

#define BANNER_RECT_X 260                   //Users Loco name from the config file
#define BANNER_RECT_Y 80 + TTF_VERT_OFFSET

//Basic Colour Definitions

static const SDL_Color LC_BLACK		  =	{0x00,0x00,0x00,0xff};
static const SDL_Color LC_DARK_GRAY	  =	{0x55,0x55,0x55,0xff};
static const SDL_Color LC_GRAY		  =	{0x80,0x80,0x80,0xff};
static const SDL_Color LC_LIGHT_GRAY  =	{0xaa,0xaa,0xaa,0xff};
static const SDL_Color LC_WHITE		  =	{0xff,0xff,0xff,0xff};
static const SDL_Color LC_RED		  =	{0xff,0x00,0x00,0xff};
static const SDL_Color LC_LIGHT_GREEN = {0x8a,0xfa,0x0a,0xff};
static const SDL_Color LC_DARK_GREEN  =	{0x00,0x64,0x00,0xff};
static const SDL_Color LC_BLUE		  =	{0x00,0x00,0xff,0xff};
static const SDL_Color LC_CYAN		  =	{0x00,0xff,0xff,0xff};
static const SDL_Color LC_YELLOW	  =	{0xff,0xff,0x00,0xff};
static const SDL_Color LC_MAGENTA	  =	{0xff,0x00,0xff,0xff};
static const SDL_Color LC_ORANGE	  =	{0xff,0x80,0x00,0xff};
static const SDL_Color LC_PURPLE	  =	{0x80,0x00,0x80,0xff};
static const SDL_Color LC_BROWN		  =	{0x99,0x66,0x33,0xff};
static const SDL_Color LC_ALMOND	  = {0xff,0xde,0xad,0xff};

typedef enum {
    MODE_GRAPHIC,                   //Screen is currently showing graphics (default)
    MODE_DIAGNOSTIC,                       //Screen is currently showing system messages
    MODE_CONFIG,                    //System is currently displaying config options/info
} LC_ScreenMode_t;

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
	SDL_Rect		background;		//x,y,w,h of the bars background
    SDL_Rect        label;          //where the text label should be
	SDL_Color		backColour;		//Colour of the background
	SDL_Color		barColour;		//Colour of the bar on the bargraph
} LC_BarGraph_t;

typedef char  LC_MsgLine_t[MSG_RECT_LINE_LENGTH];

/**********************************************
 *
 *  Module Variable Declarations
 *
 **********************************************/

SDL_Window*				m_mainWindow;               //Pointer to the main graphic window
SDL_Renderer*			m_mainRenderer;		        //main screen renderer used by all graphics objects
SDL_Texture*			m_background;		        //background graphics for the main screen
TTF_Font*				m_msgFont;		            //The font we use for all status messages
TTF_Font*				m_bigFont;			        //Large font for throttle setting etc
LC_ScreenMode_t         m_screenMode;               //The current mode of the screen
SDL_Rect                m_msgArea;                  //the rectangle used to display messages
SDL_Rect                m_motorArea;                //the rectangle used to display the motor graph
LC_MsgLine_t            m_msgBuf[MSG_RECT_LINES];   //buffer is the size of displayable lines
LC_MsgLine_t            m_msgTempLine;              //A temporary buffer for assembling a message line with variables
int                     m_msgPtr;                   //Pointer used in update and display refresh operations
LC_BarGraph_t			m_motorGraph[6];	        //Array to hold details of the 6 motor graphs
LC_Button_t			    m_startBtn;			        //graphic for the engine start button & related variables
LC_Button_t             m_menuBtn;                  //graphic for the diagnostic screen mode button
int                     m_maxAmps;                  //Maximum amperage per motor - used to determine 100% for bar graph
float                   m_onePercentAmps;           //One percent of the max amperage
/*****************************************
 *
 * Function Prototypes
 *
 *****************************************/

int             initScreen(void);
void            closeScreen(void);
void            screenService(void);
void            changeScreenMode(void);
void            initMessageWindow(void);
void            initMotorGraph(void);
void            initBarGraph(LC_BarGraph_t* graph,const int xpos,const int ypos,const int height,const int width,const int border,const int labelx,const int labely,const int labelw,const int labelh,const SDL_Color backColour, const SDL_Color barColour);
int             initButton(LC_Button_t* button,const char * BMPfilename,const int xpos,const int ypos,const int height,const int width, const char *cmd );
void            updateMessageWindow(void);
void            updateMotorGraphSet(void);
void            updateBarGraph(LC_BarGraph_t* graph,const int motorAmps, const SDL_Color barColour);
void            updateButton(LC_Button_t* button);
void            updateBanner(void);
void            updateThrottle(void);
void            updateDynamic(void);
void            updateReverser(void);
void            updateSpeedo(void);
void            updateBattery(void);
void            renderText(const char* text, TTF_Font* font, const SDL_Color colour, SDL_Rect Message_rect);
void            renderSquare(const SDL_Rect* coords, const SDL_Color lineColour, const SDL_Color fillColour);
void            addMessageLine(const char* msgline);
void            clearMessageWindow(void);
SDL_Texture*    loadTextureFromBMP(SDL_Renderer* renderer,const char* fileName);



#endif /* defined(__LocoControl__LC_screen__) */
