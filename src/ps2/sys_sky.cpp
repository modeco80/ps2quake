/*
Copyright (C) 1996-1997 Id Software, Inc.
Copyright (C) 2022 Lily <lily.modeco80@protonmail.ch>

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
// sys_sky.c - PS2 system driver

// HACK for now. Headers should get c guards later
extern "C" {
#include "quakedef.h"
#include "errno.h"
}

extern "C" {

qboolean isDedicated;

/*
===============================================================================

FILE IO

===============================================================================
*/

#define MAX_HANDLES             10
FILE    *sys_handles[MAX_HANDLES];

int             findhandle (void)
{
	int             i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int             pos;
	int             end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (const char *path, int *hndl)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	
	return filelength(f);
}

int Sys_FileOpenWrite (const char *path)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	sys_handles[i] = f;
	
	return i;
}

void Sys_FileClose (int handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return -1; // No-op. Leaving original code here just in case we wanna reenable.
	//return fwrite (data, 1, count, sys_handles[handle]);
}

int     Sys_FileTime (const char *path)
{
	FILE    *f;
	
	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);
		return 1;
	}
	
	return -1;
}

void Sys_mkdir (const char *path)
{
}


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

// No-op on ps2, as the EE kernel has no semblance of memory protection besides
// kernel and user memory
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_Error (const char *error, ...)
{
	va_list         argptr;

	// TODO: Probably put up a primitive or draw directly on fb without clearing
	printf ("Sys_Error: ");   
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");

	exit (1);
}

void Sys_Printf (const char *fmt, ...)
{
	va_list         argptr;
	
	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr);
}

void Sys_Quit (void)
{
	exit(0);
}

// TODO: We can probably use either the EE timers
// or performance timer.
double Sys_FloatTime (void)
{
	static double t;
	
	t += 0.1;
	
	return t;
}

// TODO: is this for egg
char *Sys_ConsoleInput (void)
{
	return NULL;
}

// TODO: When the engine calls this if we are multithreading
// we should maybe begin trying to yield to other threads? Dunno
void Sys_Sleep (void)
{
}

// Where we could probably hack-in
void Sys_SendKeyEvents (void)
{
}

// Doesn't matter for non x86, may even completely remove the functions
void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}

//=============================================================================

int main (int argc, char **argv)
{
	quakeparms_t parms;

	// ?: Realistically, we could probably go without calling malloc at all
	// since the ee kernel pretty much gives us full reign of the heap.
	// However if some libc function calls malloc internally then that risks
	// memory corruption. So malloc()'s fine for now.
	parms.memsize = 8*1024*1024;
	parms.membase = malloc (parms.memsize);
	parms.basedir = ".";

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	printf ("Host_Init\n");
	Host_Init (&parms);

	// TODO: We'll need to use aforementioned timer code to pace the game loop
	// properly.
	while (true)
	{
		Host_Frame (0.1);
	}

	return 0;
}

}

