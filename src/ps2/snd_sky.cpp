/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// snd_sky.cpp: null driver for SNDDMA

// TODO: SPU2, streaming redbook, all that.

#include "quakedef.h"

#include <audsrv.h>

int snd_inited;

namespace {
	dma_t dma;

	// Tunable parameters for the sound driver.
	constexpr auto CHANNEL_COUNT = 2;
	constexpr auto CHANNEL_BITS = 16;
	constexpr auto SAMPLE_RATE = 22050;
	constexpr auto SAMPLE_COUNT = 16384;

	// Statically allocated DMA buffer.
	unsigned char dmabuffer[SAMPLE_COUNT];

	int last_write;

	int wbufp;

}

qboolean SNDDMA_Init() {
	if(!snd_inited) {

		auto ret = audsrv_init();

		if(ret != 0)
			Sys_Error("SNDDMA_Init: audsrv_init returned %d (%s)", ret, audsrv_get_error_string());

		// create Quake adapter
		shm = &dma;

		// Initialize DMA buffer
		shm->splitbuffer = false;
		shm->samplebits = CHANNEL_BITS;
		shm->speed = SAMPLE_RATE;
		shm->channels = CHANNEL_COUNT;
		shm->samples = SAMPLE_COUNT / (shm->samplebits / 8);
		shm->samplepos = 0;
		shm->soundalive = true;
		shm->gamealive = true;
		shm->submission_chunk = 1;

		// buffer should be managed by the spu2 class thing maybe?
		// might also be worthwhile offloading some to iop but idk how i can do that well with cmake
		shm->buffer = &dmabuffer[0];

		// Initalize audsrv audio format
		audsrv_fmt_t format{};
		format.channels = CHANNEL_COUNT;
		format.bits = CHANNEL_BITS;
		format.freq = SAMPLE_RATE;

		ret = audsrv_set_format(&format);
		if(ret != 0)
			Sys_Error("SNDDMA_Init: Could not set SPU2 sample format (%d (%s))", ret, audsrv_get_error_string());

		// as a test.
		audsrv_set_volume(MAX_VOLUME);

		// we're all ready!

		snd_inited = true;
		return true;
	}

	Sys_Error("Cannot initialize sound system while it's been initialized");
}

int SNDDMA_GetDMAPos() {
	if(!snd_inited)
		return 0;

	return last_write % shm->samples;
}

void SNDDMA_Shutdown() {
	if(snd_inited) {
		snd_inited = false;

		shm->buffer = nullptr;
		shm = nullptr;
		audsrv_quit();
	}
}

/*
==============
SNDDMA_Submit
Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit() {
	int samps;
	int bsize;
	int bytes, b;
	static unsigned char writebuf[1024];
	unsigned char *p;
	int idx;
	int stop = paintedtime;
	extern int soundtime;

	// TODO: It seems like a lot of this function implements writing to a staging buffer.
	//	Audsrv has its own ring buffer mechanism, so i'd like to see about fixing it.

	if (paintedtime < wbufp)
		wbufp = 0; // reset

	bsize = shm->channels * (shm->samplebits/8);
	bytes = (paintedtime - wbufp) * bsize;

	if (!bytes)
		return;

	if (bytes > sizeof(writebuf)) {
		bytes = sizeof(writebuf);
		stop = wbufp + bytes/bsize;
	}

	p = writebuf;
	idx = (wbufp*bsize) & (32768 - 1);

	for (b = bytes; b; b--) {
		*p++ = dmabuffer[idx];
		idx = (idx + 1) & (SAMPLE_COUNT - 1);
	}

	wbufp = stop;

	// wait for the last bit of audio to complete before submitting?
	// seems to still be kinda choppy either way
	//audsrv_wait_audio(bytes);

	int res = audsrv_play_audio((const char*)writebuf, bytes);

	if(res < 0) {
		Con_Printf("SPU2: audsrv_play_audio returned negative result (%s)\n", audsrv_get_error_string());
		return;
	}

	last_write += res;

	// cap off ring buffer
	if(last_write > SAMPLE_COUNT)
		last_write = 0;
}