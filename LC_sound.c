//
//  LC_sound.c
//  Loco Control Sound Manager
//
//  Created by Chris Draper on 6/05/15.
//  Copyright (c) 2015-2019. All rights reserved.
//
//  VERSION 2.4.0 released 26/12/2019

#include "LC_sound.h"

/*********************************************
*
* Module Variable Declarations Specific to this Module
*
*********************************************/

char  Q_LABEL_E[] = "Engine Queue";
char  Q_LABEL_D[] = "Dynamic Queue";
char  Q_LABEL_H[] = "Horn Queue";
char  Q_LABEL_A[] = "AirComp Queue";
char  Q_LABEL_T[] = "Traction Queue";
char  Q_LABEL_U[] = "Closing, or is an ORPHAN!";


Mix_Chunk		 *m_SoundSamples[SF_MAX_ITEMS];			//Holds all the sound samples and related information in memory for fast access
LC_SoundQueue_t  m_EngineQueue;							//Used to contain each string of samples to be played
LC_SoundQueue_t  m_DynBrakeQueue;
LC_SoundQueue_t  m_HornQueue;
LC_SoundQueue_t	 m_AirCompQueue;
LC_SoundQueue_t  m_TractionQueue;


int	        	 m_RevUp[9];							//Holds the sample start/stop points for each notch in revup
int	             m_RevDown[9];							//Holds the sample start/stop points for each notch in revdown


int			     m_SndThrottlePos = 0;				     //Module store for our current internal setting. Used to detect change arriving
bool			 m_SndHornPressed = 0;					 //ditto
int			     m_SndDynBrakePos = 0;					 //ditto

int				 m_fadeShort = 500;
int				 m_fadeSTD = 1500;
int				 m_fadeLong = 10000;

/*********************************************
*
* soundService is the main service routine
*addMessageLine("");
*********************************************/
void soundService()
{
	// main hook to ensure sound is going. call frequently

    //Horn can be used at any time motor running or not (for safety)
    if (g_LC_ControlState.HornPressed != m_SndHornPressed)
	{
		changeHorn();
	}

    //Rest of sounds depend on current motor state - e.g. cant have engine sounds when engine starting or stopped!
    switch(g_LC_ControlState.MotorState) {

        case MOTOR_STOPPED:
            //motor is stopped - so check on air dryer sound only

            break;
        case MOTOR_STARTING:
            //todo - kick off starting sounds here, then watch for the IsLooping to more to Running
            //Also stop air dryer sounds if running

            if(m_EngineQueue.IsPlaying == false)
            {
                clearAllQueues();       //start with a clean slate - just in case.
                queueSound(&m_EngineQueue,0,SF_BELL,0,0,LC_PLAY_ONCE);
                queueSound(&m_EngineQueue,1,SF_START,0,0,LC_PLAY_ONCE);    //todo - dont set 'idle' until passed this point
                queueSound(&m_EngineQueue,2,SF_IDLE,0,0,LC_PLAY_LOOP);
                playQueueItem(&m_EngineQueue);

                clearQueue(&m_AirCompQueue);        //This should already be done by clear all queues above?
                if(m_AirCompQueue.IsPlaying)
                {
                    fadeOutQueue(&m_AirCompQueue,m_fadeShort);  //force current sounds to fade out - we are not stopped any longer.
                }
            }
            if(m_EngineQueue.IsLooping == true)
            {
                g_LC_ControlState.MotorState = MOTOR_RUNNING;
                g_LC_ControlState.ThrottleActive = true;
                snprintf(m_startBtn.textPressed,BTN_TEXT_LEN," STOP");
            }

            break;

        case MOTOR_STOPPING:
            //todo - kick off stopping sounds here, then watch for the IsLooping to more to Stopped

            g_LC_ControlState.ThrottleActive = false;		//Reset controls to prevent override
            g_LC_ControlState.DynBrakeActive = false;

            if (m_EngineQueue.IsPlaying)
            {
                fadeOutQueue(&m_EngineQueue,m_fadeSTD);  //force current sounds to fade out gradually
            }
            else
            {

                //clear out current settings for the engine queue
                clearQueue(&m_EngineQueue);


                //Special manipulation of air compressor so it plays air dryer forever while we are shut down
                if (m_AirCompQueue.IsPlaying)
                {
                    fadeOutQueue(&m_AirCompQueue,m_fadeShort);  //force current sounds to fade out gradually
                }
                clearQueue(&m_AirCompQueue);
                queueSound(&m_AirCompQueue, 0, SF_AIRDRYER, 0, 100, LC_PLAY_LOOP);
                playQueueItem(&m_AirCompQueue);

                g_LC_ControlState.MotorState = MOTOR_STOPPED;

            }
            break;

        case MOTOR_RUNNING:
            //Motor running - so all the sounds can be checked

            if (g_LC_ControlState.ThrottleActive && g_LC_ControlState.ThrottlePos != m_SndThrottlePos)
            {
                changeThrottle();
            }

            if (g_LC_ControlState.DynBrakeActive && g_LC_ControlState.DynBrakePos != m_SndDynBrakePos)
            {
                changeDynamic();
            }

            //Dont play compressor sounds until we are properly in idle
            if (g_LC_ControlState.ThrottlePos == 0 && g_LC_ControlState.DynBrakePos ==0 && m_EngineQueue.IsInTransition == false)
            {
                changeCompressor();                     //compressor runs on random time
            }
            else if(m_AirCompQueue.IsPlaying)
            {
                fadeOutQueue(&m_AirCompQueue,m_fadeShort);  //force current sounds to fade out - we are not idling any longer.
            }

            break;
        }

   //Go service the channels that are running (incl those just changed)
    serviceChannel(&m_HornQueue);
    serviceChannel(&m_EngineQueue);
    serviceChannel(&m_TractionQueue);
    serviceChannel(&m_DynBrakeQueue);
    serviceChannel(&m_AirCompQueue);

}

/********************************************
*
* changeThrottle - handles throttle setting changes
*
*********************************************/

void changeThrottle(void)
{
	//Handle throttle notch change up or down - also handles start and stop
	logMessage("changeThrottle()",true);

	if (m_EngineQueue.IsPlaying){
		fadeOutQueue(&m_EngineQueue,m_fadeSTD);  //force current sounds to fade out gradually
	}

	//clear out current settings
	clearQueue(&m_EngineQueue);

	if(g_LC_ControlState.ThrottlePos > m_SndThrottlePos)   //start or speed up
	{
		//Queue appropriate rev-up sequence as well as new throttle setting.
		logInt("Reving up from ",m_SndThrottlePos);
        logInt("Reving up to ", g_LC_ControlState.ThrottlePos);
		queuePartSound(&m_EngineQueue, 0, SF_REVUP, m_RevUp[m_SndThrottlePos], m_RevUp[g_LC_ControlState.ThrottlePos], m_fadeShort, m_fadeSTD, LC_PLAY_ONCE);
		queueSound(&m_EngineQueue,1,g_LC_ControlState.ThrottlePos,m_fadeSTD,m_fadeSTD,LC_PLAY_LOOP);
	}
	else   //slow down or stop
	{
		logInt("Rev down from ",m_SndThrottlePos);
		logInt("Rev down to ", g_LC_ControlState.ThrottlePos);
		queuePartSound(&m_EngineQueue, 0, SF_REVDOWN, m_RevDown[m_SndThrottlePos], m_RevDown[g_LC_ControlState.ThrottlePos],  m_fadeShort, m_fadeSTD, LC_PLAY_ONCE);
		queueSound(&m_EngineQueue,1,g_LC_ControlState.ThrottlePos,m_fadeSTD,m_fadeSTD,LC_PLAY_LOOP);
	}

	//having set up a queue - trigger the playing of the first sound (others will follow from service channel)
	playQueueItem(&m_EngineQueue);
	m_SndThrottlePos = g_LC_ControlState.ThrottlePos;
}
/********************************************
*
* changeDynamic - handles Dynamic setting changes
*
*********************************************/
void changeDynamic(void)
{

    logMessage("changeDynamic()",true);
    //if dynamic brake is NOT currently playing and lever is NOT at idle or Notch 1 (used to 'fiddle' the speed in coasting mode)
	if( !m_DynBrakeQueue.IsPlaying && g_LC_ControlState.DynBrakePos > 1 && g_LC_ControlState.DynBrakePos < 8)
	{
		//startup dyn braking - stays the same irrespective of dyn brake handle position
		clearQueue(&m_DynBrakeQueue);
		queueSound(&m_DynBrakeQueue,0,SF_DYNBK_ST,0,m_fadeSTD,LC_PLAY_ONCE);
		queueSound(&m_DynBrakeQueue,1,SF_DYNBK,m_fadeSTD,0,LC_PLAY_LOOP);
		playQueueItem(&m_DynBrakeQueue);

	}

    //Traction Motor Blowers

    //if moving to position 1, idle or 8 (remember 8 is an emergency position)
    if  (g_LC_ControlState.DynBrakePos < 2 || g_LC_ControlState.DynBrakePos > 7)
    {
        fadeOutQueue(&m_DynBrakeQueue,m_fadeSTD);  //force current sounds to fade out gradually
        fadeOutQueue(&m_TractionQueue,m_fadeSTD);  //force current sounds to fade out gradually
    }
    //if currently < 5 and moving to 5 or greater
    else if  (g_LC_ControlState.DynBrakePos > 4 && m_SndDynBrakePos < 5)				//traction sound should play when traction motors are working
    {
        clearQueue(&m_TractionQueue);
        queueSound(&m_TractionQueue, 0, SF_TRACTION, m_fadeLong, 0, LC_PLAY_LOOP);
        playQueueItem(&m_TractionQueue);

    }
    //if currently > 5 and moving to less than 5 but not to zero
    else if  (g_LC_ControlState.DynBrakePos != 0 && m_SndDynBrakePos >= 5)		//traction sound should play when traction motors are working
    {
        if (m_TractionQueue.IsPlaying)
        {
            fadeOutQueue(&m_TractionQueue,m_fadeLong);  //force current sounds to fade out gradually
        }
    }

	m_SndDynBrakePos = g_LC_ControlState.DynBrakePos;
}


/********************************************
*
* changeHorn - handles Horn setting changes
*
*********************************************/
void changeHorn(void)
{
    if (m_SndHornPressed)
    {
        if(m_HornQueue.currentItem == 1)  //already playing main horn sound - shut it up
        {
            fadeOutQueue(&m_HornQueue,m_fadeShort);  //force current sounds to fade out
            clearQueue(&m_HornQueue);
            queueSound(&m_HornQueue, 0, SF_HORN_END, 100, 0, LC_PLAY_ONCE);
            playQueueItem(&m_HornQueue);
            m_SndHornPressed = false;
        }
	}
	else  //horn not playing yet - let her rip!
	{

		clearQueue(&m_HornQueue);
		queueSound(&m_HornQueue, 0, SF_HORN_ST, 0, 100, LC_PLAY_ONCE);
		queueSound(&m_HornQueue, 1, SF_HORN, 100, 100, LC_PLAY_LOOP);
		playQueueItem(&m_HornQueue);
		m_SndHornPressed = true;

	}
}
/********************************************
*
* changeCompressor - handles Compressor setting changes
*
*********************************************/
void changeCompressor(void)
{

    int value = 2 + rand() % 10;            //3 - 10 mins range seed
    static Uint32 torun = 30000;            //first time goes off 30 seconds into idle after startup

    if(SDL_TICKS_PASSED(SDL_GetTicks(),torun))
    {
        logMessage("changeCompressor() - starting run",true);
        if (m_AirCompQueue.IsPlaying)
        {
            fadeOutQueue(&m_AirCompQueue,m_fadeShort);  //force current sounds to fade out gradually
        }

        //start the air compressor sound
		clearQueue(&m_AirCompQueue);
		queueSound(&m_AirCompQueue, 0, SF_AIRCOMP, 0, 100, LC_PLAY_ONCE);
		queueSound(&m_AirCompQueue,1,SF_AIRCOMP_END,0,100,LC_PLAY_ONCE);
		playQueueItem(&m_AirCompQueue);

        //set the next time to run
        torun = SDL_GetTicks()+ (value * 60000);   //set current time plus value * one minute to give 1 - 10 mins range
    }
    else if(!m_AirCompQueue.IsPlaying)  //if queue is currently not playing
    {
        logMessage("changeCompressor() - completing run",true);

        clearQueue(&m_AirCompQueue);
 		queueSound(&m_AirCompQueue, 0, SF_AIRDRYER, 0, 100, LC_PLAY_LOOP);
        playQueueItem(&m_AirCompQueue);
    }

}

/********************************************
*
* queueSound - add a sound and its parameters to the specified queue
*
*********************************************/

 void queueSound(LC_SoundQueue_t *pQ,
					const int index,
					const LC_SoundFile_t sound,
					const Uint32 fadeIn,
					const Uint32 fadeOut,
					const int loopCount)
{

    if(m_SoundSamples[sound] != NULL)
	{
	    pQ->soundChunk[index] = Mix_QuickLoad_RAW(m_SoundSamples[sound]->abuf,m_SoundSamples[sound]->alen);

        pQ->fadeInTime[index] = fadeIn;
        pQ->fadeOutTime[index] = fadeOut;
        pQ->loopCount[index] = loopCount;

	}
}

/*********************************************
 *
 * queuePartSound - add part of a sound and its parameters to the specified queue
 *
 *********************************************/

void queuePartSound(LC_SoundQueue_t *pQ,
				const int index,
				const LC_SoundFile_t sound,
				const Uint32 startPos,
				const Uint32 endPos,
				const Uint32 fadeIn,
				const Uint32 fadeOut,
				const int loopCount )
{

        //Called to extract an exerpt from a longer sound
        //Used exclusively for reving up and down in this version of the code

    if(m_SoundSamples[sound] != NULL)
	{

        Uint32 bufstart = (Uint32) m_SoundSamples[sound]->abuf;
        Uint32 slen = m_SoundSamples[sound]->alen;
        Uint32 reqlen = 0;

        bufstart += startPos;					//startPos is the offset from the buffer start

        //endpos check allows caller to specify a huge number to ensure end is reached.
        //check for and trim to sound length before doing sample math to get requested length
        if (endPos > slen)
            reqlen = slen - startPos;
        else
            reqlen = endPos - startPos;				//calculates the actual required length.

    /*   Debug code only
        fprintf(stderr,"_______________\nFile buffer starts at \t\t %p \n",m_SoundSamples[sound]->abuf);
        fprintf(stderr,"File length is \t\t %d bytes \n",slen);
        fprintf(stderr,"File ends at address \t\t %p bytes \n",m_SoundSamples[sound]->abuf + slen);
        fprintf(stderr,"Play starting at offset \t\t %d bytes \n",startPos);
        fprintf(stderr,"So Play starting address is \t\t %p bytes \n",(void *)bufstart);
        fprintf(stderr,"Play length is \t\t %d bytes \n",reqlen);
        fprintf(stderr,"Play ending address is \t\t %X \n\n",bufstart + reqlen);
    */
        //bounds checking
        if(bufstart == 0 || reqlen == 0 || reqlen > slen || startPos >= endPos)
            logMessage("Specified mix_chunk buffer, requested length and/or start/stop positions invalid",true);
        else
        {

            pQ->soundChunk[index] =  Mix_QuickLoad_RAW((Uint8*) bufstart,reqlen);
            pQ->fadeInTime[index] = fadeIn;
            pQ->fadeOutTime[index] = fadeOut;
            pQ->loopCount[index] = loopCount;
        }
	}
}

/*********************************************
*
* playQueueItem
*
*********************************************/
void playQueueItem(LC_SoundQueue_t *pQ)
{

   if(pQ->soundChunk[0] != NULL)
   {
        //set fade out parameters for later checking (time based on actual time NOW - being the time we started this chunk playing)

        if ((pQ->loopCount[pQ->currentItem] != LC_PLAY_LOOP) && (pQ->soundChunk[pQ->currentItem]->alen > 0))  //also guard against divide by zero
        {
            pQ->currentPlayEnd = SDL_GetTicks();
            pQ->currentPlayEnd += ((pQ->soundChunk[pQ->currentItem]->alen / LC_SOUND_SAMPLE_RATE )*250); //milliseconds from bytes
            pQ->currentFadeStart = pQ->currentPlayEnd - pQ->fadeOutTime[pQ->currentItem];
            pQ->IsInTransition = true;          //signal to other systems we are in the process of changing the motor revs
            pQ->IsLooping = false;

        } else {
            //we are setting up a looping sound so ensure end and fade start values are 'disabled' by huge value
            pQ->currentPlayEnd = 99999999;
            pQ->currentFadeStart = 99999999;
            pQ->IsLooping = true;               //Can only get here on loop. Single or multiple play sounds do not come through here.
            pQ->IsInTransition = false;         //playing a looping sound guarantees we are no longer in transition
        }

        //Play sound on next available channel and retrieve the channel allocated for later reference
        pQ->channel = Mix_FadeInChannel(-1, pQ->soundChunk[pQ->currentItem],pQ->loopCount[pQ->currentItem], pQ->fadeInTime[pQ->currentItem]);

        if (pQ->channel == -1)  //no channel was allocated, so playing did not start
        {
            pQ->IsPlaying = false;
            pQ->IsLooping = false;
            pQ->IsInTransition = false;
            logInt("playQueueItem() Error playing sound ",pQ->currentItem);
            logInt("playQueueItem() On channel ",pQ->channel);
            logString("playQueueItem() For ",pQ->Qlabel);
        }
        else
        {
            //Set the overall volume of the channel properly here according to user config settings
            Mix_SetPanning(pQ->channel,pQ->volLeft,pQ->volRight);

            pQ->IsPlaying = true;
            logString("playQueueItem() for ",pQ->Qlabel);
            logInt("playQueueItem() Assigned to channel ",pQ->channel);
            logInt("playQueueItem() IsInTransition ",pQ->IsInTransition);
            logInt("playQueueItem() IsLooping ",pQ->IsLooping);

        }
    }
}

/*********************************************
 *
 * fadeOutQueue Playing queue gets faded out
 *
 *********************************************/
void fadeOutQueue(LC_SoundQueue_t *pQ,const Uint32 fadeOut)
{
	pQ->currentFadeStart = SDL_GetTicks()-1;         //Trigger the fade out of the sound in the serviceChannel() call
	pQ->fadeOutTime[pQ->currentItem] = fadeOut;
	pQ->currentPlayEnd = pQ->currentFadeStart + fadeOut; //Trigger the end of the sound in the service channel code
	pQ->loopCount[pQ->currentItem] = 0;
	if( pQ->currentItem < LC_SOUND_QUEUE_MAX)         //Dont alter next sound if we are at the end of the queue
    {
        pQ->loopCount[(pQ->currentItem)+1] = 0;
        pQ->soundChunk[(pQ->currentItem)+1] = NULL;
    }
    logString("fadeOutQueue() for ",pQ->Qlabel);
	logInt("fadeOutQueue() channel ",pQ->channel);
	serviceChannel(pQ);
}

/*********************************************
 *
 * Service current playing channels
 *
 *********************************************/
void serviceChannel(LC_SoundQueue_t *pQ)
{

	static Uint32 currentTime;


	if (pQ->IsPlaying)		//This logic only applies to channels currently playing
	{

		currentTime = SDL_GetTicks();

		//has current sound finished?
		if (SDL_TICKS_PASSED(currentTime,pQ->currentPlayEnd) ) //check for complete end-of-sound b4 checking fade out times
		{
            logString("serviceChannel() for ",pQ->Qlabel);
            logInt("serviceChannel() Sound finished on channel ",pQ->channel);
			finishPlayingSound(pQ);

		}
		//has current sound got to the point of fading out?
		else if (SDL_TICKS_PASSED(currentTime, pQ->currentFadeStart))
		{
			logInt("serviceChannel()Fading out sound on channel ", pQ->channel);
			Mix_FadeOutChannel(pQ->channel, pQ->fadeOutTime[pQ->currentItem]);
			finishPlayingSound(pQ);
		}
	}
}

/*********************************************
*
*  finishPlayingSound
*
**********************************************/
void finishPlayingSound(LC_SoundQueue_t *pQ)
{
    //If sound is complete and there are no more in this queue - then shut the queue down here even though we might be still fading

    if(pQ->currentItem == LC_SOUND_QUEUE_MAX)   //got to max queue - have to check first to avoid bad pointer
    {
        pQ->IsPlaying = false;
        pQ->IsInTransition = false;
		pQ->IsLooping = false;
        logString("finishPlayingSound() Queue reached max samples ",pQ->Qlabel);
    }
    else if(pQ->soundChunk[pQ->currentItem +1] == NULL)  //if next item is a null then no more in queue
	{
		pQ->IsPlaying = false;
		pQ->IsInTransition = false;
		pQ->IsLooping = false;
		logString("finishPlayingSound() Queue reached end ",pQ->Qlabel);
	}
	else
	{
		// there are more in this queue - so tidy up and go start the next one.
		logString("finishPlayingSound() Queuing next for ",pQ->Qlabel);
		pQ->currentItem++;      //inc array index and start the next sound
		playQueueItem(pQ);
	}
}

/*********************************************
 *
 * Shows the status of all the sound channels
 *
 *********************************************/
void showChannelSummary(void)
{

//todo - split into display and gather routines
//and add a garbage collection routine that can call instead of display every so many seconds

    int     f = 0;
    char    *ptr = NULL;

    addMessageLine("_______________");    //signal start of new set of commands
    addMessageLine("Update on Mixer Channel Status");


    for(f=0; f<LC_MAX_CHANNELS;f++)
    {
        if(Mix_Playing(f))
        {
            if(m_EngineQueue.channel == f && m_EngineQueue.IsPlaying)
                ptr = Q_LABEL_E;
            else if(m_DynBrakeQueue.channel == f && m_DynBrakeQueue.IsPlaying)
                ptr = Q_LABEL_D;
            else if(m_HornQueue.channel == f && m_HornQueue.IsPlaying)
                ptr = Q_LABEL_H;
            else if(m_AirCompQueue.channel == f && m_AirCompQueue.IsPlaying)
                ptr = Q_LABEL_A;
            else if(m_TractionQueue.channel == f && m_TractionQueue.IsPlaying)
                ptr = Q_LABEL_T;
            else
            {
                ptr = Q_LABEL_U;              //Unkown or Orphan
                if(Mix_FadingChannel(f))
                {
                    snprintf(m_msgTempLine,LC_MSGLINE_LEN,"%d channel is fading for %s", f, ptr);
                    addMessageLine(m_msgTempLine);
                }
            }

            snprintf(m_msgTempLine,LC_MSGLINE_LEN,"%d channel is playing for %s", f, ptr);
            addMessageLine(m_msgTempLine);

        }
        else  //if NOT Mix_playing
        {
            snprintf(m_msgTempLine,LC_MSGLINE_LEN,"%d channel is free", f);
            addMessageLine(m_msgTempLine);
        }
     }
}

/*********************************************
 *
 * Checks all the sound channels and kills any orphans
 * Called from main loop every 10 seconds
 *
 *********************************************/
void soundChannelWatchdog(void)
{

    snprintf(m_msgTempLine,LC_MSGLINE_LEN,"XX- Sound Channel Watchdog Run -XX");   //only appears in log with debug flag set
    addMessageLine(m_msgTempLine);


    int     f = 0;

    for(f=0; f<LC_MAX_CHANNELS;f++)
    {
        if(Mix_Playing(f))  //If channel is active...
        {
            if(             //AND if channel is NOT associated with any valid queue...
              (m_EngineQueue.channel != f)
            &&(m_DynBrakeQueue.channel != f)
            &&(m_HornQueue.channel != f)
            &&(m_AirCompQueue.channel != f)
            &&(m_TractionQueue.channel != f)
            )
            {
                if(!Mix_FadingChannel(f))  // AND if the channel is not current fading - then it is likely an Orphan
                {
                    snprintf(m_msgTempLine,LC_MSGLINE_LEN,"Channel %d HALTED by soundChannelWatchdog()", f);
                    addMessageLine(m_msgTempLine);
                    Mix_HaltChannel(f);

                    //Note - channel could have been caught just as it was closing, but additional halt command should not hurt
                }
            }
        }
     }
}



/*********************************************
 *
 * Clear all Queues  - clears all queues to default state
 *
 *********************************************/
 void clearAllQueues(void)
 {
    int f = 0;

    logMessage("Clearing All Sound Queues",true);

    for(f=0;f<LC_MAX_CHANNELS;f++)
    {
              // Mix_FadeOutChannel(f,1);
              Mix_HaltChannel(f);
    }

	clearQueue(&m_EngineQueue);
	clearQueue(&m_AirCompQueue);
	clearQueue(&m_DynBrakeQueue);
	clearQueue(&m_HornQueue);
	clearQueue(&m_TractionQueue);

 }

/*********************************************
 *
 * Clear Queue  - clears the specified queue to default state
 *
 *********************************************/

void clearQueue(LC_SoundQueue_t *pQ)
{

	int f;

	pQ->IsPlaying = false;
	pQ->IsInTransition = false;
	pQ->IsLooping = false;
	pQ->currentItem = 0;
	pQ->currentFadeStart = 0;
	pQ->currentPlayEnd = 0;
	//NB: dont touch the volume variables - leave them as-is

	for(f = 0;f< LC_SOUND_QUEUE_MAX;f++)  //clear the queue
	{
		pQ->soundChunk[f] = NULL;
		pQ->fadeInTime[f] = 0;
		pQ->fadeOutTime[f] = 0;
		pQ->loopCount[f] = 0;

	}
	if (Mix_Playing(pQ->channel))
	{
		logString("clearQueue() for ",pQ->Qlabel);
		logInt("clearQueue() channel ",pQ->channel);
 		pQ->channel = -1;         //set so next sound will be allocated a fresh mix channel

	}


}

/*********************************************
 *
 * SetSound Volume - checks the volume flags and clears the specified queue to proper default state
 *
 *********************************************/

void setSoundVolume()
{
    //Set Sound Volumes to full default values first.

	m_EngineQueue.volLeft = getConfigVal("VOL_ENGINE_LEFT");
	m_EngineQueue.volRight = getConfigVal("VOL_ENGINE_RIGHT");

	m_AirCompQueue.volLeft = getConfigVal("VOL_AIRCOMPRESSOR_LEFT");
    m_AirCompQueue.volRight = getConfigVal("VOL_AIRCOMPRESSOR_RIGHT");

	m_DynBrakeQueue.volLeft = getConfigVal("VOL_DYNAMIC_LEFT");
    m_DynBrakeQueue.volRight = getConfigVal("VOL_DYNAMIC_RIGHT");

	m_TractionQueue.volLeft = getConfigVal("VOL_TRACTION_LEFT");
    m_TractionQueue.volRight = getConfigVal("VOL_TRACTION_RIGHT");

	m_HornQueue.volLeft = getConfigVal("VOL_HORN_LEFT");     //NB: Horn never gets quieter - safety issue
    m_HornQueue.volRight = getConfigVal("VOL_HORN_RIGHT");


    if (m_volumeHalfBtn.IsPressed == true)     //Now halve config values if half volume set
    {
        m_EngineQueue.volLeft = (m_EngineQueue.volLeft >> 1);
        m_EngineQueue.volRight = (m_EngineQueue.volRight >> 1);

        m_AirCompQueue.volLeft = (m_AirCompQueue.volLeft >> 1);
        m_AirCompQueue.volRight = (m_AirCompQueue.volRight >> 1);

        m_DynBrakeQueue.volLeft = (m_DynBrakeQueue.volLeft >> 1);
        m_DynBrakeQueue.volRight = (m_DynBrakeQueue.volRight >> 1);

        m_TractionQueue.volLeft =  (m_TractionQueue.volLeft >> 1);
        m_TractionQueue.volRight = (m_TractionQueue.volRight >> 1);
    }
    else if (m_volumeOffBtn.IsPressed == true)      //Clear sound queues only (No point in queing sounds or changing volumes!)
    {
        m_EngineQueue.volLeft = 2;
        m_EngineQueue.volRight = 2;

        m_AirCompQueue.volLeft = 2;
        m_AirCompQueue.volRight = 2;

        m_DynBrakeQueue.volLeft = 2;
        m_DynBrakeQueue.volRight = 2;

        m_TractionQueue.volLeft =  2;
        m_TractionQueue.volRight = 2;
    }

    // See what is playing currently and set their channel volumes accordingly (loud or med)

    if(m_EngineQueue.IsPlaying == true)
    {
       Mix_SetPanning(m_EngineQueue.channel,m_EngineQueue.volLeft,m_EngineQueue.volRight);
    }

    if(m_AirCompQueue.IsPlaying == true)
    {
       Mix_SetPanning(m_AirCompQueue.channel,m_AirCompQueue.volLeft,m_AirCompQueue.volRight);
    }

    if(m_DynBrakeQueue.IsPlaying == true)
    {
       Mix_SetPanning(m_DynBrakeQueue.channel,m_DynBrakeQueue.volLeft,m_DynBrakeQueue.volRight);
    }

    if(m_TractionQueue.IsPlaying == true)
    {
       Mix_SetPanning(m_TractionQueue.channel,m_TractionQueue.volLeft,m_TractionQueue.volRight);
    }
}

/*********************************************
*
* Initializes the Audio system(s)
*
*********************************************/
int initAudio(void)
{

    #ifdef _WIN32
        SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING,"1");
    #endif // _WIN32
    if(Mix_OpenAudio(LC_SOUND_SAMPLE_RATE, LC_SOUND_FORMAT, LC_NUM_CHANNELS, LC_CHUNK_SIZE))
	{
        fprintf(stderr,"Unable to initialize SDL mixer: %s\n",SDL_GetError());
        return 1;
    }

    const SDL_version *link_version=Mix_Linked_Version();
    fprintf(stderr,"SDL_mixer version %d.%d.%d\n",link_version->major, link_version->minor, link_version->patch);

	Mix_AllocateChannels(LC_MAX_CHANNELS);


	if (setFilePath(getConfigStr("SOUND_FILE_PATH"),false)){
		return 1;
	}

	//Load the sound samples using the macro - throw errors on any files not found
	LC_LOADWAV(F_START,SF_START);
	LC_LOADWAV(F_IDLE,SF_IDLE);
	LC_LOADWAV(F_NOTCH1,SF_NOTCH1);
	LC_LOADWAV(F_NOTCH2,SF_NOTCH2);
	LC_LOADWAV(F_NOTCH3,SF_NOTCH3);
	LC_LOADWAV(F_NOTCH4,SF_NOTCH4);
	LC_LOADWAV(F_NOTCH5,SF_NOTCH5);
	LC_LOADWAV(F_NOTCH6,SF_NOTCH6);
	LC_LOADWAV(F_NOTCH7,SF_NOTCH7);
	LC_LOADWAV(F_NOTCH8,SF_NOTCH8);
	LC_LOADWAV(F_REVUP,SF_REVUP);
	LC_LOADWAV(F_REVDOWN, SF_REVDOWN);
	LC_LOADWAV(F_HORN, SF_HORN);
	LC_LOADWAV(F_HORN_ST, SF_HORN_ST);
	LC_LOADWAV(F_HORN_END, SF_HORN_END);
	LC_LOADWAV(F_BELL, SF_BELL);
	LC_LOADWAV(F_DYNBK_ST,SF_DYNBK_ST);
	LC_LOADWAV(F_DYNBK,SF_DYNBK);
    LC_LOADWAV(F_AIRCOMP, SF_AIRCOMP);
    LC_LOADWAV(F_AIRCOMP_END, SF_AIRCOMP_END);
    LC_LOADWAV(F_AIRDRYER, SF_AIRDRYER);
	LC_LOADWAV(F_TRACTION,SF_TRACTION);

    //Ensure the queues are in their default state
	m_EngineQueue.Qlabel = Q_LABEL_E;			//Engine Q. (tags are only there to make debugging output easier to read)
	m_AirCompQueue.Qlabel = Q_LABEL_A;			//Air Comp Q
	m_DynBrakeQueue.Qlabel = Q_LABEL_D;			//Dynamic Fans
	m_HornQueue.Qlabel = Q_LABEL_H;				//Horn Q
	m_TractionQueue.Qlabel = Q_LABEL_T;			//Traction Blowers

	//Load up the Notch points in the Rev Up / Rev Down control Arrays from the config file
	m_RevUp[0] = getConfigVal("REV_UP_IDLE");
	m_RevUp[1] = getConfigVal("REV_UP_NOTCH1");
	m_RevUp[2] = getConfigVal("REV_UP_NOTCH2");
	m_RevUp[3] = getConfigVal("REV_UP_NOTCH3");
	m_RevUp[4] = getConfigVal("REV_UP_NOTCH4");
	m_RevUp[5] = getConfigVal("REV_UP_NOTCH5");
	m_RevUp[6] = getConfigVal("REV_UP_NOTCH6");
	m_RevUp[7] = getConfigVal("REV_UP_NOTCH7");
	m_RevUp[8] = getConfigVal("REV_UP_NOTCH8");

	m_RevDown[0] = getConfigVal("REV_DN_IDLE");
	m_RevDown[1] = getConfigVal("REV_DN_NOTCH1");
	m_RevDown[2] = getConfigVal("REV_DN_NOTCH2");
	m_RevDown[3] = getConfigVal("REV_DN_NOTCH3");
	m_RevDown[4] = getConfigVal("REV_DN_NOTCH4");
	m_RevDown[5] = getConfigVal("REV_DN_NOTCH5");
	m_RevDown[6] = getConfigVal("REV_DN_NOTCH6");
	m_RevDown[7] = getConfigVal("REV_DN_NOTCH7");
	m_RevDown[8] = getConfigVal("REV_DN_NOTCH8");

	m_fadeShort		= getConfigVal("FADE_SHORT");
	m_fadeSTD 		= getConfigVal("FADE_STD");
	m_fadeLong 		= getConfigVal("FADE_LONG");

    clearAllQueues();
    setSoundVolume();

	atexit(closeAudio);    					//memory will be freed when app exits properly

	return 0;
}

/*********************************************
 *
 * closes the sound module out
 *
 *********************************************/


void closeAudio(void)
{
    //Called automatically at exit DO NOT CALL FROM YOUR CODE
	freeBuffers();
	Mix_CloseAudio();
}

/*********************************************
*
* freeBuffers: Frees all the memory used to hold the samples
*
*********************************************/

void freeBuffers(void)
{
    if(m_SoundSamples != NULL)
    {
        int f;
        for (f=1; f < SF_MAX_ITEMS; f++)
        {
            Mix_FreeChunk(m_SoundSamples[f]);
        }

        fprintf(stderr,"Sound sample buffers freed\n");
    }
}


