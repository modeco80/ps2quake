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
// sys_sky.cpp - PS2 system driver

#include "quakedef.h"


// PS2SDK
#include <iopcontrol.h>
#include <iopheap.h>
#include <kernel.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <sio.h>

/** *The* game loop instance. */
static struct QuakeGameLoop* gGameLoop;

/**
 * This class controls the Quake gameloop
 * TODO: implement in another file
 */
struct QuakeGameLoop {
	QuakeGameLoop() {
		gGameLoop = this;
	}

	~QuakeGameLoop() {
		gGameLoop = nullptr;
	}

	bool Init(int argc, char** argv) {
		quakeparms_t parms;

		// ?: Realistically, we could probably go without calling malloc at all
		// since the ee kernel pretty much gives us full reign of the memory except for like 1mb.
		// However if some libc function calls malloc internally then that risks
		// memory corruption. So malloc()'s fine for now.
		parms.memsize = 8 * 1024 * 1024;
		parms.membase = malloc(parms.memsize);

		if(!parms.membase)
			return false;

		parms.basedir = ".";

		COM_InitArgv(argc, argv);

		parms.argc = com_argc;
		parms.argv = com_argv;

		Sys_Printf("QuakeGameLoop::Init: Quake Host_Init\n");
		Host_Init(&parms);
		Sys_Printf("QuakeGameLoop::Init: Quake Host_Init done\n");

		return true;
	}

	void DoUntilExit() {
		while(!shouldExit) {
			DoOne();
		}
	}

	void PostExit() {
		if(!shouldExit)
			shouldExit = true;
	}

   private:
	/**
	 * Do one iteration of the gameloop.
	 */
	void DoOne() {
		// TODO: We'll need to use aforementioned timer code to pace the game loop
		// properly.
		// It should be members of GameLoop.
		Host_Frame(0.1);
	}

	bool shouldExit { false };
};

// for some reason this has to be wrapped in its own extern "C" block.
extern "C" {
qboolean isDedicated;
}

/*
===============================================================================

FILE IO

===============================================================================
*/
// TODO: this could totally be made better.
#define MAX_HANDLES 10
FILE* sys_handles[MAX_HANDLES];

// Allocate a file handle. Returns it.
int alloc_handle() {
	for(int i = 1; i < MAX_HANDLES; i++)
		if(!sys_handles[i])
			return i;
	Sys_Error("out of free file handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength(FILE* f) {
	int pos;
	int end;

	pos = ftell(f);
	fseek(f, 0, SEEK_END);
	end = ftell(f);
	fseek(f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead(const char* path, int* hndl) {
	FILE* f;
	int i;

	i = alloc_handle();

	f = fopen(path, "rb");
	if(!f) {
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;

	return filelength(f);
}

int Sys_FileOpenWrite(const char* path) {
	FILE* f;
	int i;

	i = alloc_handle();

	f = fopen(path, "wb");
	if(!f)
		Sys_Error("Error opening %s: %s", path, strerror(errno));
	sys_handles[i] = f;

	return i;
}

void Sys_FileClose(int handle) {
	if(handle < 1 || handle > MAX_HANDLES)
		Sys_Error("Sys_FileClose: invalid file handle");

	if(sys_handles[handle] != nullptr) {
		fclose(sys_handles[handle]);
		sys_handles[handle] = NULL;
	} else {
		Sys_Error("Sys_FileClose: trying to close a non-existant file handle");
	}
}

void Sys_FileSeek(int handle, int position) {
	fseek(sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead(int handle, void* dest, int count) {
	return fread(dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite(int handle, void* data, int count) {
	return -1; // No-op. Leaving original code here just in case we wanna reenable.
			   // return fwrite (data, 1, count, sys_handles[handle]);
}

int Sys_FileTime(const char* path) {
	FILE* f;

	f = fopen(path, "rb");
	if(f) {
		fclose(f);
		return 1;
	}

	return -1;
}

void Sys_mkdir(const char* path) {
}

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_Vprintf(const char* fmt, va_list val) {
	static char buffer[2048];
	vsnprintf(buffer, 2048, fmt, val);
	sio_putsn(buffer);
}

SYS_NORETURN void Sys_Error(const char* error, ...) {
	va_list argptr;

	// TODO: Probably put up a primitive or draw directly on fb without clearing
	Sys_Printf("Sys_Error: ");
	va_start(argptr, error);
	Sys_Vprintf(error, argptr);
	va_end(argptr);
	Sys_Printf("\n");

	// Spin so someone with a debugger can see:
	while(1)
		;

	// exit fast
	// exit (1);
}

void Sys_Printf(const char* fmt, ...) {
	va_list argptr;

	static char buffer[2048];

	va_start(argptr, fmt);
	// vprintf (fmt,argptr);
	Sys_Vprintf(fmt, argptr);
	va_end(argptr);
}

void Sys_Quit() {
	if(!gGameLoop)
		Sys_Error("no gameloop instance? how's that work");

	// Tell the gameloop nicely to stop iterating
	gGameLoop->PostExit();
}

// TODO: We can probably use either the EE timers
// or the performance timer to get elapsed delta time.
double Sys_FloatTime() {
	static double t;

	t += 0.1;

	return t;
}

// Seems to be for DS stdin.
// Don't need that right now.
char* Sys_ConsoleInput() {
	return nullptr;
}

// Where we could probably hack-in
void Sys_SendKeyEvents() {
}

void ResetIOP() {
	SifInitRpc(0);

	while(!SifIopReset("", 0))
		;
	while(!SifIopSync())
		;

	SifInitRpc(0);
}

void LoadIOPModules() {
	ResetIOP();
	SifInitIopHeap();
	// SifLoadFileInit();

	// Load ROM IOP modules.
	SifLoadModule("rom0:SIO2MAN", 0, nullptr);
	SifLoadModule("rom0:MCMAN", 0, nullptr);
	SifLoadModule("rom0:MCSERV", 0, nullptr);
	SifLoadModule("rom0:PADMAN", 0, nullptr);

	// TODO: load PS2SDK modules
}

int main(int argc, char** argv) {
	QuakeGameLoop gameLoop;

	// Reset IOP and load modules
	// Should we do this in QuakeGameLoop?
	ResetIOP();
	LoadIOPModules();

	if(!gameLoop.Init(argc, argv))
		Sys_Error("Could not initalize Quake engine");

	// Start kicking off the game loop
	gameLoop.DoUntilExit();

	// If we get here, the gameloop was posted to exit.
	// Sys_Exit() is called when all game services have also went away,
	// so we can just return!

	return 0;
}
