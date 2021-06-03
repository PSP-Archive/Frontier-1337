#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

#include <SDL/SDL.h>

#include <pspkernel.h>
#include <psppower.h>

// Pour le debug
#include <pspctrl.h>
#include <pspdebug.h>

#include "main.h"
#include "audio.h"
#include "m68000.h"
#include "input.h"
#include "screen.h"

PSP_MODULE_INFO("Frontier", 0, 1, 0);
PSP_HEAP_SIZE_KB(-256);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int SetupCallbacks(void);
int exit_callback(int arg1, int arg2, void *common);
int CallbackThread(SceSize args, void *argp);


//  Initialise frontier
void Main_Init(void) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER);
	SDL_ShowCursor(SDL_DISABLE);
	Screen_Init();
	Init680x0(); // Init CPU emulation
	Audio_Init();
}


//  Un-Initialise frontier
void Main_UnInit(void) {
	Audio_UnInit();
	SDL_Quit();
}


static Uint32 vbl_callback () {
	FlagException (0);
	return 1;
}


//  Main
int main(int argc, char *argv[]) {
	// Overclock the PSP to 333MHz
	scePowerSetClockFrequency(333, 333, 166);
	// Callbacks for home button
	SetupCallbacks();
	// Init emulator system
	Main_Init();
	SDL_AddTimer (1, &vbl_callback, NULL);
	// Start the PSP joystick
	SDL_Joystick *joystick = NULL;
	joystick = SDL_JoystickOpen(0);
	// Start the audio engine
	Audio_EnableAudio(1);
	// Start emulation
	Start680x0();
	// Un-init emulation system
	SDL_JoystickClose(joystick);
	Main_UnInit();
	sceKernelExitGame();
	return(0);
}


// Exit callback
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}

// Callback thread
int CallbackThread(SceSize args, void *argp)
{
	int cbid;
	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
	return 0;
}

// Sets up the callback thread and returns its thread id
int SetupCallbacks(void)
{
	int thid = 0;
	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if(thid >= 0) sceKernelStartThread(thid, 0, 0);
	return thid;
}
