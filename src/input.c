#include <SDL/SDL_endian.h>
#include <SDL/SDL.h>

#include "main.h"
#include "input.h"
#include "m68000.h"
#include "screen.h"

INPUT input;

void Call_GetMouseInput () {
	short *mouse_mov, *mouse_abs;
	unsigned long params;
	
	params = GetReg (REG_A7);
	params -= SIZE_WORD;

	mouse_mov = (short*)(STRam + STMemory_ReadLong (params + SIZE_WORD));
	mouse_abs = (short*)(STRam + STMemory_ReadLong (params + SIZE_WORD + SIZE_LONG));

	mouse_mov[0] = SDL_SwapBE16 (SDL_SwapBE16 (mouse_mov[0]) + input.motion_x);
	mouse_mov[1] = SDL_SwapBE16 (SDL_SwapBE16 (mouse_mov[1]) + input.motion_y);
	
	mouse_abs[0] = SDL_SwapBE16 (320 * input.abs_x / screen_w);
	mouse_abs[1] = SDL_SwapBE16 (200 * input.abs_y / screen_h);
	
	mouse_mov[2] = SDL_SwapBE16 (0xf8 | input.cur_mousebut_state);
}


void Call_GetKeyboardEvent () {
	if ((input.buf_head) != (input.buf_tail)) {
		SetReg (REG_D0, input.key_buf [input.buf_head++]);
		input.buf_head %= SIZE_KEYBUF;
	} else {
		SetReg (REG_D0, 0);
	}
}


void Input_PressSTKey (unsigned char ScanCode, bool bPress) {
	if (!bPress) ScanCode |= 0x80;
	input.key_buf [input.buf_tail++] = ScanCode;
	input.buf_tail %= SIZE_KEYBUF;
}


unsigned char Fkey = 1;
int Ltrigger = 0;
int Rtrigger = 0;
bool Xbutton = FALSE;

/* KEYCODES LIST
SDLK_ESCAPE		0x01
SDLK_1			0x02
SDLK_2			0x03
SDLK_3			0x04
SDLK_4			0x05
SDLK_5			0x06
SDLK_6			0x07
SDLK_7			0x08
SDLK_8			0x09
SDLK_9			0x0a
SDLK_0			0x0b
SDLK_BACKSPACE	0x0e
SDLK_TAB		0x0f
SDLK_RETURN		0x1c
SDLK_SPACE		0x39
SDLK_q			0x10
SDLK_w			0x11
SDLK_e			0x12
SDLK_r			0x13
SDLK_t			0x14
SDLK_y			0x15
SDLK_u			0x16
SDLK_i			0x17
SDLK_o			0x18
SDLK_p			0x19
SDLK_a			0x1e
SDLK_s			0x1f
SDLK_d			0x20
SDLK_f			0x21
SDLK_g			0x22
SDLK_h			0x23
SDLK_j			0x24
SDLK_k			0x25
SDLK_l			0x26
SDLK_z			0x2c
SDLK_x			0x2d
SDLK_c			0x2e
SDLK_v			0x2f
SDLK_b			0x30
SDLK_n			0x31
SDLK_m			0x32
SDLK_CAPSLOCK	0x3a
SDLK_LSHIFT		0x2a
SDLK_LCTRL		0x1d
SDLK_LALT		0x38
SDLK_F1			0x3b
SDLK_F2			0x3c
SDLK_F3			0x3d
SDLK_F4			0x3e
SDLK_F5			0x3f
SDLK_F6			0x40
SDLK_F7			0x41
SDLK_F8			0x42
SDLK_F9			0x43
SDLK_F10		0x44
*/


void Input_ButtonPress (unsigned char button) {
	switch (button) {
		case 0: // triangle
			Input_PressSTKey(0x1c, TRUE);
			break;
		case 1: // circle
			Input_PressSTKey(0x36, TRUE);
			break;
		case 2:	// cross
			Input_PressSTKey(0x19, TRUE); // p to pander to the prick
			input.cur_mousebut_state |= 0x1; // right mouse clic
			input.mousebut_buf [input.mbuf_tail++] = input.cur_mousebut_state;
			input.mbuf_tail %= SIZE_MOUSEBUF;
			Xbutton = TRUE;
			break;
		case 3:	// square
			input.cur_mousebut_state |= 0x2; // left mouse clic
			input.mousebut_buf [input.mbuf_tail++] = input.cur_mousebut_state;
			input.mbuf_tail %= SIZE_MOUSEBUF;	
			break;
		case 4:	// L trigger
			Fkey --;
			if (Fkey == 0) Fkey = 9;
			Ltrigger = 1;
			if (Rtrigger) Input_FkeyPress(Fkey);
			break;
		case 5:	// R trigger
			Fkey ++;
			if (Fkey == 10) Fkey = 1;
			Rtrigger = 1;
			input.motion_x = input.motion_xx / 6;
			input.motion_y = input.motion_yy / 6;
			if (Ltrigger) Input_FkeyPress(Fkey);
			break;
		case 6:	// D-pad down
			switch (Xbutton) {
				case FALSE: Input_PressSTKey(0x50, TRUE); break;
				case TRUE :	Input_PressSTKey(0x4E, TRUE); break;
			}
			break;
		case 7: // D-pad left
			Input_PressSTKey(0x4b, TRUE);
			break;
		case 8:	// D-pad up
			switch (Xbutton) {
				case FALSE: Input_PressSTKey(0x48, TRUE); break;
				case TRUE : Input_PressSTKey(0x4A, TRUE); break;
			}
			if (Rtrigger) { // Force mis-jump
				Input_PressSTKey(0x38, TRUE); // Alt
				Input_PressSTKey(0x42, TRUE); // F8
			}
			break;
		case 9: // D-pad right
			Input_PressSTKey(0x4d, TRUE);
			break;
		case 10: // select 
			switch (Rtrigger) {
				case 0: Input_PressSTKey(0x3f, TRUE); break; // F5
				case 1: Input_PressSTKey(0x21, TRUE); break; // f for fps
			}
			break;
		case 11: // start
			Input_PressSTKey(0x01, TRUE); // esc
			break;
	}
	return;
}


void Input_ButtonRelease (unsigned char button) {
	switch (button) {
		case 0: // triangle
			Input_PressSTKey(0x1c, FALSE);
			break;
		case 1: // circle
			Input_PressSTKey(0x36, FALSE);
			break;
		case 2:	// cross
			Input_PressSTKey(0x19, FALSE); // p to pander to the prick
			input.cur_mousebut_state &= ~0x1; // right mouse clic
			input.mousebut_buf [input.mbuf_tail++] = input.cur_mousebut_state;
			input.mbuf_tail %= SIZE_MOUSEBUF;
			Xbutton = FALSE;
			break;
		case 3:	// square
			input.cur_mousebut_state &= ~0x2; // left mouse clic
			input.mousebut_buf [input.mbuf_tail++] = input.cur_mousebut_state;
			input.mbuf_tail %= SIZE_MOUSEBUF;
			break;
		case 4:	// L trigger
			Ltrigger = 0;
			Input_FkeyRelease(Fkey);
			break;
		case 5:	// R trigger
			Rtrigger = 0;
			input.motion_x = input.motion_xx / 2;
			input.motion_y = input.motion_yy / 2;
			Input_FkeyRelease(Fkey);
			break;
		case 6:	// D-pad down
			switch (Xbutton) {
				case FALSE: Input_PressSTKey(0x50, FALSE); break;
				case TRUE : Input_PressSTKey(0x4E, FALSE); break;
			}
			break;
		case 7: // D-pad left
			Input_PressSTKey(0x4b, FALSE);
			break;
		case 8:	// D-pad up
			switch (Xbutton) {
				case FALSE: Input_PressSTKey(0x48, FALSE); break;
				case TRUE : Input_PressSTKey(0x4A, FALSE); break;
			}
			if (Rtrigger) { // Force mis-jump
				Input_PressSTKey(0x38, FALSE); // Alt
				Input_PressSTKey(0x42, FALSE); // F8
			}
			break;
		case 9: // D-pad right
			Input_PressSTKey(0x4d, FALSE);
			break;
		case 10: // select 
			switch (Rtrigger) {
				case FALSE: Input_PressSTKey(0x3f, FALSE); break; // F5
				case TRUE : Input_PressSTKey(0x21, FALSE); break; // f for fps
			}
			break;
		case 11: // start
			Input_PressSTKey(0x01, FALSE);
			break;
	}
	return;
}


void Input_FkeyPress (unsigned char simFkey) {
		switch (simFkey) {
			case 1:	Input_PressSTKey(0x3b, TRUE);	break;	//  F1
			case 2:	Input_PressSTKey(0x3c, TRUE);	break;	//  F2
			case 3:	Input_PressSTKey(0x3d, TRUE);	break;	//  F3
			case 4:	Input_PressSTKey(0x3e, TRUE);	break;	//  F4
			case 5:	Input_PressSTKey(0x40, TRUE);	break;	//  F6
			case 6:	Input_PressSTKey(0x41, TRUE);	break;	//  F7
			case 7:	Input_PressSTKey(0x42, TRUE);	break;	//  F8
			case 8:	Input_PressSTKey(0x43, TRUE);	break;	//  F9
			case 9:	Input_PressSTKey(0x44, TRUE);	break;	//  F10
		}
	return;
}


void Input_FkeyRelease (unsigned char simFkey) {
		switch (simFkey) {
			case 1:	Input_PressSTKey(0x3b, FALSE);	break;	//  F1
			case 2:	Input_PressSTKey(0x3c, FALSE);	break;	//  F2
			case 3:	Input_PressSTKey(0x3d, FALSE);	break;	//  F3
			case 4:	Input_PressSTKey(0x3e, FALSE);	break;	//  F4
			case 5:	Input_PressSTKey(0x40, FALSE);	break;	//  F6
			case 6:	Input_PressSTKey(0x41, FALSE);	break;	//  F7
			case 7:	Input_PressSTKey(0x42, FALSE);	break;	//  F8
			case 8:	Input_PressSTKey(0x43, FALSE);	break;	//  F9
			case 9:	Input_PressSTKey(0x44, FALSE);	break;	//  F10
		}
	return;
}

//  Here we process the SDL joystick events and map it to Atari events
void Main_EventHandler() {
	SDL_Event event;

	while (SDL_PollEvent (&event)) {
		switch (event.type) {
			case SDL_QUIT:
				SDL_Quit ();
				exit (0);
			break;
			case SDL_JOYBUTTONDOWN:	Input_ButtonPress (event.jbutton.button);	break;
			case SDL_JOYBUTTONUP:	Input_ButtonRelease (event.jbutton.button);	break;
			case SDL_JOYAXISMOTION:
				switch (event.jaxis.axis) {
					case 0:
						input.motion_xx = abs(event.jaxis.value) * event.jaxis.value / (100000000); 
						input.motion_x = input.motion_xx / (Rtrigger ? 6 : 2);
					break;
					case 1: 
						input.motion_yy = abs(event.jaxis.value) * event.jaxis.value / (100000000); 
						input.motion_y = input.motion_yy / (Rtrigger ? 6 : 2);
					break; 
				}
			break;
		}
	}

	// Only move the cursor if it is visible
	if(!Xbutton) {
		input.abs_x += input.motion_x;
		if (input.abs_x < 1) input.abs_x = 1;
		if (input.abs_x > screen_w - 8) input.abs_x = screen_w - 8; //470/480 314/320
		input.abs_y += input.motion_y;
		if (input.abs_y < 2) input.abs_y = 2;
		if (input.abs_y >screen_h - 10) input.abs_y = screen_h - 10; //260/272 192/200
	}
}
