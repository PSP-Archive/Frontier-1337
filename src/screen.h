#ifndef HATARI_SCREEN_H
#define HATARI_SCREEN_H

#include <SDL/SDL_video.h>	// for SDL_Surface 

extern unsigned long VideoBase;
extern unsigned char *VideoRaster;

extern unsigned int MainRGBPalette[256];
extern unsigned int CtrlRGBPalette[16];
extern int fe2_bgcol;

extern SDL_Surface *sdlscrn;

// palette length changes as dynamic colours change
extern int len_main_palette;
extern unsigned short MainPalette[256];
extern unsigned short CtrlPalette[16];

// XXX this crap is only needed for the software renderer
// Do not use directly - they are just locations in STRam
extern unsigned long logscreen, logscreen2, physcreen, physcreen2;
// Use these instead. They read the value
#define LOGSCREEN	(STRam + STMemory_ReadLong (logscreen))
#define LOGSCREEN2	(STRam + STMemory_ReadLong (logscreen2))
#define PHYSCREEN	(STRam + STMemory_ReadLong (physcreen))
#define PHYSCREEN2	(STRam + STMemory_ReadLong (physcreen2))

// Returns new xpos
extern int DrawStr (int xpos, int ypos, int col, unsigned char *str, bool shadowed);

extern void Screen_Init(void);

extern void Nu_DrawScreen ();
extern void Nu_IsGLRenderer ();


extern int screen_w;
extern int screen_h;
extern bool mouse_shown;

extern void draw_cursor(unsigned short abs_x, unsigned short abs_y);
extern void draw_Fkey(unsigned short abs_x);
#endif
