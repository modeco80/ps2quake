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

int snd_inited;

namespace {
	dma_t dma;
}

qboolean SNDDMA_Init() {
	if(!snd_inited) {
		snd_inited = true;

		// create spu2 wrapper object thing..

		// create Quake adapter
		shm = &dma;

		// Initialize DMA buffer
		shm->splitbuffer = false;
		shm->samplebits = 16;
		shm->speed = 22050;
		shm->channels = 2;
		shm->samples = 32768;
		shm->samplepos = 0;
		shm->soundalive = true;
		shm->gamealive = true;
		shm->submission_chunk = 1;

		// buffer should be managed by the spu2 class thing maybe?
		// might also be worthwhile offloading some to iop but idk how i can do that well with cmake
		shm->buffer = (byte*)Hunk_AllocName((shm->samples * shm->channels), "ee spu2 buf");

		if(!shm->buffer)
			Sys_Error("Could not allocate EE-side SPU2 buffer!");

		return true;
	}

	Sys_Error("Cannot initialize sound system while it's been initialized");
}

int SNDDMA_GetDMAPos() {
	return 0;
}

void SNDDMA_Shutdown(void) {
	if(snd_inited) {
		snd_inited = false;

		shm->buffer = nullptr;
		shm = nullptr;
	}
}

/*
==============
SNDDMA_Submit
Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit(void) {
	// This is where we'd send the sound to SPU2.
}