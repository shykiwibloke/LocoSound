//
//  LC_Sound.h
//  Part of the LocoControl system
//
//  Created by Chris Draper on 20/09/14.
//  Copyright (c) 2014 Winter Creek. All rights reserved.
//

#ifndef LocoControl_Sound_h
#define LocoControl_Sound_h


#include <stdbool.h>
#include "LC_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "LC_globals.h"     //definitions that are common across all MPU's in the Loco Sound system
#include "LC_configReader.h"
#include "LC_screen.h"

/*********************************************
*
* Sound related constants
*
*********************************************/

#define  LC_SOUND_SAMPLE_RATE	22050
#define  LC_SOUND_FORMAT		AUDIO_S16
#define  LC_NUM_CHANNELS		2       //stereo
#define  LC_CHUNK_SIZE			1024 //2048
#define  LC_SOUND_QUEUE_MAX		5      //number of samples that can be queued at any one time
#define  LC_MAX_CHANNELS		8
#define  LC_PLAY_ONCE			0
#define  LC_PLAY_LOOP			-1
/*******************************************
 *  Sound Module Specific File Name Declarations
 *  NB: The Order and number of these must match
 *  the enumerations in SoundFile_t
 *******************************************/

#define F_NONE     "NONE"          //dummy entry for file table. If this name turns up in a message you have a bug!
#define F_START    "START.WAV"
#define F_IDLE     "IDLE.WAV"
#define F_NOTCH1   "NOTCH1.WAV"
#define F_NOTCH2   "NOTCH2.WAV"
#define F_NOTCH3   "NOTCH3.WAV"
#define F_NOTCH4   "NOTCH4.WAV"
#define F_NOTCH5   "NOTCH5.WAV"
#define F_NOTCH6   "NOTCH6.WAV"
#define F_NOTCH7   "NOTCH7.WAV"
#define F_NOTCH8   "NOTCH8.WAV"
#define F_REVUP	   "REVUP.WAV"		//IDLE TO NOTCH EIGHT
#define F_REVDOWN  "REVDOWN.WAV"    //From Notch 8 to Idle
#define F_HORN     "HORN.WAV"
#define F_HORN_ST  "HORN_ST.WAV"
#define F_HORN_END "HORN_END.WAV"
#define F_BELL     "BELL.WAV"
#define F_DYNBK_ST "DYNBK_ST.WAV"
#define F_DYNBK    "DYNBK.WAV"
#define F_AIRCOMP  "AIRCOMP.WAV"
#define F_TRACTION "TRACTION.WAV"


/*******************************************
 *  Sound Module Specific Type Definitions
 *
 * (See LS_globals for system wide typedefs)
 *******************************************/

typedef enum {
	SF_NONE     = -1,			  //No sound to play
	SF_IDLE		=  0,
	SF_NOTCH1	=  1,
	SF_NOTCH2	=  2,
	SF_NOTCH3	=  3,
	SF_NOTCH4	=  4,
	SF_NOTCH5	=  5,
	SF_NOTCH6	=  6,
	SF_NOTCH7   =  7,
	SF_NOTCH8   =  8,
	SF_REVUP	=  9,             //Acceleration sounds
	SF_REVDOWN  = 10,
	SF_START	= 11,             //Engine Starting Sound
	SF_HORN_ST	= 12,
	SF_HORN		= 13,             //Steady state horn that can be looped
	SF_HORN_END = 14,
	SF_BELL		= 15,             //Bell used for fail safe & emergency alarms. Also hear on starting the engine.
	SF_DYNBK_ST	= 16,
	SF_DYNBK	= 17,
	SF_AIRCOMP	= 18,             //Air Compressor (Generally restricted to idle, but random
	SF_TRACTION	= 19,             //Traction blower + gearbox noises
} LC_SoundFile_t;

#define SF_MAX_ITEMS   21         //Used to determine the size of this enumeration


//sound queue structure definition
typedef struct {
	char            *Qlabel;                             //points to a description of the owner of this queue
	int				channel;							//The mixer channel we are using
	bool			IsPlaying;							//True if this queue is currently being played. False when finished.
	int			    currentItem;						//gets incremented until it points to a track that repeats or is a SF_NONE
	Uint32			currentFadeStart;					//ByteCount to start fading out
	Uint32			currentPlayEnd;						//BytesCount to stop playing
	Uint8           volLeft;                            //Left stereo channel volume
	Uint8           volRight;                           //Right stereo channel volume

	Mix_Chunk		*soundChunk[LC_SOUND_QUEUE_MAX];	    //Holds pointers to memory for current sound
	int				fadeInTime[LC_SOUND_QUEUE_MAX];	    //Length of fade in. 0 for none
	int 			fadeOutTime[LC_SOUND_QUEUE_MAX];	//Length of fade out. 0 for none
	int				loopCount[LC_SOUND_QUEUE_MAX];		//number of times to repeat. -1 = infinite,0 = 1, 1 = 2 etc
} LC_SoundQueue_t;

/*********************************************
*
*  Macro Definitions
*
*********************************************/

//Macro to make file operations easier to read

#define LC_LOADWAV(name, index) \
    if ((m_SoundSamples[index] = Mix_LoadWAV(name)) == NULL) \
	    { fprintf(stderr, "SDL_LoadWAV ERROR loading '%s': %s\n", name, SDL_GetError()); }



/*****************************************
*
 * Function Prototypes
*
******************t3***********************/

int  initAudio(void);
void closeAudio(void);
void soundService(void);
void freeBuffers(void);
void serviceChannel(LC_SoundQueue_t *pQ);
void fadeOutQueue(LC_SoundQueue_t *pQ,Uint32 fadeOut);
void finishPlayingSound(LC_SoundQueue_t *pQ);
void changeThrottle(void);
void changeDynamic(void);
void changeHorn(void);
void changeCompressor(void);
void queueSound(LC_SoundQueue_t *pQ, int index, LC_SoundFile_t sound, Uint32 fadeIn, Uint32 fadeOut, int loopCount);
void queuePartSound(LC_SoundQueue_t *pQ, int index, LC_SoundFile_t sound, Uint32 startPos, Uint32 endPos, Uint32 fadeIn, Uint32 fadeOut, int loopCount );
void showChannelSummary(void);
void clearAllQueues(void);
void clearQueue(LC_SoundQueue_t *pQ);
void playQueueItem(LC_SoundQueue_t *pQ);
void stopQueue(LC_SoundQueue_t *pQ, Uint32 fadeOut);

#endif
