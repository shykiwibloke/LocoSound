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

// LC_Sounds holds information about the memory copies of all sound samples
// Individual Sounds can be indexed using the enumeration LC_SoundFile_t

Mix_Chunk		 *m_SoundSamples[SF_MAX_ITEMS];			//Holds all the sound samples and related information in memory for fast access
LC_SoundQueue_t  m_EngineQueue;							//Used to contain each string of samples to be played
LC_SoundQueue_t  m_DynBrakeQueue;
LC_SoundQueue_t  m_HornQueue;
LC_SoundQueue_t	 m_AirCompQueue;
LC_SoundQueue_t  m_TractionQueue;

LC_RevUp_t		 m_RevUp[] = {RU_IDLE,RU_NOTCH1,RU_NOTCH2,RU_NOTCH3,RU_NOTCH4,RU_NOTCH5,RU_NOTCH6,RU_NOTCH7,RU_NOTCH8};
LC_RevDown_t	 m_RevDown[] = {RD_IDLE,RD_NOTCH1,RD_NOTCH2,RD_NOTCH3,RD_NOTCH4,RD_NOTCH5,RD_NOTCH6,RD_NOTCH7,RD_NOTCH8};


int			     m_SndThrottlePos = -1;				     //Module store for our current internal setting. Used to detect change arriving
bool			 m_SndHornPressed = 0;					 //ditto
int			     m_SndDynBrakePos = 0;					 //ditto



/*********************************************
*
* soundService is the main service routine
*
*********************************************/
void soundService()
{
	// main hook to ensure sound is going. call frequently

	//Check for changes to process

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
	if (g_LC_ControlState.ThrottlePos == 0)     //if throttle idle
	{
		changeCompressor();
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
		fadeOutQueue(&m_EngineQueue,1500);  //force current sounds to fade out gradually
	}

	//clear out current settings
	clearQueue(&m_EngineQueue);

	if(g_LC_ControlState.ThrottlePos > m_SndThrottlePos)   //start or speed up
	{

		if (m_SndThrottlePos <= -1)   //special case - startup sequence requested
		{
			queueSound(&m_EngineQueue,0,SF_BELL,0,0,LC_PLAY_ONCE,LC_VOLUME_MAX);
			queueSound(&m_EngineQueue,1,SF_START,0,0,LC_PLAY_ONCE,LC_VOLUME_MAX);
			queueSound(&m_EngineQueue,2,SF_IDLE,0,0,LC_PLAY_LOOP,LC_VOLUME_MAX);

		}
		else   //General Acceleration handling
		{
			//Queue appropriate rev-up sequence as well as new throttle setting.
			fprintf(stderr,"_____\n\nRev up from %d to %d\n",m_SndThrottlePos, g_LC_ControlState.ThrottlePos);
			queuePartSound(&m_EngineQueue, 0, SF_REVUP, m_RevUp[m_SndThrottlePos], m_RevUp[g_LC_ControlState.ThrottlePos], 1000, 1500, LC_PLAY_ONCE);
			queueSound(&m_EngineQueue,1,g_LC_ControlState.ThrottlePos,1500,1000,LC_PLAY_LOOP,LC_VOLUME_MAX);


			//Traction Motor Blowers
			if  (g_LC_ControlState.ThrottlePos > 6)				//traction sound should play when traction motors are working
			{
				clearQueue(&m_TractionQueue);
				queueSound(&m_TractionQueue, 0, SF_TRACTION, 10000, 0, LC_PLAY_LOOP,LC_VOLUME_HALF);
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
			queuePartSound(&m_EngineQueue, 0, SF_REVDOWN, m_RevDown[m_SndThrottlePos], m_RevDown[g_LC_ControlState.ThrottlePos],  500, 2000, LC_PLAY_ONCE);
			queueSound(&m_EngineQueue,1,g_LC_ControlState.ThrottlePos,1500,1000,LC_PLAY_LOOP,LC_VOLUME_MAX);

			//Traction Motor Blowers
			if (g_LC_ControlState.ThrottlePos < 3 && m_TractionQueue.IsPlaying)
			{
				fadeOutQueue(&m_TractionQueue,10000);  //force current sounds to fade out gradually
                fprintf(stderr,"fading traction for engine for ten seconds now\n");
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

	if(!m_DynBrakeQueue.IsPlaying && g_LC_ControlState.DynBrakePos > 0)  // if dyn brake is active - we are already playing the sound?
	{
		//startup dyn braking
		clearQueue(&m_DynBrakeQueue);
		queueSound(&m_DynBrakeQueue,0,SF_DYNBK_ST,0,1000,LC_PLAY_ONCE,LC_VOLUME_MAX);
		queueSound(&m_DynBrakeQueue,1,SF_DYNBK,1000,0,LC_PLAY_LOOP,LC_VOLUME_MAX);
		playQueueItem(&m_DynBrakeQueue);

	}

	else if(g_LC_ControlState.DynBrakePos != m_SndDynBrakePos)   //Alter braking effort
	{

		//Traction Motor Blowers
		if  (g_LC_ControlState.DynBrakePos > 5)				//traction sound should play when traction motors are working
		{
			clearQueue(&m_TractionQueue);
			queueSound(&m_TractionQueue, 0, SF_TRACTION, 5000, 0, LC_PLAY_LOOP,LC_VOLUME_MAX);
			playQueueItem(&m_TractionQueue);

		}
		else if  (g_LC_ControlState.DynBrakePos > 0)		//traction sound should play when traction motors are working
		{
			clearQueue(&m_TractionQueue);
			queueSound(&m_TractionQueue, 0, SF_TRACTION, 5000, 0, LC_PLAY_LOOP,LC_VOLUME_HALF);
			playQueueItem(&m_TractionQueue);

		}
		else   //at idle
		{
			if (m_DynBrakeQueue.IsPlaying){
				fadeOutQueue(&m_DynBrakeQueue,6000);  //force current sounds to fade out gradually
			}
			if (m_TractionQueue.IsPlaying){
				fadeOutQueue(&m_TractionQueue,6000);  //force current sounds to fade out gradually
			}
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
		if (m_SndHornPressed)  //already playing - shut it up
	{
		fadeOutQueue(&m_HornQueue,500);  //force current sounds to fade out
		clearQueue(&m_HornQueue);
		queueSound(&m_HornQueue, 0, SF_HORN_END, 100, 0, LC_PLAY_ONCE,LC_VOLUME_MAX);
		playQueueItem(&m_HornQueue);
		m_SndHornPressed = false;

	}
	else  //horn not playing yet - let her rip!
	{

		clearQueue(&m_HornQueue);
		queueSound(&m_HornQueue, 0, SF_HORN_ST, 0, 100, LC_PLAY_ONCE,LC_VOLUME_MAX);
		queueSound(&m_HornQueue, 1, SF_HORN, 100, 100, LC_PLAY_LOOP,LC_VOLUME_MAX);
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

	//todo - random number + min time check here
	//  if true then start the air compressor sound

	//use srand(seed); to set range and rand(); to get a random number

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
					int loopCount,
					Uint8 volume)
{
	/*
	pQ->soundChunk[index].allocated = 0;  //dont deallocate as we are peeking into another chunk's buffer.
	pQ->soundChunk[index].volume = volume;
	pQ->soundChunk[index].abuf = m_SoundSamples[sound]->abuf;
	pQ->soundChunk[index].alen = m_SoundSamples[sound]->alen;
*/

	//Mix_Chunk * QueueChunk = pQ->soundChunk[index];

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

	/*   Debug code only - TODO - place in bounds checking once verified*/
	fprintf(stderr,"_______________\nFile buffer starts at \t\t %p \n",m_SoundSamples[sound]->abuf);
	fprintf(stderr,"File length is \t\t %d bytes \n",slen);
	fprintf(stderr,"File ends at address \t\t %p bytes \n",m_SoundSamples[sound]->abuf + slen);
	fprintf(stderr,"Play starting at offset \t\t %d bytes \n",startPos);
	fprintf(stderr,"So Play starting address is \t\t %p bytes \n",(void *)bufstart);
	fprintf(stderr,"Play length is \t\t %d bytes \n",reqlen);
	fprintf(stderr,"Play ending address is \t\t %X \n\n",bufstart + reqlen);

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

   fprintf(stderr,"Starting to play Queue %c sound %d\n",pQ->Q_tag,pQ->currentItem);

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
		fprintf(stderr,"Error playing sound %d for channel %d for Queue %c\n",pQ->currentItem, pQ->channel,pQ->Q_tag);
	}
	else
	{
		pQ->IsPlaying = true;
		fprintf(stderr,"Queue %c Sound %d assigned channel %d and playing OK \n",pQ->Q_tag,pQ->currentItem, pQ->channel);

		int f = 0;

        fprintf(stderr,"\n_______________\n\n");    //signal start of new set of commands
		fprintf(stderr, "Chan Summary\n\n");


		for(f=0; f<LC_MAX_CHANNELS;f++)
        {
            if(Mix_Playing(f))
            {
                fprintf(stderr,"%d channel is playing\n", f);
            } else {
                fprintf(stderr,"%d channel is free\n", f);
            }
         }

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

	fprintf(stderr,"Stopping Queue %c with sound playing on channel %d, index %d for %d ms\n",pQ->Q_tag, pQ->channel, pQ->currentItem, pQ->fadeOutTime[pQ->currentItem]);
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
				fprintf(stderr,"Queue %c chan %d confirmed expired\n",pQ->Q_tag,pQ->channel);

//TODO - HOW BEST TO SEE CHUNK REMOVED ? FLAG SET ON CALLBACK?
//				Mix_FreeChunk(pQ->soundChunk[pQ->currentItem]);
			}
			finishPlayingSound(pQ);

		}
		//has current sound got to the point of fading out?
		else if (SDL_TICKS_PASSED(currentTime, pQ->currentFadeStart))
		{
			fprintf(stderr,"Fading out queue %c channel %d, index %d for %d ms\n", pQ->Q_tag, pQ->channel, pQ->currentItem, pQ->fadeOutTime[pQ->currentItem]);//IF the next sound is zerolength AND we are not going to be at the end of the queue
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
 * Clear Queue  - clears the specified que to default state
 *
 *********************************************/

void clearQueue(LC_SoundQueue_t *pQ)
{

	int f;

	pQ->IsPlaying = false;
	pQ->currentItem = 0;
	pQ->currentFadeStart = 0;
	pQ->currentPlayEnd = 0;

	for(f = 0;f< LC_SOUND_QUEUE_MAX;f++)  //clear the queue
	{
		pQ->soundChunk[f] = NULL;
		pQ->fadeInTime[f] = 0;
		pQ->fadeOutTime[f] = 0;
		pQ->loopCount[f] = 0;

	}
	if (pQ->channel > -1)
	{
		fprintf(stderr,"Queue %c with sound currently playing on channel %d cleared\n",pQ->Q_tag,pQ->channel);
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


    if(Mix_OpenAudio(LC_SOUND_SAMPLE_RATE, LC_SOUND_FORMAT, LC_NUM_CHANNELS, LC_CHUNK_SIZE))
	{
        fprintf(stderr,"Unable to initialize SDL mixer: %s\n",SDL_GetError());
        return 1;
    }

    fprintf(stderr,"SDL Mixer version %d.%d opened OK\n",SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION);
	Mix_AllocateChannels(LC_MAX_CHANNELS);

	Mix_ChannelFinished(handleSoundCallback);

	if (setDataFilePath()){
		return 1;
	}

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
	m_SoundSamples[SF_REVDOWN]->volume = 0x40;		//half volume
	LC_LOADWAV(F_HORN, SF_HORN);
	LC_LOADWAV(F_HORN_ST, SF_HORN_ST);
	LC_LOADWAV(F_HORN_END, SF_HORN_END);
	LC_LOADWAV(F_BELL, SF_BELL);
	LC_LOADWAV(F_DYNBK_ST,SF_DYNBK_ST);
	LC_LOADWAV(F_DYNBK,SF_DYNBK);
    LC_LOADWAV(F_AIRCOMP, SF_AIRCOMP);
	LC_LOADWAV(F_TRACTION,SF_TRACTION);

	//Ensure the queues are in their default state
	clearQueue(&m_EngineQueue);
	m_EngineQueue.Q_tag = 'E';
	clearQueue(&m_AirCompQueue);
	m_AirCompQueue.Q_tag = 'A';
	clearQueue(&m_DynBrakeQueue);
	m_DynBrakeQueue.Q_tag = 'D';
	clearQueue(&m_HornQueue);
	m_HornQueue.Q_tag = 'H';
	clearQueue(&m_TractionQueue);
	m_TractionQueue.Q_tag = 'T';

	atexit(closeAudio);    //memory will be freed when app exits properly

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
*  Call back
*
*********************************************/

void handleSoundCallback(int channel)
{
    fprintf(stderr,"Callback for channel %d\n",channel);
}


