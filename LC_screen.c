//
//  LC_screen.c
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015-2019. All rights reserved.
//
//  VERSION 2.0.0 released 24/04/2019

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
    //Regular servicing of the screen contents.

    if (m_mainWindow)
    {

        if(m_mainRenderer)
        {
            updateBanner();
            updateThrottle();
            updateReverser();
            updateBattery();
            updateAmperage();
            updateDynOnIndicator();
            updateMtrsOnIndicator();
            updateButton(&m_startBtn);
            updateButton(&m_volumeFullBtn);
            updateButton(&m_volumeHalfBtn);
            updateButton(&m_volumeOffBtn);
            updateMotorGraphSet();

            SDL_RenderPresent(m_mainRenderer);		//go refresh the screen and return

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

    m_mainWindow = NULL;
    m_mainRenderer   = NULL;
    m_background = NULL;

    Uint32 windowFlags = (SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

    atexit(closeScreen);  //setup exit disposal of memory hungry objects and resources

    //if fullscreen requested then set to fullscreen
    if(strncmp(getConfigStr("SCREEN_MAX"),"YES",3) == 0)
        windowFlags = (windowFlags | SDL_WINDOW_FULLSCREEN_DESKTOP);


    // Create an application window with the following settings:
    m_mainWindow = SDL_CreateWindow(
                       "WCRLocoController",               // window title
                       SDL_WINDOWPOS_UNDEFINED,           // initial x position
                       SDL_WINDOWPOS_UNDEFINED,           // initial y position
                       getConfigVal("SCREEN_WIDTH"),      // width, in pixels
                       getConfigVal("SCREEN_HEIGHT"),     // height, in pixels
                       windowFlags);

    // Check that the window was successfully created
    if (m_mainWindow == NULL)
    {
        // In the event that the window could not be made...
        //note - cant put simple message box here as we dont have a window to put it on!
        fprintf(stderr,"Could not create window:'%s'\n", SDL_GetError());
        logMessage("Could not create window", true);
        return 1;
    }
    else
    {

        //Create renderer for window
        m_mainRenderer = SDL_CreateRenderer( m_mainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE );
        if( m_mainRenderer == NULL )
        {
            fprintf(stderr,"Could not create renderer:'%s'\n", SDL_GetError());
            logMessage("Could not create renderer", true);
            return 1;
        }

        //necessary to esnure all the graphics are placed as intended when we do not know the exact size of the displayed window
        SDL_RenderSetLogicalSize(m_mainRenderer, SCREEN_LOGICAL_W,SCREEN_LOGICAL_H);

        //Initialize default renderer colour
        SDL_SetRenderDrawColor( m_mainRenderer, 0x0, 0x0, 0x0, 0x0 );

        //Now initialize the fonts we wish to use
        if(TTF_Init()==-1)
        {
            printf("Error Initializing SDL True Type Font Module: '%s'\n", TTF_GetError());
            exit(EXIT_FAILURE);
        }

        //Now open the fonts we need from the graphics path
        setFilePath(getConfigStr("GRAPHIC_FILE_PATH"),false);

        m_msgFont = TTF_OpenFont( getConfigStr("FONT_FILE"), SMALL_FONT_HEIGHT); //this opens a font style and sets a point size
        if (m_msgFont == NULL)
        {

            fprintf(stderr,"Could not open small font file: '%s'\n",SDL_GetError());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"FATAL ERROR LOADING SMALL FONT",SDL_GetError(),m_mainWindow);
            return 1;
        }

        m_bigFont = TTF_OpenFont( getConfigStr("FONT_FILE"), LARGE_FONT_HEIGHT); //this opens a font style and sets a point size
        if (m_bigFont == NULL)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"FATAL ERROR LOADING BIG FONT",SDL_GetError(),m_mainWindow);
            return 1;
        }

        m_bannerFont = TTF_OpenFont( getConfigStr("FONT_FILE"), BANNER_FONT_HEIGHT); //this opens a font style and sets a point size
        if (m_bannerFont == NULL)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"FATAL ERROR LOADING BANNER FONT",SDL_GetError(),m_mainWindow);
            return 1;
        }
        initMotorGraph();

        //Load any buttons or other overlay images that go on all screen modes here
        initButton(&m_startBtn,LC_DARK_GREEN,LC_WHITE," START",LC_RED,LC_WHITE,"  STOP ",false,START_BTN_X,START_BTN_Y,START_BTN_H,START_BTN_W,"s");    //commands must be lower case!

        initVolumeControls();

        //Set out the motor graph area
        renderSquare(&m_motorArea,LC_WHITE,LC_DARK_GRAY);   //create the background
        renderText("  Motor Amperages ",m_msgFont,LC_WHITE,m_motorArea);

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

    SDL_DestroyWindow(m_mainWindow);
    SDL_DestroyRenderer(m_mainRenderer);
}

/*****************************
 *
 * Create Motor Graph - a collection of graph objects
 *
 *****************************/
void initMotorGraph(void)
{
    //initialize the six motor bar graph. Note this is not painted until next screen refresh
    //(see updateMotorGraph)

    m_maxAmps = getConfigVal("MAX_AMPS");
    if(m_maxAmps < 1) m_maxAmps = 1;        //just in case some wally sets the value wrong and causes a divide by zero!!
    m_onePercentAmps = (float) m_maxAmps / 100;


    m_motorArea.x = MOTOR_AREA_X;
    m_motorArea.y = MOTOR_AREA_Y;
    m_motorArea.h = MOTOR_AREA_H;
    m_motorArea.w = MOTOR_AREA_W;

    initBarGraph(&m_motorGraph[0], MOTOR_BAR0_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,RECT_BORDER_WIDTH,MOTOR_BAR0_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);
    initBarGraph(&m_motorGraph[1], MOTOR_BAR1_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,RECT_BORDER_WIDTH,MOTOR_BAR1_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);
    initBarGraph(&m_motorGraph[2], MOTOR_BAR2_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,RECT_BORDER_WIDTH,MOTOR_BAR2_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);
    initBarGraph(&m_motorGraph[3], MOTOR_BAR3_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,RECT_BORDER_WIDTH,MOTOR_BAR3_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);
    initBarGraph(&m_motorGraph[4], MOTOR_BAR4_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,RECT_BORDER_WIDTH,MOTOR_BAR4_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);
    initBarGraph(&m_motorGraph[5], MOTOR_BAR5_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,RECT_BORDER_WIDTH,MOTOR_BAR5_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);

}

/*****************************
 *
 *
 * Init a single Bar Graph
 *
 *
 ****************************/
void initBarGraph(LC_BarGraph_t* graph,const int xpos,const int ypos,const int height,const int width,const int border,const int labelx,const int labely,const int labelw,const int labelh,const SDL_Color backColour, const SDL_Color barColour)
{

    //creates single vertical bargraph object at the specified coordinates
    //note - the actual graph is not painted on screen until the next screen refresh

    graph->background.x = xpos;
    graph->background.y = ypos;
    graph->background.h = height;
    graph->background.w = width;
    graph->backColour   = backColour;
    graph->barColour    = barColour;
    graph->bar.x        = xpos + border;
    graph->bar.y        = ypos + height - border;
    graph->bar.w        = width - (border * 2);
    graph->bar.h        = height - (border * 2);
    graph->label.x      = labelx;
    graph->label.y      = labely;
    graph->label.w      = labelw;
    graph->label.h      = labelh;

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
 *  Add a line of text to the message area
 *
 *****************************/
void addMessageLine(const char* msgline)
{
    //appends supplied character string to the next available line in the text area
    //if text area full - msgPtr makes the area act like it scrolls

    logMessage(msgline,true);            //log message in the log file (only works if debug option set on startup)

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

    int             f           = 0;
    char            bartext[6]  = {0}; //used to hold each text label as it is created
    float           amps        = 0;
    static float    lastamps[6] = {0};

    for(f=0; f<6; f++)   //iterate through the six graphs updating each and their text
    {

        if(g_LC_ControlState.motorAmps[f] !=0)
            amps = (float) g_LC_ControlState.motorAmps[f]/10; //value is in milliamps + guard against divide/0
        else
            amps = 0;

        if(amps != lastamps[f])         //only update the bar if it has changed
        {

            //are we generating (green) or consuming (red)
            if(g_LC_ControlState.motorAmps[f] < 0)
                updateBarGraph(&m_motorGraph[f], abs(amps),LC_DARK_GREEN);
            else
                updateBarGraph(&m_motorGraph[f], abs(amps),LC_RED);

            snprintf(bartext,5,"%-2.1f",amps);
            renderText(bartext,m_msgFont,LC_WHITE,m_motorGraph[f].label);
        }
    }




}


/*****************************
 *
 *
 * Update a Bar Graph
 *
 *
 ****************************/
void updateBarGraph(LC_BarGraph_t* graph,const int motorAmps, const SDL_Color barColour)
{

    //takes an already initialised bar graph and redraws it to specified bar height and bar colour

    //redraw background
    SDL_SetRenderDrawColor( m_mainRenderer, graph->backColour.r, graph->backColour.g, graph->backColour.b, graph->backColour.a );
    SDL_RenderFillRect( m_mainRenderer, &graph->background);

    //update bar size and colour
    graph->barColour = barColour;

    //check limit guards here
    if (motorAmps != 0)
    {
        graph->bar.h = (int) graph->onePercent * abs((float)motorAmps/m_onePercentAmps);        //calculates percentage times pixels per percent
    } else
    {
        graph->bar.h = 0;
    }

    if (graph->bar.h > graph->background.h)
    {
        graph->bar.h = graph->background.h;
    }

    graph->bar.y = graph->anchor - graph->bar.h;

    //redraw bar
    SDL_SetRenderDrawColor( m_mainRenderer, barColour.r, barColour.g, barColour.b, barColour.a );
    SDL_RenderFillRect( m_mainRenderer, &graph->bar);

}

/*****************************
 *
 *  Init a button
 *
 *****************************/
void initButton(LC_Button_t* button,
               const SDL_Color buttonColour,
               const SDL_Color textColour,
               const char *text,
               const SDL_Color buttonPressedColour,
               const SDL_Color textPressedColour,
               const char *textPressed,
               const bool IsPressed,
               const int xpos,
               const int ypos,
               const int height,
               const int width,
               const char *cmd )
{
    //Loads the specified button structure with the positional information & function to call if pressed

    button->buttonColour = buttonColour;
    button->textColour = textColour;
    snprintf(button->text,BTN_TEXT_LEN,text);
    button->buttonPressedColour = buttonPressedColour;
    button->textPressedColour = textPressedColour;
    snprintf(button->textPressed,BTN_TEXT_LEN,textPressed);
    button->IsPressed = IsPressed;
    button->rect.x = xpos;
    button->rect.y = ypos;
    button->rect.h = height;
    button->rect.w = width;

    registerCommand(&button->rect, cmd);			//tell the mouse handler about our button and what command
    updateButton(button);

}

/*****************************
 *
 *  Update a button
 *
 *****************************/
void updateButton(LC_Button_t* button)
{
    if(button->IsPressed == true)
    {
        renderSquare(&button->rect,LC_WHITE,button->buttonPressedColour);
        renderText(button->textPressed, m_bigFont, button->textPressedColour, button->rect);
    }
    else
    {
        renderSquare(&button->rect,LC_WHITE,button->buttonColour);
        renderText(button->text, m_bigFont, button->textColour, button->rect);
    }


}

/*****************************
 *
 *  init Volume Control Area and controls
 *
 *****************************/

void initVolumeControls(void)
{

        const SDL_Rect volArea = {VOL_AREA_X, VOL_AREA_Y, VOL_AREA_W, VOL_AREA_H};
        renderSquare(&volArea,LC_WHITE,LC_DARK_GRAY);
        renderText("      Volume Control",m_msgFont,LC_WHITE,volArea);

        initButton(&m_volumeFullBtn,LC_GRAY,LC_BLACK," LOUD",LC_YELLOW,LC_DARK_GREEN,"  vol 11",false,VOL_FULL_BTN_X,VOL_FULL_BTN_Y,VOL_HALF_BTN_H,VOL_HALF_BTN_W,"l");

        initButton(&m_volumeHalfBtn,LC_GRAY,LC_BLACK,"   MED",LC_CYAN,LC_BLACK,"  vol 5",true,VOL_HALF_BTN_X,VOL_HALF_BTN_Y,VOL_HALF_BTN_H,VOL_HALF_BTN_W,"m");    //commands must be lower case!

        initButton(&m_volumeOffBtn,LC_GRAY, LC_BLACK," MUTE",LC_RED,LC_WHITE,"   OFF",false,VOL_OFF_BTN_X,VOL_OFF_BTN_Y,VOL_OFF_BTN_H,VOL_OFF_BTN_W,"o");


}

/*****************************
 *
 *  Update Banner
 *
 *****************************/
void updateBanner(void)
{
    static const SDL_Rect	Banner_rect = {BANNER_RECT_X,BANNER_RECT_Y,BANNER_RECT_W,BANNER_RECT_H};
    static const SDL_Rect   Version_rect = {VER_RECT_X,VER_RECT_Y,VER_RECT_W,VER_RECT_H};
    static char bannerText[50] = {0};


    if(bannerText[0] == '\0')       //ensures routine only called once.
    {
        snprintf(bannerText,50,"    %s Locomotive Controller",getConfigStr("LOCO_NAME"));
        renderSquare(&Banner_rect,LC_BLACK,LC_BLACK);
        renderText(bannerText, m_bannerFont, LC_ORANGE, Banner_rect);
        renderSquare(&Version_rect,LC_BLACK,LC_BLACK);
        renderText(PROGRAM_VERSION, m_msgFont, LC_ORANGE, Version_rect);

        registerCommand(&Banner_rect, "x");			//tell the mouse handler about our hidden quit button

    }

}

/*****************************
 *
 *  Update Throttle Notch
 *
 *****************************/
void updateThrottle(void)
{
    static const SDL_Rect	Throttle_rect = {THR_RECT_X,THR_RECT_Y,THR_RECT_W,THR_RECT_H};    //custom numbers to match bmp
    static char throttleText[25] = {0};
    static int lastThrottlePos = 0;
    static int lastDynamicPos = 0;
    static unsigned lastMotorState = 99; //invalid value to ensure first pass works

    if(g_LC_ControlState.ThrottlePos != lastThrottlePos || g_LC_ControlState.DynBrakePos != lastDynamicPos || g_LC_ControlState.MotorState != lastMotorState)
    {
        //only update if something has changed

        lastThrottlePos = g_LC_ControlState.ThrottlePos;
        lastDynamicPos = g_LC_ControlState.DynBrakePos;


        switch(g_LC_ControlState.MotorState)
        {
            case MOTOR_STOPPED:
                renderSquare(&Throttle_rect,LC_WHITE,LC_ORANGE);
                snprintf(throttleText,25,"      OFF LINE");
                renderText(throttleText, m_bigFont, LC_BLACK, Throttle_rect);
                break;

            case MOTOR_STARTING:
                renderSquare(&Throttle_rect,LC_WHITE,LC_DARK_GRAY);
                snprintf(throttleText,25,"       STARTING");
                renderText(throttleText, m_bigFont, LC_WHITE, Throttle_rect);
                break;

            case MOTOR_STOPPING:
                renderSquare(&Throttle_rect,LC_WHITE,LC_DARK_GRAY);
                snprintf(throttleText,25,"       STOPPING");
                renderText(throttleText, m_bigFont, LC_WHITE, Throttle_rect);
                break;

            case MOTOR_RUNNING:
                if(g_LC_ControlState.DynBrakePos > 7)
                {
                    renderSquare(&Throttle_rect,LC_WHITE,LC_RED);
                    snprintf(throttleText,25,"    EMERGENCY");
                    renderText(throttleText, m_bigFont, LC_WHITE, Throttle_rect);
                }
                else if(g_LC_ControlState.DynBrakePos > 0)
                {
                    renderSquare(&Throttle_rect,LC_WHITE,LC_BLUE);
                    snprintf(throttleText,25,"   DYN. BRAKE %d",g_LC_ControlState.DynBrakePos);
                    renderText(throttleText, m_bigFont, LC_WHITE, Throttle_rect);
                }
                else if(g_LC_ControlState.ThrottlePos > 0)
                {
                    renderSquare(&Throttle_rect,LC_WHITE,LC_YELLOW);
                    snprintf(throttleText,25," THROT. NOTCH %d",g_LC_ControlState.ThrottlePos);
                    renderText(throttleText, m_bigFont, LC_BLACK, Throttle_rect);
                } else {
                    renderSquare(&Throttle_rect,LC_WHITE,LC_DARK_GREEN);
                    snprintf(throttleText,25,"    ENGINE IDLE");
                    renderText(throttleText, m_bigFont, LC_WHITE, Throttle_rect);
                }
                break;
        }
    }
}


/*****************************
 *
 *  Update Reverser
 *
 *****************************/
void updateReverser(void)
{
    static const SDL_Rect	Reverser_rect = {REV_RECT_X,REV_RECT_Y,REV_RECT_W,REV_RECT_H};    //custom numbers to match bmp
    static bool lastrenderedFWD = true;     //illogical start condition to ensure initial paint occurs
    static bool lastrenderedREV = true;     //illogical...

    if(g_LC_ControlState.DirForward != lastrenderedFWD || g_LC_ControlState.DirReverse != lastrenderedREV)
    {
        //Only do expensive graphics calls if changed

        lastrenderedFWD = g_LC_ControlState.DirForward;
        lastrenderedREV = g_LC_ControlState.DirReverse;

        if(g_LC_ControlState.DirForward)
        {
           renderSquare(&Reverser_rect,LC_WHITE,LC_YELLOW);
           renderText("       FORWARD", m_bigFont, LC_DARK_GREEN,Reverser_rect);
        }
        else if(g_LC_ControlState.DirReverse)
        {
            renderSquare(&Reverser_rect,LC_WHITE,LC_RED);
            renderText("       REVERSE", m_bigFont, LC_WHITE,Reverser_rect);
        }

        else if(!g_LC_ControlState.DirForward && !g_LC_ControlState.DirReverse)
        {
            renderSquare(&Reverser_rect,LC_WHITE,LC_DARK_GREEN);
            renderText("        NEUTRAL", m_bigFont, LC_WHITE,Reverser_rect);
        }
    }
}

/*****************************
 *
 *  Update Battery
 *
 *****************************/
void updateBattery(void)
{
    static const SDL_Rect	Battery_rect = {BAT_RECT_X,BAT_RECT_Y,BAT_RECT_W,BAT_RECT_H};    //custom numbers to match bmp
    static float lastRenderValue = -1;
    char str[20];

    if(g_LC_ControlState.vbat != lastRenderValue)        //Only render if changed
    {
        snprintf(str,20,"     BATT: %.2fV", g_LC_ControlState.vbat);
        lastRenderValue = g_LC_ControlState.vbat;

        if(g_LC_ControlState.vbat > 23.5)
        {
            renderSquare(&Battery_rect,LC_WHITE,LC_DARK_GREEN);
            renderText(str, m_bigFont, LC_WHITE, Battery_rect);
        }
        else if(g_LC_ControlState.vbat > 22)
        {
            renderSquare(&Battery_rect,LC_WHITE,LC_YELLOW);
            renderText(str, m_bigFont, LC_DARK_GREEN, Battery_rect);
        }
        else
        {
            renderSquare(&Battery_rect,LC_WHITE,LC_RED);
            renderText(str, m_bigFont, LC_WHITE, Battery_rect);
        }
    }
}


/*****************************
 *
 *  Update Total Amperage
 *
 *****************************/
void updateAmperage(void)
{
    static const SDL_Rect	Amperage_rect = {AMP_RECT_X,AMP_RECT_Y,AMP_RECT_W,AMP_RECT_H};    //custom numbers to match bmp
    static float lastRenderValue = -99;
    float f;
    char str[20];

    f = g_LC_ControlState.motorAmps[0]+g_LC_ControlState.motorAmps[1]+g_LC_ControlState.motorAmps[2]+g_LC_ControlState.motorAmps[3]+g_LC_ControlState.motorAmps[4]+g_LC_ControlState.motorAmps[5];
    if (f!= 0)
        f = f/10;

    if(f != lastRenderValue)        //Only render if changed
    {
        lastRenderValue = f;
        snprintf(str,20,"     BATT: %.2fA", f);

        if(f < 0)
        {
            renderSquare(&Amperage_rect,LC_WHITE,LC_DARK_GREEN);
            renderText(str, m_bigFont, LC_WHITE, Amperage_rect);
        }
        else if (f < 10)
        {
            renderSquare(&Amperage_rect,LC_WHITE,LC_BLUE);
            renderText(str, m_bigFont, LC_WHITE, Amperage_rect);
        }
        else if(f < 16)
        {
            renderSquare(&Amperage_rect,LC_WHITE,LC_YELLOW);
            renderText(str, m_bigFont, LC_DARK_GREEN, Amperage_rect);
        }
        else
        {
            renderSquare(&Amperage_rect,LC_WHITE,LC_RED);
            renderText(str, m_bigFont, LC_WHITE, Amperage_rect);
        }
    }
}


/*****************************
 *
 *  Update Dynamic Indicator
 *
 *****************************/
void updateDynOnIndicator(void)
{
    static const SDL_Rect	DynOn_rect = {DYN_ON_RECT_X,DYN_ON_RECT_Y,DYN_ON_RECT_W,DYN_ON_RECT_H};    //custom numbers to match bmp
    const char str[] = "    DYN\0";

    if(g_LC_ControlState.DynamicEnabled == true)
    {
        renderSquare(&DynOn_rect,LC_WHITE,LC_BLUE);
        renderText(str, m_bigFont, LC_WHITE, DynOn_rect);
    }
    else
    {
        renderSquare(&DynOn_rect,LC_DARK_GRAY,LC_DARK_GRAY);
        renderText(str, m_bigFont, LC_WHITE, DynOn_rect);
    }

}
/*****************************
 *
 *  Update Motors On Indicator
 *
 *****************************/
void updateMtrsOnIndicator(void)
{
    static const SDL_Rect	MtrsOn_rect = {MTR_ON_RECT_X,MTR_ON_RECT_Y,MTR_ON_RECT_W,MTR_ON_RECT_H};    //custom numbers to match bmp
    const char str[] = "   MTRS\0";

    if(g_LC_ControlState.MotorsEnabled == true)
    {
        renderSquare(&MtrsOn_rect,LC_WHITE,LC_DARK_GREEN);
        renderText(str, m_bigFont, LC_WHITE, MtrsOn_rect);
    }
    else
    {
        renderSquare(&MtrsOn_rect,LC_BLACK,LC_DARK_GRAY);
        renderText(str, m_bigFont, LC_WHITE, MtrsOn_rect);
    }
}
/*****************************
 *
 *
 * Load specified Bitmap file from the data directory and convert it to a texture
 *
 ****************************/

SDL_Texture* loadTextureFromBMP(SDL_Renderer* renderer, const char* fileName)
{

    SDL_Texture* newTexture = NULL;

    setFilePath(getConfigStr("GRAPHIC_FILE_PATH"),false);

    SDL_Surface* loadedSurface = SDL_LoadBMP(fileName);

    if (loadedSurface == NULL)
    {
        logString("Unable to find or load image file: ",fileName);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"ERROR","Cant load image file. See log for details",m_mainWindow);
    }

    newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);

    SDL_FreeSurface( loadedSurface );

    return newTexture;
}

void renderText(const char* text, TTF_Font* font,const SDL_Color colour, SDL_Rect Message_rect)
{

    if (font == NULL)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"FATAL ERROR using FONT",SDL_GetError(),m_mainWindow);
            logString("Fatal Error attempting to render text ",text);
            exit(EXIT_FAILURE);
        }
    // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
    SDL_Surface* surfaceMessage = TTF_RenderUTF8_Blended(font, text, colour);

    //now you can convert it into a texture
    SDL_Texture* Message = SDL_CreateTextureFromSurface(m_mainRenderer, surfaceMessage);

    Message_rect.w = surfaceMessage->w; // controls the width of the rect
    Message_rect.h = surfaceMessage->h ; // controls the height of the rect

    SDL_RenderCopy(m_mainRenderer, Message, NULL, &Message_rect); //you put the renderer's name first, the Message, the crop size(you can ignore this if you don't want to dabble with cropping), and the rect which is the size and coordinate of your texture

    SDL_DestroyTexture(Message);
    SDL_FreeSurface(surfaceMessage);

}

void renderSquare(const SDL_Rect* coords, const SDL_Color lineColour, const SDL_Color fillColour)
{
    SDL_SetRenderDrawColor(m_mainRenderer,fillColour.r,fillColour.g,fillColour.b,fillColour.a);
    SDL_RenderFillRect(m_mainRenderer,coords);
    SDL_SetRenderDrawColor(m_mainRenderer,lineColour.r,lineColour.g,lineColour.b,lineColour.a);
    SDL_RenderDrawRect(m_mainRenderer,coords);

}

