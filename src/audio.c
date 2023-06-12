/*
	wrx-engine: audio routines

	Jason A. Petrasko, muragami, muragami@wishray.com 2023

	MIT License
*/

#define DR_MP3_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION

#define DR_MP3_NO_STDIO
#define DR_FLAC_NO_STDIO
#define DR_WAV_NO_STDIO

#include "wrx.h"
#include <portaudio.h>
#include <opus/opus.h>
#include "../include/dr_wav.h"
#include "../include/dr_mp3.h"
#include "../include/dr_flac.h"

typedef struct {
	int valid;
	int muted;
	PaStream *stream;
	PaError err;
} wrxAudio;

int wrxAudioCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags, void *userData) {
	wrxAudio *a = userData; 
    float *out = (float*)output;
    unsigned int i;

	if (a->muted) {
		// we are muted, so just emit silence
		for (i = 0; i < frameCount; i++ )
    	{
        	 *out = 0.0f;  /* left */
    		out++;
         	*out = 0.0f;  /* right */
    		out++;
    	}
	} else {
		// emit silence or sample data
	}     
    
	return 0;
}

void wrxAudioStart(wrxState *p) {
	wrxAudio *a = calloc(sizeof(wrxAudio), 1);
	
	if (a == NULL) {
		return;
	}

	a->err = Pa_Initialize();
	if (a->err != paNoError) {
		return;
	}

	/* Open an audio I/O stream. */
    a->err = Pa_OpenDefaultStream(&a->stream,
                                0,          /* no input channels */
                                2,          /* stereo output */
                                paFloat32,  /* 32 bit floating point output */
                                48000,
                                (480 / 100),       /* frames per buffer, i.e. the number
                                                   of sample frames that PortAudio will
                                                   request from the callback. Many apps
                                                   may want to use
                                                   paFramesPerBufferUnspecified, which
                                                   tells PortAudio to pick the best,
                                                   possibly changing, buffer size.*/
                                wrxAudioCallback, /* this is your callback function */
                                a ); /*This is a pointer that will be passed to
                                                   your callback*/
    if (a->err != paNoError) {
    	return;
    }

    a->err = Pa_StartStream(a->stream);
	if (a->err != paNoError) {
		return;
	}


    a->valid = 1;
    a->muted = 1;
	p->audio = a;
}

void wrxAudioStop(wrxState *p) {
	wrxAudio *a = p->audio;

	Pa_AbortStream(a->stream);

	a->err = Pa_Terminate();
	if (a->err != paNoError) {

	}
}