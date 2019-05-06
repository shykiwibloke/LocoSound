//
//  LC_screen.h
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015-2019. All rights reserved.
//
//  VERSION 2.0.0 released 24/04/2019

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


#define SCREEN_LOGICAL_W 1024           //used for internally laying out the screen
#define SCREEN_LOGICAL_H 600            //irresepcetive of what the screen size really is
#define BANNER_FONT_HEIGHT 60
#define LARGE_FONT_HEIGHT 40
#define SMALL_FONT_HEIGHT 16
#define RECT_BORDER_WIDTH 1

//#NB: Y is up/down and X is left/right for screen coordinates
#define MOTOR_AREA_X 40
#define MOTOR_AREA_Y 190
#define MOTOR_AREA_H 355
#define MOTOR_AREA_W 404
#define MOTOR_BAR0_X 75
#define MOTOR_BAR1_X 135
#define MOTOR_BAR2_X 195
#define MOTOR_BAR3_X 255
#define MOTOR_BAR4_X 315
#define MOTOR_BAR5_X 375
#define MOTOR_BAR_Y  MOTOR_AREA_Y + 40
#define MOTOR_BAR_H  280
#define MOTOR_BAR_W  30
#define MOTOR_BAR_TEXT_Y MOTOR_BAR_Y + MOTOR_BAR_H + 10
#define MOTOR_BAR_TEXT_H 12
#define MOTOR_BAR_TEXT_W 30

#define BANNER_RECT_X 20                   //Users Loco name from the config file
#define BANNER_RECT_Y 50
#define BANNER_RECT_W 400
#define BANNER_RECT_H 100

#define THR_RECT_X MOTOR_AREA_X + MOTOR_AREA_W +20      //Throttle Status rectangle
#define THR_RECT_Y MOTOR_AREA_Y
#define THR_RECT_W 350
#define THR_RECT_H 60

#define REV_RECT_X THR_RECT_X                 //Reverser Status rectangle
#define REV_RECT_Y THR_RECT_Y + THR_RECT_H + 20
#define REV_RECT_W 350
#define REV_RECT_H 60

#define BAT_RECT_X THR_RECT_X                  //Battery voltage rectangle
#define BAT_RECT_Y REV_RECT_Y + REV_RECT_H + 20
#define BAT_RECT_W 350
#define BAT_RECT_H 60

#define AMP_RECT_X THR_RECT_X                  //Amperage voltage rectangle
#define AMP_RECT_Y BAT_RECT_Y + BAT_RECT_H + 20
#define AMP_RECT_W 350
#define AMP_RECT_H 60

#define VER_RECT_X THR_RECT_X +40                 //Version  rectangle
#define VER_RECT_Y AMP_RECT_Y + AMP_RECT_H + 30
#define VER_RECT_W 300
#define VER_RECT_H 30

#define START_BTN_X THR_RECT_X + THR_RECT_W + 20    //Engine Start/Stop button
#define START_BTN_Y MOTOR_AREA_Y
#define START_BTN_W 160
#define START_BTN_H 60

#define VOL_AREA_X START_BTN_X
#define VOL_AREA_Y REV_RECT_Y
#define VOL_AREA_W START_BTN_W
#define VOL_AREA_H MOTOR_AREA_H - START_BTN_H - 20

#define VOL_FULL_BTN_X START_BTN_X + 10                  //Select FULL NOISE Button
#define VOL_FULL_BTN_Y REV_RECT_Y + 40
#define VOL_FULL_BTN_W START_BTN_W - 20
#define VOL_FULL_BTN_H START_BTN_H

#define VOL_HALF_BTN_X START_BTN_X + 10                 //Select FULL NOISE Button
#define VOL_HALF_BTN_Y VOL_FULL_BTN_Y + VOL_FULL_BTN_H + 10
#define VOL_HALF_BTN_W START_BTN_W - 20
#define VOL_HALF_BTN_H START_BTN_H

#define VOL_OFF_BTN_X START_BTN_X + 10                  //Select FULL NOISE Button
#define VOL_OFF_BTN_Y VOL_HALF_BTN_Y + VOL_HALF_BTN_H + 10
#define VOL_OFF_BTN_W START_BTN_W - 20
#define VOL_OFF_BTN_H START_BTN_H

#define BTN_TEXT_LEN 15

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

typedef struct {
	SDL_Rect		rect;			//x,y,w,h of the button
    SDL_Color       buttonColour;
    SDL_Color       textColour;
    char            text[BTN_TEXT_LEN];
    SDL_Color       buttonPressedColour;
    SDL_Color       textPressedColour;
    char            textPressed[BTN_TEXT_LEN];
    bool            IsPressed;
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

#define LC_MSGLINE_LEN 254
typedef char  LC_MsgLine_t[LC_MSGLINE_LEN];

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
TTF_Font*               m_bannerFont;               //For Headline only
SDL_Rect                m_msgArea;                  //the rectangle used to display messages
SDL_Rect                m_motorArea;                //the rectangle used to display the motor graph
LC_MsgLine_t            m_msgTempLine;              //A temporary buffer for assembling a message line with variables
int                     m_msgPtr;                   //Pointer used in update and display refresh operations
LC_BarGraph_t			m_motorGraph[6];	        //Array to hold details of the 6 motor graphs
LC_Button_t			    m_startBtn;			        //Engine start/stop button & related variables
LC_Button_t             m_volumeFullBtn;            //When pressed Engine sounds at full config values
LC_Button_t             m_volumeHalfBtn;            //When pressed Engine sounds at half config values
LC_Button_t             m_volumeOffBtn;             //When pressed Engine sounds muted - but not Horn!
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
void            initMotorGraph(void);
void            initBarGraph(LC_BarGraph_t* graph,const int xpos,const int ypos,const int height,const int width,const int border,const int labelx,const int labely,const int labelw,const int labelh,const SDL_Color backColour, const SDL_Color barColour);
void            initButton(LC_Button_t* button,
                           const SDL_Color buttonColour,
                           const SDL_Color textColour,
                           const char *text,
                           const SDL_Color buttonPressedColour,
                           const SDL_Color textPressedColour,
                           const char * pressedText,
                           const bool IsPressed,
                           const int xpos,
                           const int ypos,
                           const int height,
                           const int width,
                           const char *cmd );
void            initVolumeControls(void);
void            updateMotorGraphSet(void);
void            updateBarGraph(LC_BarGraph_t* graph,const int motorAmps, const SDL_Color barColour);
void            updateButton(LC_Button_t* button);
void            updateBanner(void);
void            updateThrottle(void);
void            updateReverser(void);
void            updateBattery(void);
void            updateAmperage(void);
void            renderText(const char* text, TTF_Font* font, const SDL_Color colour, SDL_Rect Message_rect);
void            renderSquare(const SDL_Rect* coords, const SDL_Color lineColour, const SDL_Color fillColour);
void            addMessageLine(const char* msgline);
SDL_Texture*    loadTextureFromBMP(SDL_Renderer* renderer,const char* fileName);



#endif /* defined(__LocoControl__LC_screen__) */
