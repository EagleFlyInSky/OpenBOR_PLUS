/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * All rights reserved, see LICENSE in OpenBOR root for details.
 *
 * Copyright (c) 2004 - 2014 OpenBOR Team
 */

// A soundblaster interface. No bugs?

#include "sblaster.h"
#include "soundmix.h"
#include "sdlport.h"

static SDL_AudioSpec cspec;
static SDL_AudioDeviceID audio_dev;
static int sample_per_byte;
static int buffsize = 4096;
static int voicevol = 15;


static void callback(void *userdata, Uint8 *stream, int len)
{
	update_sample(stream,len);
	//writeToLogFile("sb call back\n");
}

static int started;

/*#ifdef PS3
int roundDownPowerOfTwo(int samples) {
	// Public domain code from Sean Eron Anderson
	// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	int rounded = samples;
	--rounded;
	rounded |= rounded >> 1;
	rounded |= rounded >> 2;
	rounded |= rounded >> 4;
	rounded |= rounded >> 8;
	rounded |= rounded >> 16;
	++rounded;

	if (rounded != samples)
		rounded >>= 1;

	return rounded;
}
#endif*/

int SB_playstart(int bits, int samplerate)
{
	SDL_AudioSpec spec;

	spec.channels = 2;
	spec.format = bits==16?AUDIO_S16SYS:AUDIO_U8;
	spec.freq = samplerate;
	sample_per_byte = 16/bits*spec.channels;
	spec.samples = buffsize/sample_per_byte/2;
	spec.userdata = NULL;
	spec.callback = callback;

	/*SDL_PauseAudio(1);
	if (SDL_OpenAudio(&spec,&cspec)<0) return 0;
	SDL_PauseAudio(0);*/

	SDL_PauseAudioDevice(audio_dev, 1);
	if (!(audio_dev = SDL_OpenAudioDevice(NULL, 0, &spec, &cspec, 0))) return 0; //SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE
	SDL_PauseAudioDevice(audio_dev, 0);

	started = 1;
	return 1;
}

void SB_playstop()
{
	//SDL_CloseAudio();
    SDL_CloseAudioDevice(audio_dev);
}

void SB_setvolume(char dev, char volume)
{
	if(dev == SB_VOICEVOL)
	{
#ifdef GP2X
		voicevol = volume;
		voicevol = (voicevol * 100) / 15;
		gp2x_sound_set_volume(voicevol, voicevol);
#endif
		voicevol = volume;
	}
}


void SB_updatevolume(int volume)
{
#ifdef GP2X
		int lastvol = 0;
		voicevol += volume;
		if(voicevol > 15) voicevol = 15;
		if(voicevol < 0 ) voicevol = 0;
		lastvol = voicevol;
		voicevol = (voicevol * 100) / 15;
		gp2x_sound_set_volume(voicevol, voicevol);
		voicevol = lastvol;
#endif
}
