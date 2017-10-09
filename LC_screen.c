//
//  LC_screen.c
//  LocoControl
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//
//  VERSION 1.0.2 released 11/04/2017

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
    //todo - add a context mode switch to update graphics, messages, config

    if (m_mainWindow)
    {

        if(m_mainRenderer)
        {
            SDL_SetRenderDrawColor( m_mainRenderer, 0x0, 0x0, 0x0, 0x0 );
            SDL_RenderClear(m_mainRenderer);

            // render background, NULL for source and destination rectangles just means "use the default"
            SDL_RenderCopy(m_mainRenderer, m_background, NULL, NULL);

            updateBanner();

            updateButton(&m_menuBtn);


                   //initialize the various sub-sections of the screen area
            switch(m_screenMode)
            {
                case MODE_DIAGNOSTIC:
                    updateMessageWindow();
                    break;
                case MODE_CONFIG:               //todo no config mode for now so allow it to go to graphic/default
                case MODE_GRAPHIC:
                default:
                    updateThrottle();
                    updateDynamic();
                    updateReverser();
                    updateSpeedo();
                    updateBattery();
                    updateButton(&m_startBtn);					//only want to show this button if the engine is not yet started.
                    updateMotorGraphSet();
            }

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
    m_screenMode = MODE_GRAPHIC;

    atexit(closeScreen);  //setup exit disposal of memory hungry objects and resources

    // Create an application window with the following settings:
    m_mainWindow = SDL_CreateWindow(
                       "WCRLocoControllerV1",               // window title
                       SDL_WINDOWPOS_UNDEFINED,           // initial x position
                       SDL_WINDOWPOS_UNDEFINED,           // initial y position
                       getConfigVal("SCREEN_WIDTH"),      // width, in pixels
                       getConfigVal("SCREEN_HEIGHT"),     // height, in pixels
                       SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
                       //| SDL_WINDOW_BORDERLESS
                   );

    // Check that the window was successfully created
    if (m_mainWindow == NULL)
    {
        // In the event that the window could not be made...
        //note - cant put simple message box here as we dont have a window to put it on!
        fprintf(stderr,"Could not create window:'%s'\n", SDL_GetError());
        return 1;
    }
    else
    {

        //if fullscreen requested then set to fullscreen
        if(strncmp(getConfigStr("SCREEN_MAX"),"YES",3) == 0)
            SDL_SetWindowFullscreen(m_mainWindow,SDL_WINDOW_FULLSCREEN_DESKTOP);

        //Create renderer for window
        m_mainRenderer = SDL_CreateRenderer( m_mainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE );
        if( m_mainRenderer == NULL )
        {
            fprintf(stderr,"Could not create renderer:'%s'\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }

        //necessary to esnure all the graphics are placed as intended when we do not know the exact size of the displayed window
        SDL_RenderSetLogicalSize(m_mainRenderer, SCREEN_LOGICAL_W,SCREEN_LOGICAL_H);

        //Initialize default renderer colour
        SDL_SetRenderDrawColor( m_mainRenderer, 0x0, 0x0, 0x0, 0x0 );


        //Now load the background image into buffer
        m_background = loadTextureFromBMP(m_mainRenderer, "BACKGROUND.BMP");

        //Now initialize the fonts we wish to use
        if(TTF_Init()==-1)
        {
            printf("Error Initializing SDL True Type Font Module: '%s'\n", TTF_GetError());
            exit(EXIT_FAILURE);
        }

        const SDL_version *link_version=TTF_Linked_Version();
        printf("SDL_ttf opened OK version: %d.%d.%d\n", link_version->major, link_version->minor, link_version->patch);

        //Now open the fonts we need
        m_msgFont = TTF_OpenFont( getConfigStr("FONT_FILE"), MSG_RECT_FONT_HEIGHT); //this opens a font style and sets a point size
        if (m_msgFont == NULL)
        {

            fprintf(stderr,"Could not open message font: '%s'\n",SDL_GetError());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"FATAL ERROR LOADING FONT",SDL_GetError(),m_mainWindow);
            exit(EXIT_FAILURE);
        }

        m_bigFont = TTF_OpenFont( getConfigStr("FONT_FILE"), LARGE_FONT_HEIGHT); //this opens a font style and sets a point size
        if (m_bigFont == NULL)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"FATAL ERROR LOADING FONT",SDL_GetError(),m_mainWindow);
            exit(EXIT_FAILURE);
        }

        initMotorGraph();
        initMessageWindow();

        //Load any buttons or other overlay images that go on all screen modes here
        initButton(&m_menuBtn,"MODE_BTN.BMP",805,45,70,210,"m");

        //todo - start button should become start stop
       // initButton(&m_startBtn,"START_BTN.BMP",180,340,150,150,"s");    //commands must be lower case!
        initButton(&m_startBtn,"START_BTN.BMP",820,395,150,150,"s");    //commands must be lower case!


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
 * Change the current mode of the Screen to the next valid option
 *
 *****************************/
void changeScreenMode(void)
{
    //Just the two modes to handle right now
    //todo - add config mode when there is one!

    if(m_screenMode == MODE_GRAPHIC)
        m_screenMode = MODE_DIAGNOSTIC;
    else
        m_screenMode = MODE_GRAPHIC;

}

/*****************************
 *
 * Initalize the window where system messages can be displayed
 *
 *****************************/
void initMessageWindow(void)
{
    m_msgArea.x = MSG_RECT_X;
    m_msgArea.y = MSG_RECT_Y;
    m_msgArea.w = SCREEN_LOGICAL_W-MSG_RECT_X-MSG_RECT_X;
    m_msgArea.h = SCREEN_LOGICAL_H-MSG_RECT_H;
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

    initBarGraph(&m_motorGraph[0], MOTOR_BAR0_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,MOTOR_BAR_B,MOTOR_BAR0_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);
    initBarGraph(&m_motorGraph[1], MOTOR_BAR1_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,MOTOR_BAR_B,MOTOR_BAR1_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);
    initBarGraph(&m_motorGraph[2], MOTOR_BAR2_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,MOTOR_BAR_B,MOTOR_BAR2_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);
    initBarGraph(&m_motorGraph[3], MOTOR_BAR3_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,MOTOR_BAR_B,MOTOR_BAR3_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);
    initBarGraph(&m_motorGraph[4], MOTOR_BAR4_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,MOTOR_BAR_B,MOTOR_BAR4_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);
    initBarGraph(&m_motorGraph[5], MOTOR_BAR5_X,MOTOR_BAR_Y,MOTOR_BAR_H,MOTOR_BAR_W,MOTOR_BAR_B,MOTOR_BAR5_X+5,MOTOR_BAR_TEXT_Y,MOTOR_BAR_TEXT_W,MOTOR_BAR_TEXT_H,LC_ALMOND,LC_DARK_GREEN);

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
 * Update system messages display
 *
 *****************************/
void updateMessageWindow(void)
{
    //todo - This becomes the update Log code to a larger screen
    //toto - replace here with a manager than shows the buttons Start/Stop/Reset/Screen Swap -> msgs ->graphics ->config

    //repaints the window and generates all the text in it

    int f = 0;
    int ptr = m_msgPtr;     //get a local copy
    SDL_Rect thisline;

    //create a rectangle with margins from the message area dimensions for our text
    thisline.x = m_msgArea.x + 5;
    thisline.y = m_msgArea.y;
    thisline.w = m_msgArea.w - 2;
    thisline.h = MSG_RECT_LINE_HEIGHT;

    renderSquare(&m_msgArea,LC_WHITE,LC_DARK_GREEN);
    for(f=0; f<MSG_RECT_LINES; f++)
    {
        if(m_msgBuf[ptr][0] != 0)
        {
            renderText(m_msgBuf[ptr],m_msgFont,LC_YELLOW,thisline);
            thisline.y+=MSG_RECT_LINE_HEIGHT;
        }

        if(++ptr>=MSG_RECT_LINES)
            ptr = 0;               //treat as circular buffer
    }

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
    strncpy(m_msgBuf[m_msgPtr++],msgline,MSG_RECT_LINE_LENGTH-1);
    if(m_msgPtr>=MSG_RECT_LINES)
        m_msgPtr = 0;               //buffer is treated as circular, so go around again

}

/*****************************
 *
 *  clear the message area
 *
 *****************************/
void clearMessageWindow(void)
{
    //resets all to initial values
    memset(m_msgBuf, 0, sizeof m_msgBuf);    //clear the contents of the buffer

    m_msgPtr = 0;

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

    int         f = 0;
    char        bartext[5] = {0,0,0,0,0};

    renderSquare(&m_motorArea,LC_WHITE,LC_DARK_GREEN);   //create the background

    for(f=0; f<6; f++)
    {
        //are we generating (green) or consuming (red)?
        if(g_LC_ControlState.motorAmps[f] < 0)
            updateBarGraph(&m_motorGraph[f], g_LC_ControlState.motorAmps[f],LC_LIGHT_GREEN);
        else
            updateBarGraph(&m_motorGraph[f], g_LC_ControlState.motorAmps[f],LC_RED);

        snprintf(bartext,4,"%-2.1f",(float) g_LC_ControlState.motorAmps[f]);
        renderText(bartext,m_msgFont,LC_BLACK,m_motorGraph[f].label);
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
    graph->bar.h = (int) graph->onePercent * abs((float)motorAmps/m_onePercentAmps);        //calculates percentage times pixels per percent
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
int initButton(LC_Button_t* button, const char * BMPfilename,const int xpos,const int ypos,const int height,const int width, const char *cmd )
{
    //Loads the specified button structure with the bitmap and positional information
    //returns 0 if OK

    button->rect.x = xpos;
    button->rect.y = ypos;
    button->rect.h = height;
    button->rect.w = width;
    button->image =  loadTextureFromBMP(m_mainRenderer, BMPfilename);

    if(button->image == NULL)
    {
        fprintf(stderr,"Button Image file %s failed to load\n",BMPfilename);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"ERROR","Button Image file not found or failed to load",m_mainWindow);
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
//    static int angle = 0;
    SDL_RenderCopy(m_mainRenderer, button->image, NULL, &button->rect);
    //SDL_RenderCopyEx(m_mainRenderer, button->image, NULL, &button->rect,angle++,NULL,SDL_FLIP_NONE);
}


/*****************************
 *
 *  Update Banner
 *
 *****************************/
void updateBanner(void)
{
    static const SDL_Rect	Banner_rect = {BANNER_RECT_X,BANNER_RECT_Y,25,25};

    renderText(getConfigStr("LOCO_NAME"), m_bigFont, LC_ORANGE, Banner_rect);

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
        renderText(str, m_bigFont, LC_BLACK, Throttle_rect);
    }
}

/*****************************
 *
 *  Update Dynamic
 *
 *****************************/
void updateDynamic()
{

    static const SDL_Rect	Dynamic_rect = {DYN_RECT_X,DYN_RECT_Y,25,25};    //custom numbers to match bmp

    if(g_LC_ControlState.DynBrakePos > -1)
    {
        char str[2] = {'0' + g_LC_ControlState.DynBrakePos,0};
        renderText(str, m_bigFont, LC_BLACK,Dynamic_rect);
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
        renderText("FORWARD", m_bigFont, LC_RED,Dynamic_rect);
    }
    else if(g_LC_ControlState.DirReverse)
    {
        renderText("REVERSE", m_bigFont, LC_RED,Dynamic_rect);
    }

    else if(!g_LC_ControlState.DirForward && !g_LC_ControlState.DirReverse)
    {
        renderText("NEUTRAL", m_bigFont, LC_DARK_GREEN,Dynamic_rect);
    }
}

/*****************************
 *
 *  Update Speedo
 *
 *****************************/
void updateSpeedo(void)
{
    //TODO - speedo code
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
    renderText(str, m_bigFont, LC_BLACK, Battery_rect);
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
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"ERROR","Cant load image file. See log for details",m_mainWindow);

    }

    newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);

    SDL_FreeSurface( loadedSurface );

    return newTexture;


}

void renderText(const char* text, TTF_Font* font,const SDL_Color colour, SDL_Rect Message_rect)
{

    // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
    SDL_Surface* surfaceMessage = TTF_RenderUTF8_Blended(font, text, colour);

    //now you can convert it into a texture
    SDL_Texture* Message = SDL_CreateTextureFromSurface(m_mainRenderer, surfaceMessage);

    Message_rect.w = surfaceMessage->w; // controls the width of the rect
    Message_rect.h = surfaceMessage->h ; // controls the height of the rect

    SDL_RenderCopy(m_mainRenderer, Message, NULL, &Message_rect); //you put the renderer's name first, the Message, the crop size(you can ignore this if you don't want to dabble with cropping), and the rect which is the size and coordinate of your texture

    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);

}

void renderSquare(const SDL_Rect* coords, const SDL_Color lineColour, const SDL_Color fillColour)
{
    SDL_SetRenderDrawColor(m_mainRenderer,fillColour.r,fillColour.g,fillColour.b,fillColour.a);
    SDL_RenderFillRect(m_mainRenderer,coords);
    SDL_SetRenderDrawColor(m_mainRenderer,lineColour.r,lineColour.g,lineColour.b,lineColour.a);
    SDL_RenderDrawRect(m_mainRenderer,coords);

}

