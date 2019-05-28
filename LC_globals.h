//
//  LC_globals.h
//  LocoSoundv2
//
//  Created by Chris Draper on 20/09/14.
//  Copyright (c) 2014-2019. All rights reserved.
//

#ifndef LocoControl_LC_globals_h
#define LocoControl_LC_globals_h

#define PROGRAM_VERSION "Version 2.3.1 Released 28 May 2019\0"

#include <stdbool.h>

/*******************************************
 *  System Wide Type Definitions
 *******************************************/

 typedef enum {
    MOTOR_STOPPED,
    MOTOR_STARTING,
    MOTOR_RUNNING,
    MOTOR_STOPPING
} LC_MotorState_t;
/*******************************************
 *
 * ControlState is the structure that is filled by authenticated
 * Data from the drivers console - and is consumed by various modules
 * including the actual motor control and the sound output
 *
 *******************************************/

typedef struct {
    LC_MotorState_t MotorState;       //for prime mover status - running, starting or stopping or stopped
    bool            DynBrakeActive;     //Set high to assert dynamic brake in use
    bool            ThrottleActive;     //Set high to assert throttle in use
    bool            DirForward;         //Set high to assert in Forward
    bool            DirReverse;         //Set high to assert in Reverse
    bool            HornPressed;        //Set high when button pressed
    bool            Emergency;          //Set high by controller when normal - i.e. NOT asserting an emergency
    bool            ConsoleHealthy;     //true when drivers console is OK  - NOTE - this whole struct should NEVER BE ZERO if healthy
    int             ThrottlePos;        //-1=stopped, 0=Idle, 1-8 power on. No other values permitted
    int             DynBrakePos;        //0 = off, 1-8 for degrees of braking
    int             motorAmps[6];       //amps returned as milliamps - to avoid using floats in comms. Zero based array
    int             speed;              //actual ground speed returned as meters per hour to avoid using floats
    float           vbat;               //Current traction battery voltage
    bool            MotorsEnabled;      //True if the traction motors are enabled
    bool            DynamicEnabled;     //True if the Dynamic Braking mode is enabled
} LC_ControlState_t;


bool				g_Debug;       //Global Debug flag set by command line option if true then enable debugging info to screen
LC_ControlState_t	g_LC_ControlState;     //Global state of the controls
SDL_Event			g_event;			   //SDL event handling object

#endif
