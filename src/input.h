#ifndef _INPUT_H
#define _INPUT_H

#include <SDL/SDL_keyboard.h>

#define	SIZE_KEYBUF		16
#define SIZE_MOUSEBUF	16

typedef struct {
	unsigned char key_buf[SIZE_KEYBUF];
	unsigned char mousebut_buf[SIZE_MOUSEBUF];
	int buf_head, buf_tail;
	int mbuf_head, mbuf_tail;
	int cur_mousebut_state;
	
	// change in mouse pos since last polled, absolute position
	int motion_x, motion_y;
	int motion_xx, motion_yy;
	int abs_x, abs_y;
} INPUT;

extern INPUT input;
unsigned char Fkey;

extern void Main_EventHandler();

void Input_PressSTKey (unsigned char ScanCode, bool bPress);
void Call_GetMouseInput ();
void Call_GetKeyboardEvent ();
void Input_ButtonPress (unsigned char button);
void Input_ButtonRelease (unsigned char button);
void Input_FkeyPress (unsigned char simFkey);
void Input_FkeyRelease (unsigned char simFkey);
extern bool Xbutton;
#endif
