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


qboolean SNDDMA_Init() {
	snd_inited = true;
	return true;
}

int SNDDMA_GetDMAPos() {
	return 0;
}

void SNDDMA_Shutdown(void)
{
	if (snd_inited)
	{
		snd_inited = false;
	}
}

/*
==============
SNDDMA_Submit
Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit(void)
{
}