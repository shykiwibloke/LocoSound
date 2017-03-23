//
//  main.c
//  Loco Control Sound Manager
//
//  Created by Chris Draper on 19/09/14.
//  Copyright (c) 2014 Winter Creek. All rights reserved.
//

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


int			     m_SndThrottlePos = -1;				     //Module store for our current internal setting. Used to detect change arriving
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

	//As Idle is treated as 'trottle' position - need a way of terminating dynamic brake fans when done
    if (g_LC_ControlState.ThrottleActive && g_LC_ControlState.ThrottlePos != m_SndThrottlePos)
	{
		changeThrottle();
	}

    if (g_LC_ControlState.DynBrakeActive && g_LC_ControlState.DynBrakePos != m_SndDynBrakePos)
	{
		changeDynamic();
	}

	if (g_LC_ControlState.HornPressed != m_SndHornPressed)
	{
		changeHorn();
	}
	if (g_LC_ControlState.ThrottlePos == 0 && g_LC_ControlState.DynBrakePos ==0)     //if throttle idle (i.e. engine has started already)
	{
		changeCompressor();                     //compressor runs on random time
	}

	//Go service the channels that are running (incl those just changed)
	serviceChannel(&m_EngineQueue);
	serviceChannel(&m_TractionQueue);
	serviceChannel(&m_DynBrakeQueue);
	serviceChannel(&m_HornQueue);
	serviceChannel(&m_AirCompQueue);

}

/********************************************
*
* changeThrottle - handles throttle setting changes
*
*********************************************/

void changeThrottle(void)
{

	fprintf(stderr,"\n_______________\n\n");    //signal start of new set of commands
	fprintf(stderr, "THROTTLE CHANGE\n");

	//Handle throttle notch change up or down - also handles start and stop

	if (m_EngineQueue.IsPlaying){
		fadeOutQueue(&m_EngineQueue,m_fadeSTD);  //force current sounds to fade out gradually
	}

	//clear out current settings
	clearQueue(&m_EngineQueue);

	if(g_LC_ControlState.ThrottlePos > m_SndThrottlePos)   //start or speed up
	{

		if (m_SndThrottlePos <= -1)   //special case - startup sequence requested
		{
			queueSound(&m_EngineQueue,0,SF_BELL,0,0,LC_PLAY_ONCE);
			queueSound(&m_EngineQueue,1,SF_START,0,0,LC_PLAY_ONCE);    //todo - dont set 'idle' until passed this point
			queueSound(&m_EngineQueue,2,SF_IDLE,0,0,LC_PLAY_LOOP);

		}
		else   //General Acceleration handling
		{
			//Queue appropriate rev-up sequence as well as new throttle setting.
			fprintf(stderr,"_____addMessageLine("");\n\nRev up from %d to %d\n",m_SndThrottlePos, g_LC_ControlState.ThrottlePos);
			queuePartSound(&m_EngineQueue, 0, SF_REVUP, m_RevUp[m_SndThrottlePos], m_RevUp[g_LC_ControlState.ThrottlePos], m_fadeShort, m_fadeSTD, LC_PLAY_ONCE);
			queueSound(&m_EngineQueue,1,g_LC_ControlState.ThrottlePos,m_fadeSTD,m_fadeSTD,LC_PLAY_LOOP);


			//Traction Motor Blowers
			if  (g_LC_ControlState.ThrottlePos > 6)				//traction sound should play when traction motors are working
            {
				clearQueue(&m_TractionQueue);
				queueSound(&m_TractionQueue, 0, SF_TRACTION, m_fadeLong, 0, LC_PLAY_LOOP);
				playQueueItem(&m_TractionQueue);

			}
		}
	}
	else   //slow down or stop
	{
		if ( g_LC_ControlState.ThrottlePos == -1)  //special case - stop engine
		{
			//todo - engine + traction stop
		}
		else  //General Deceleration handling
		{
			fprintf(stderr,"_____\n\nRev down from %d to %d\n",m_SndThrottlePos, g_LC_ControlState.ThrottlePos);
			queuePartSound(&m_EngineQueue, 0, SF_REVDOWN, m_RevDown[m_SndThrottlePos], m_RevDown[g_LC_ControlState.ThrottlePos],  m_fadeShort, m_fadeSTD, LC_PLAY_ONCE);
			queueSound(&m_EngineQueue,1,g_LC_ControlState.ThrottlePos,m_fadeSTD,m_fadeSTD,LC_PLAY_LOOP);

			//Traction Motor Blowers
			if (g_LC_ControlState.ThrottlePos < 3 && m_TractionQueue.IsPlaying)
			{
				fadeOutQueue(&m_TractionQueue,m_fadeSTD);  //force current sounds to fade out gradually
			}
		}
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

    //if dynamic brake is NOT currently playing and lever is NOT at idle
	if( !m_DynBrakeQueue.IsPlaying && g_LC_ControlState.DynBrakePos != 0)
	{
		//startup dyn braking - stays the same irrespective of dyn brake handle position
		clearQueue(&m_DynBrakeQueue);
		queueSound(&m_DynBrakeQueue,0,SF_DYNBK_ST,0,m_fadeSTD,LC_PLAY_ONCE);
		queueSound(&m_DynBrakeQueue,1,SF_DYNBK,m_fadeSTD,0,LC_PLAY_LOOP);
		playQueueItem(&m_DynBrakeQueue);

	}

    //Traction Motor Blowers

    //if currently < 5 and moving to 5 or greater
    if  (g_LC_ControlState.DynBrakePos >= 5 && m_SndDynBrakePos < 5)				//traction sound should play when traction motors are working
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
            fadeOutQueue(&m_TractionQueue,m_fadeSTD);  //force current sounds to fade out gradually
        }
    }
    //if moving to zero
    else if  (g_LC_ControlState.DynBrakePos == 0)
    {
        fadeOutQueue(&m_DynBrakeQueue,m_fadeLong);  //force current sounds to fade out gradually
        fadeOutQueue(&m_TractionQueue,m_fadeSTD);  //force current sounds to fade out gradually
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
    if (m_SndHornPressed)  //already playing - shut it up
	{
		fadeOutQueue(&m_HornQueue,m_fadeShort);  //force current sounds to fade out
		clearQueue(&m_HornQueue);
		queueSound(&m_HornQueue, 0, SF_HORN_END, 100, 0, LC_PLAY_ONCE);
		playQueueItem(&m_HornQueue);
        fprintf(stderr,"J Press\n");
		m_SndHornPressed = false;

	}
	else  //horn not playing yet - let her rip!
	{

		clearQueue(&m_HornQueue);
		queueSound(&m_HornQueue, 0, SF_HORN_ST, 0, 100, LC_PLAY_ONCE);
		queueSound(&m_HornQueue, 1, SF_HORN, 100, 100, LC_PLAY_LOOP);
		playQueueItem(&m_HornQueue);
        fprintf(stderr,"H Press\n");
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

    int value = 2 + rand() % 6;            //2 - 7 mins range seed
    static Uint32 torun = 60000;

    if SDL_TICKS_PASSED(SDL_GetTicks(),torun)    //TODO - dont allow to play while revving down
    {
        //start the air compressor sound
        fprintf(stderr,"Compressor triggered with random value of %d", value);
		clearQueue(&m_AirCompQueue);
		queueSound(&m_AirCompQueue, 0, SF_AIRCOMP, 0, 100, LC_PLAY_ONCE);
		playQueueItem(&m_AirCompQueue);

        //set the next time to run
        torun = SDL_GetTicks()+ (value * 60000);   //set current time plus value * one minute
    }

}

/********************************************
*
* queueSound - add a sound and its parameters to the specified queue
*
*********************************************/

 void queueSound(LC_SoundQueue_t *pQ,
					int index,
					LC_SoundFile_t sound,
					Uint32 fadeIn,
					Uint32 fadeOut,
					int loopCount)
{

	pQ->soundChunk[index] = Mix_QuickLoad_RAW(m_SoundSamples[sound]->abuf,m_SoundSamples[sound]->alen);

	pQ->fadeInTime[index] = fadeIn;
	pQ->fadeOutTime[index] = fadeOut;
	pQ->loopCount[index] = loopCount;

}

/*********************************************
 *
 * queuePartSound - add part of a sound and its parameters to the specified queue
 *
 *********************************************/

void queuePartSound(LC_SoundQueue_t *pQ,
				int index,
				LC_SoundFile_t sound,
				Uint32 startPos,
				Uint32 endPos,
				Uint32 fadeIn,
				Uint32 fadeOut,
				int loopCount )
{

	Uint32 bufstart = (Uint32) m_SoundSamples[sound]->abuf;
	Uint32 slen = m_SoundSamples[sound]->alen;
	Uint32 reqlen = 0;

	//endpos check allows caller to specify a huge number to ensure end is reached.
	//check for and trim to sound length before doing sample math to get requested length
	if (endPos > slen) endPos = slen;

	bufstart += startPos;					//startPos is the offset from the buffer start
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
		fprintf(stderr,"Specified mix_chunk buffer, requested length and/or start/stop positions invalid\n");
	else
	{

		pQ->soundChunk[index] =  Mix_QuickLoad_RAW((Uint8*) bufstart,reqlen);
		pQ->fadeInTime[index] = fadeIn;
		pQ->fadeOutTime[index] = fadeOut;
		pQ->loopCount[index] = loopCount;
	}

}

/*********************************************
*
* StartQueuePlaying
*
*********************************************/
void playQueueItem(LC_SoundQueue_t *pQ)
{

   fprintf(stderr,"Starting to play %s sound %d\n",pQ->Qlabel,pQ->currentItem);

	//set fade out parameters for later checking (time based on actual time NOW - being the time we started this playing)
	if ((pQ->loopCount[pQ->currentItem] > -1) && (pQ->soundChunk[pQ->currentItem]->alen > 0))
	{
		pQ->currentPlayEnd = SDL_GetTicks();
		pQ->currentPlayEnd += ((pQ->soundChunk[pQ->currentItem]->alen / LC_SOUND_SAMPLE_RATE )*250); //milliseconds from bytes
		pQ->currentFadeStart = pQ->currentPlayEnd - pQ->fadeOutTime[pQ->currentItem];
	} else {
		pQ->currentPlayEnd = 99999999;
		pQ->currentFadeStart = 99999999;
	}

 	//Play first queue sound on next available channel
	pQ->channel = Mix_FadeInChannel(-1, pQ->soundChunk[pQ->currentItem],pQ->loopCount[pQ->currentItem], pQ->fadeInTime[pQ->currentItem]);


	if (pQ->channel == -1)
	{
		pQ->IsPlaying = false;
		fprintf(stderr,"Error playing sound %d on channel %d for %s\n",pQ->currentItem, pQ->channel,pQ->Qlabel);
	}
	else
	{
        //Set the overall volume of the channel properly here according to user config settings
        Mix_SetPanning(pQ->channel,pQ->volLeft,pQ->volRight);

		pQ->IsPlaying = true;
		fprintf(stderr,"%s sound %d assigned channel %d and playing OK \n",pQ->Qlabel,pQ->currentItem, pQ->channel);

	}

}

/*********************************************
 *
 * fadeOutQueue Playing queue gets faded out
 *
 *********************************************/
void fadeOutQueue(LC_SoundQueue_t *pQ,Uint32 fadeOut)
{

	pQ->currentFadeStart = SDL_GetTicks();         //Trigger the fade out of the sound in the service channel code
	pQ->fadeOutTime[pQ->currentItem] = fadeOut;
	pQ->currentPlayEnd = SDL_GetTicks() + fadeOut; //Trigger the end of the sound in the service channel code
	pQ->loopCount[pQ->currentItem] = 0;
	pQ->soundChunk[(pQ->currentItem)+1] = NULL;

	fprintf(stderr,"Stopping %s sound on channel %d, index %d for %d ms\n",pQ->Qlabel, pQ->channel, pQ->currentItem, pQ->fadeOutTime[pQ->currentItem]);
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
			//if (SDL_TICKS_PASSED(currentTime,pQ->currentPlayEnd)) //check end b4 fade - duplicate!
			fprintf(stderr,"calculated time expired\n");

			if ( !Mix_Playing(pQ->channel)) //check end b4 fade
			{
				fprintf(stderr,"%s chan %d confirmed expired\n",pQ->Qlabel,pQ->channel);

			}
			finishPlayingSound(pQ);

		}
		//has current sound got to the point of fading out?
		else if (SDL_TICKS_PASSED(currentTime, pQ->currentFadeStart))
		{
			fprintf(stderr,"Fading out %s channel %d, index %d for %d ms\n", pQ->Qlabel, pQ->channel, pQ->currentItem, pQ->fadeOutTime[pQ->currentItem]);//IF the next sound is zero length AND we are not going to be at the end of the queue
			Mix_FadeOutChannel(pQ->channel, pQ->fadeOutTime[pQ->currentItem]+1);
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

   	int next = pQ->currentItem +1;

	if(pQ->soundChunk[next] == NULL && next < LC_SOUND_QUEUE_MAX)
	{
		//If sound is complete and there are no more in this queue - then shut the queue down here even though we might be still fading
		pQ->IsPlaying = false;
	}
	else if(next == LC_SOUND_QUEUE_MAX)  //IF we are at the end of the queue entirely
	{
		pQ->IsPlaying = false;
	}
	else
	{
		// there are more in this queue - so go start the next one.
		pQ->currentItem++;
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
                ptr = Q_LABEL_U;              //Unkown or Orphan

            snprintf(m_msgTempLine,MSG_RECT_LINE_LENGTH,"%d channel is playing for %s", f, ptr);
            addMessageLine(m_msgTempLine);

        }
        else  //if NOT Mix_playing
        {
            snprintf(m_msgTempLine,MSG_RECT_LINE_LENGTH,"%d channel is free", f);
            addMessageLine(m_msgTempLine);
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

    for(f=0;f<LC_MAX_CHANNELS;f++)
    {
               Mix_FadeOutChannel(f,1);
    }

    //Ensure the queues are in their default state
	m_EngineQueue.Qlabel = Q_LABEL_E;				//Engine Q. (tags are only there to make debugging output easier to read)
	m_EngineQueue.volLeft = getConfigVal("VOL_ENGINE_LEFT");
	m_EngineQueue.volRight = getConfigVal("VOL_ENGINE_RIGHT");
	clearQueue(&m_EngineQueue);
	m_AirCompQueue.Qlabel = Q_LABEL_A;				//Air Comp Q
	m_AirCompQueue.volLeft = getConfigVal("VOL_AIRCOMPRESSOR_LEFT");
    m_AirCompQueue.volRight = getConfigVal("VOL_AIRCOMPRESSOR_RIGHT");
	clearQueue(&m_AirCompQueue);
	m_DynBrakeQueue.Qlabel = Q_LABEL_D;			//Dynamic Fans
	m_DynBrakeQueue.volLeft = getConfigVal("VOL_DYNAMIC_LEFT");
    m_DynBrakeQueue.volRight = getConfigVal("VOL_DYNAMIC_RIGHT");
	clearQueue(&m_DynBrakeQueue);
	m_HornQueue.Qlabel = Q_LABEL_H;				//Horn Q
	m_HornQueue.volLeft = getConfigVal("VOL_HORN_LEFT");
    m_HornQueue.volRight = getConfigVal("VOL_HORN_RIGHT");
	clearQueue(&m_HornQueue);
	m_TractionQueue.Qlabel = Q_LABEL_T;			//Traction Blowers
	m_TractionQueue.volLeft = getConfigVal("VOL_TRACTION_LEFT");
    m_TractionQueue.volRight = getConfigVal("VOL_TRACTION_RIGHT");
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
		fprintf(stderr,"%s channel %d cleared\n",pQ->Qlabel,pQ->channel);
 		pQ->channel = -1;         //set so next sound will be allocated a fresh mix channel

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
    fprintf(stderr,"Successfully Opened with SDL_mixer version %d.%d.%d\n",link_version->major, link_version->minor, link_version->patch);

	Mix_AllocateChannels(LC_MAX_CHANNELS);


	if (setDataFilePath()){
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
	LC_LOADWAV(F_TRACTION,SF_TRACTION);

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
	fprintf(stderr,"Sound Closed\n");

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

/*********************************************
*
*  Call back Hack
*
*********************************************/

void handleSoundCallback(int channel)
{
	//Used for debug only - inserts message in debug stream to show Mixer has recognized channel as completed playing
    fprintf(stderr,"Callback for channel %d\n",channel);
}


