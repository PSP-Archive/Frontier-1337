#include <SDL/SDL.h>
#include <GL/gl.h>

#include "main.h"
#include "m68000.h"
#include "screen.h"
#include "input.h"

#include <pspctrl.h>
#include <pspdebug.h>

unsigned long VideoBase;	// Base address in ST Ram for screen (read on each VBL)
unsigned char *VideoRaster;	// Pointer to Video raster, after VideoBase in PC address space. Use to copy data on HBL

int len_main_palette;
unsigned short MainPalette[256];
unsigned short CtrlPalette[16];
int fe2_bgcol;

unsigned int MainRGBPalette[256];
unsigned int CtrlRGBPalette[16];

unsigned long logscreen, logscreen2, physcreen, physcreen2;

SDL_Surface *sdlscrn;			// The SDL screen surface

// mouse shown this frame?
int mouse_shown = FALSE;

// fe2 UI blits are done to old screen memory and copied to this texture.
static unsigned int screen_tex;

#define SCR_TEX_W	512
#define SCR_TEX_H	256

int screen_w;
int screen_h;

void Screen_Init(void)
{
	const SDL_VideoInfo *info = NULL;

	// Debug text to choose game resolution (� la Sektor)
	pspDebugScreenInit();
	SceCtrlData pad;
	pspDebugScreenPrintf("Frontier 1337 v0.3.0 - www.atien.net\n\n");
	pspDebugScreenPrintf("Push X for 480x272 full screen resolution\n");
	pspDebugScreenPrintf("Push O for 320x200 original resolution");
	while (1) {
		sceCtrlReadBufferPositive(&pad, 1);
		if (pad.Buttons & PSP_CTRL_CROSS){
			screen_w = 480;
			screen_h = 272;
			break;
		}
		if (pad.Buttons & PSP_CTRL_CIRCLE){
			screen_w = 320;
			screen_h = 200;
			break;
		}
	}
	pspDebugScreenClear();

			
	info = SDL_GetVideoInfo ();
	assert (info != NULL);

	
	// ----------
	// PSPSDK > 0.10.0 bug is here !!!
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	
	if ((sdlscrn = SDL_SetVideoMode (screen_w, screen_h, 16, SDL_OPENGL | SDL_DOUBLEBUF)) == 0) {
		fprintf (stderr, "Video mode set failed: %s\n", SDL_GetError ());
		SDL_Quit ();
		exit (-1);
	}
	// ----------
	
	
	glDisable (GL_CULL_FACE);
	glShadeModel (GL_FLAT);
	glDisable (GL_DEPTH_TEST);
	glClearColor (0, 0, 0, 0);
	
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	
	glEnable (GL_TEXTURE_2D);
	glGenTextures (1, &screen_tex);
	glBindTexture (GL_TEXTURE_2D, screen_tex);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, SCR_TEX_W, SCR_TEX_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable (GL_TEXTURE_2D);
	
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}


static const unsigned char font_bmp[] = {
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2,0x80,0x80,0x80,0x80,0x80,0x0,
	0x80,0x0,0x0,0x2,0xa0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4,0x0,0x50,
	0xf8,0x50,0x50,0xf8,0x50,0x0,0x0,0x6,0x20,0xf0,0xa0,0xa0,0xa0,0xa0,0xf0,0x20,
	0x0,0x5,0x0,0xc8,0xd8,0x30,0x60,0xd8,0x98,0x0,0x0,0x6,0xa0,0x0,0xe0,0xa0,
	0xa0,0xa0,0xe0,0x0,0x0,0x4,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2,
	0xc0,0x80,0x80,0x80,0x80,0x80,0x80,0xc0,0x0,0x3,0xc0,0x40,0x40,0x40,0x40,0x40,
	0x40,0xc0,0x0,0x3,0x0,0x0,0x20,0xf8,0x50,0xf8,0x20,0x0,0x0,0x6,0x0,0x0,
	0x40,0xe0,0x40,0x0,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x0,0x0,0x0,0x80,0x80,
	0x0,0x2,0x0,0x0,0x0,0xc0,0x0,0x0,0x0,0x0,0x0,0x3,0x0,0x0,0x0,0x0,
	0x0,0x0,0x80,0x0,0x0,0x2,0x0,0x8,0x18,0x30,0x60,0xc0,0x80,0x0,0x0,0x6,
	0xe0,0xa0,0xa0,0xa0,0xa0,0xa0,0xe0,0x0,0x0,0x4,0x40,0xc0,0x40,0x40,0x40,0x40,
	0xe0,0x0,0x0,0x4,0xe0,0x20,0x20,0xe0,0x80,0x80,0xe0,0x0,0x0,0x4,0xe0,0x20,
	0x20,0xe0,0x20,0x20,0xe0,0x0,0x0,0x4,0x80,0x80,0xa0,0xa0,0xe0,0x20,0x20,0x0,
	0x0,0x4,0xe0,0x80,0x80,0xe0,0x20,0x20,0xe0,0x0,0x0,0x4,0xe0,0x80,0x80,0xe0,
	0xa0,0xa0,0xe0,0x0,0x0,0x4,0xe0,0x20,0x20,0x20,0x20,0x20,0x20,0x0,0x0,0x4,
	0xe0,0xa0,0xa0,0xe0,0xa0,0xa0,0xe0,0x0,0x0,0x4,0xe0,0xa0,0xa0,0xe0,0x20,0x20,
	0xe0,0x0,0x0,0x4,0x0,0x0,0x0,0x80,0x0,0x80,0x0,0x0,0x0,0x2,0x0,0x0,
	0x0,0x80,0x0,0x0,0x80,0x80,0x0,0x2,0xe0,0x0,0xe0,0xa0,0xa0,0xa0,0xa0,0x0,
	0x0,0x4,0x0,0x0,0xe0,0x0,0xe0,0x0,0x0,0x0,0x0,0x4,0xc0,0x0,0xe0,0xa0,
	0xe0,0x80,0xe0,0x0,0x0,0x4,0xe0,0x20,0x20,0xe0,0x80,0x0,0x80,0x0,0x0,0x4,
	0xfe,0x82,0xba,0xa2,0xba,0x82,0xfe,0x0,0x0,0x8,0xf0,0x90,0x90,0x90,0xf0,0x90,
	0x90,0x0,0x0,0x5,0xf0,0x90,0x90,0xf8,0x88,0x88,0xf8,0x0,0x0,0x6,0xe0,0x80,
	0x80,0x80,0x80,0x80,0xe0,0x0,0x0,0x4,0xf8,0x48,0x48,0x48,0x48,0x48,0xf8,0x0,
	0x0,0x6,0xf0,0x80,0x80,0xe0,0x80,0x80,0xf0,0x0,0x0,0x5,0xf0,0x80,0x80,0xe0,
	0x80,0x80,0x80,0x0,0x0,0x4,0xf0,0x80,0x80,0x80,0xb0,0x90,0xf0,0x0,0x0,0x5,
	0x90,0x90,0x90,0xf0,0x90,0x90,0x90,0x0,0x0,0x5,0xe0,0x40,0x40,0x40,0x40,0x40,
	0xe0,0x0,0x0,0x4,0xf0,0x20,0x20,0x20,0x20,0x20,0xe0,0x0,0x0,0x4,0x90,0xb0,
	0xe0,0xc0,0xe0,0xb0,0x90,0x0,0x0,0x5,0x80,0x80,0x80,0x80,0x80,0x80,0xe0,0x0,
	0x0,0x4,0x88,0xd8,0xf8,0xa8,0x88,0x88,0x88,0x0,0x0,0x6,0x90,0xd0,0xf0,0xb0,
	0x90,0x90,0x90,0x0,0x0,0x5,0xf0,0x90,0x90,0x90,0x90,0x90,0xf0,0x0,0x0,0x5,
	0xf0,0x90,0x90,0xf0,0x80,0x80,0x80,0x0,0x0,0x5,0xf0,0x90,0x90,0x90,0x90,0xb0,
	0xf0,0x18,0x0,0x5,0xf0,0x90,0x90,0xf0,0xe0,0xb0,0x90,0x0,0x0,0x5,0xf0,0x80,
	0x80,0xf0,0x10,0x10,0xf0,0x0,0x0,0x5,0xe0,0x40,0x40,0x40,0x40,0x40,0x40,0x0,
	0x0,0x3,0x90,0x90,0x90,0x90,0x90,0x90,0xf0,0x0,0x0,0x5,0x90,0x90,0x90,0xb0,
	0xe0,0xc0,0x80,0x0,0x0,0x5,0x88,0x88,0x88,0xa8,0xf8,0xd8,0x88,0x0,0x0,0x6,
	0x88,0xd8,0x70,0x20,0x70,0xd8,0x88,0x0,0x0,0x6,0x90,0x90,0x90,0xf0,0x20,0x20,
	0x20,0x0,0x0,0x5,0xf0,0x10,0x30,0x60,0xc0,0x80,0xf0,0x0,0x0,0x5,0xa0,0x0,
	0xa0,0xa0,0xa0,0xa0,0xe0,0x0,0x0,0x4,0x0,0x80,0xc0,0x60,0x30,0x18,0x8,0x0,
	0x0,0x6,0xe0,0xa0,0xa0,0xe0,0xa0,0xa0,0xe0,0x80,0x80,0x4,0xe0,0xa0,0xe0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf8,0x0,0x6,
	0xa0,0x0,0xe0,0x20,0xe0,0xa0,0xe0,0x0,0x0,0x4,0x0,0x0,0xe0,0x20,0xe0,0xa0,
	0xe0,0x0,0x0,0x4,0x80,0x80,0xe0,0xa0,0xa0,0xa0,0xe0,0x0,0x0,0x4,0x0,0x0,
	0xc0,0x80,0x80,0x80,0xc0,0x0,0x0,0x3,0x20,0x20,0xe0,0xa0,0xa0,0xa0,0xe0,0x0,
	0x0,0x4,0x0,0x0,0xe0,0xa0,0xe0,0x80,0xe0,0x0,0x0,0x4,0xc0,0x80,0x80,0xc0,
	0x80,0x80,0x80,0x0,0x0,0x3,0x0,0x0,0xe0,0xa0,0xa0,0xa0,0xe0,0x20,0xe0,0x4,
	0x80,0x80,0xe0,0xa0,0xa0,0xa0,0xa0,0x0,0x0,0x4,0x80,0x0,0x80,0x80,0x80,0x80,
	0x80,0x0,0x0,0x2,0x40,0x0,0x40,0x40,0x40,0x40,0x40,0xc0,0x0,0x3,0x80,0x80,
	0xb0,0xe0,0xe0,0xb0,0x90,0x0,0x0,0x5,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x0,
	0x0,0x2,0x0,0x0,0xf8,0xa8,0xa8,0xa8,0xa8,0x0,0x0,0x6,0x0,0x0,0xe0,0xa0,
	0xa0,0xa0,0xa0,0x0,0x0,0x4,0x0,0x0,0xe0,0xa0,0xa0,0xa0,0xe0,0x0,0x0,0x4,
	0x0,0x0,0xe0,0xa0,0xa0,0xa0,0xe0,0x80,0x80,0x4,0x0,0x0,0xe0,0xa0,0xa0,0xa0,
	0xe0,0x20,0x30,0x4,0x0,0x0,0xc0,0x80,0x80,0x80,0x80,0x0,0x0,0x3,0x0,0x0,
	0xc0,0x80,0xc0,0x40,0xc0,0x0,0x0,0x3,0x80,0x80,0xc0,0x80,0x80,0x80,0xc0,0x0,
	0x0,0x3,0x0,0x0,0xa0,0xa0,0xa0,0xa0,0xe0,0x0,0x0,0x4,0x0,0x0,0xa0,0xa0,
	0xe0,0xc0,0x80,0x0,0x0,0x4,0x0,0x0,0x88,0xa8,0xf8,0xd8,0x88,0x0,0x0,0x6,
	0x0,0x0,0xa0,0xe0,0x40,0xe0,0xa0,0x0,0x0,0x4,0x0,0x0,0xa0,0xa0,0xa0,0xa0,
	0xe0,0x20,0xe0,0x4,0x0,0x0,0xf0,0x30,0x60,0xc0,0xf0,0x0,0x0,0x5,0x81,0x8d,
	0xe1,0xa0,0xa0,0xa0,0xa0,0x0,0x0,0x9,0x2,0x1a,0xc2,0x80,0xc0,0x40,0xc0,0x0,
	0x0,0x8,0xfe,0xfc,0xf8,0xfc,0xfe,0xdf,0x8e,0x4,0x0,0x7,0x7f,0x3f,0x1f,0x3f,
	0x7f,0xfb,0x71,0x20,0x0,0x8,0x4,0x8e,0xdf,0xfe,0xfc,0xf8,0xfc,0xfe,0x0,0x8,
	0x20,0x71,0xfb,0x7f,0x3f,0x1f,0x3f,0x7f,0x0,0x7,0xff,0x81,0x81,0x81,0x81,0x81,
	0x81,0xff,0x0,0x9,0x0,0x0,0xe0,0x80,0x80,0x80,0xe0,0x40,0xc0,0x4,0x60,0x0,
	0xe0,0xa0,0xe0,0x80,0xe0,0x0,0x0,0x4,0xc0,0x0,0xa0,0xa0,0xa0,0xa0,0xe0,0x0,
	0x0,0x4,0x40,0xa0,0x40,0x40,0x40,0x40,0x40,0x0,0x0,0x4,0x40,0xa0,0xe0,0x20,
	0xe0,0xa0,0xe0,0x0,0x0,0x4,0x40,0xa0,0xe0,0xa0,0xa0,0xa0,0xe0,0x0,0x0,0x4,
	0x40,0xa0,0xe0,0xa0,0xe0,0x80,0xe0,0x0,0x0,0x4,0xe0,0x0,0xa0,0xa0,0xa0,0xa0,
	0xe0,0x0,0x0,0x4,0xc0,0x0,0xe0,0x20,0xe0,0xa0,0xe0,0x0,0x0,0x4,0xe0,0xa0,
	0xa0,0xa0,0xe0,0xa0,0xa0,0x0,0x0,0x4,0xc0,0xa0,0xa0,0xc0,0xa0,0xa0,0xc0,0x0,
	0x0,0x4,0xe0,0x80,0x80,0x80,0x80,0x80,0xe0,0x0,0x0,0x4,0xc0,0xa0,0xa0,0xa0,
	0xa0,0xa0,0xc0,0x0,0x0,0x4,0xe0,0x80,0x80,0xe0,0x80,0x80,0xe0,0x0,0x0,0x4,
	0xe0,0x80,0x80,0xe0,0x80,0x80,0x80,0x0,0x0,0x4
};


static int DrawChar (int col, int xoffset, char *scrline, int chr)
{
	const unsigned char *font_pos;
	char *pix;
	int i;
	
	font_pos = font_bmp;
	font_pos += (chr&0xff)*10;
	scrline += xoffset;
	
	if (xoffset < 0) {
		font_pos += 9;
		return xoffset + *font_pos;
	}
	
	for (i=0; i<8; i++, font_pos++, scrline += SCREENBYTES_LINE) {
		pix = scrline;
		if (xoffset > 319) continue;
		if (*font_pos & 0x80) *pix = col;
		pix++;
		if (xoffset+1 > 319) continue;
		if (*font_pos & 0x40) *pix = col;
		pix++;
		if (xoffset+2 > 319) continue;
		if (*font_pos & 0x20) *pix = col;
		pix++;
		if (xoffset+3 > 319) continue;
		if (*font_pos & 0x10) *pix = col;
		pix++;
		if (xoffset+4 > 319) continue;
		if (*font_pos & 0x8) *pix = col;
		pix++;
		if (xoffset+5 > 319) continue;
		if (*font_pos & 0x4) *pix = col;
		pix++;
		if (xoffset+6 > 319) continue;
		if (*font_pos & 0x2) *pix = col;
		pix++;
		if (xoffset+7 > 319) continue;
		if (*font_pos & 0x1) *pix = col;
	}
	// width of character
	font_pos++;
	i = *font_pos;
	return xoffset + i;
}


#define MAX_QUEUED_STRINGS	200
struct QueuedString {
	int x, y, col;
	unsigned char str[64];
} queued_strings[MAX_QUEUED_STRINGS];

int queued_string_pos;



int DrawStr(int xpos, int ypos, int col, unsigned char *str, bool shadowed) {
	int x, y, chr;
	char *screen;

	x = xpos;
	y = ypos;
	
	if ((y > 192) || (y<0)) return x;
set_line:
	screen = LOGSCREEN2;
	screen += SCREENBYTES_LINE * y;

	while (*str) {
		chr = *(str++);
		
		if (chr < 0x1e) {
			if (chr == '\r') {
				y += 10;
				x = xpos;
				goto set_line;
			}
			else if (chr == 1) col = *(str++);
			continue;
		} else if (chr == 0x1e) {
			// read new xpos
			x = *(str++);
			x *= 2;
			continue;
		} else if (chr < 0x20) {
			// Read new position
			x = *(str++);
			x *= 2;
			y = *(str++);
			goto set_line;
		}

		if (shadowed) {
			DrawChar (0, x+1, screen+SCREENBYTES_LINE, chr-0x20);
		}
		x = DrawChar (col, x, screen, chr-0x20);
	}

	return x;
}



static void _BuildRGBPalette(unsigned int *rgb, unsigned short *st, int len) {
	int i;
	int st_col, r, g, b;

	for (i=0; i<len; i++, st++) {
		st_col = *st;
		b = (st_col & 0xf)<<4;
		g = (st_col & 0xf0);
		r = (st_col & 0xf00)>>4;
		rgb[i] = 0xff000000 | (b<<16) | (g<<8) | (r);
	}
}


void Nu_IsGLRenderer() {
	SetReg (0, 0);
}


void Nu_DrawScreen() {
	int x, y;
	unsigned char *scr;
	unsigned int line[320];
	unsigned int *pal;
	
	// build the RGB palettes
	_BuildRGBPalette (MainRGBPalette, MainPalette, len_main_palette);
	_BuildRGBPalette (CtrlRGBPalette, CtrlPalette, 16);

	switch (screen_w) {
		case 320: glViewport (80, 36, screen_w, screen_h); break;
		case 480: glViewport (0, 0, screen_w, screen_h); break;
	}
	
	scr = VideoRaster;
	
	glDisable (GL_DEPTH_TEST);
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	glOrtho (0, 320, 0, 200, -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, screen_tex);

	// copy whole 320x200 screen to texture
	pal = MainRGBPalette;
	for (y=0; y<200; y++) {
		if (y >= 168) pal = CtrlRGBPalette;
		for (x=0; x<320; x++) {
			line[x] = pal[*(scr++)];
		}
		glTexSubImage2D (GL_TEXTURE_2D, 0, 0, y, 320, 2, GL_RGBA, GL_UNSIGNED_BYTE, line);
	}

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBegin (GL_TRIANGLE_STRIP);
		glTexCoord2f (0.0f, 200.0f/SCR_TEX_H);
		glVertex2i (0, 0);
		glTexCoord2f (320.0f/SCR_TEX_W, 200.0f/SCR_TEX_H);
		glVertex2i (320, 0);
		glTexCoord2f (0.0f, 0.0f);
		glVertex2i (0, 200);
		glTexCoord2f (320.0f/SCR_TEX_W, 0.0f);
		glVertex2i (320, 200);
	glEnd ();

	glDisable (GL_TEXTURE_2D);
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glPopMatrix ();
	
	switch(Fkey){
		case 1:	draw_Fkey(0);	break;
		case 2:	draw_Fkey(16);	break;
		case 3:	draw_Fkey(32);	break;
		case 4:	draw_Fkey(48);	break;
		case 5:	draw_Fkey(240);	break;
		case 6:	draw_Fkey(256);	break;
		case 7:	draw_Fkey(272);	break;
		case 8:	draw_Fkey(288);	break;
		case 9:	draw_Fkey(304);	break;
	}	
	
	if (mouse_shown) {
		draw_cursor(input.abs_x, input.abs_y);
		mouse_shown = FALSE;
	}
	
	glFlush ();
	SDL_GL_SwapBuffers ();
	
	glMatrixMode (GL_MODELVIEW);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity ();
}


void draw_cursor(unsigned short abs_x, unsigned short abs_y) {
	float x;
	float y;
	x = (float)abs_x * 2 / screen_w - 1;
	y = (float)abs_y * -2 / screen_h + 1;
	glBegin(GL_LINES);
		glColor3ub(255, 255, 255);
		glVertex2f(x, y);
		glVertex2f(x + 0.04, y - 0.07);
		glVertex2f(x + 0.04, y - 0.07);
		glVertex2f(x + 0.02, y - 0.07);
		glVertex2f(x + 0.02, y - 0.07);
		glVertex2f(x, y - 0.105);
		glVertex2f(x, y - 0.105);
		glVertex2f(x, y);
	glEnd();
}


void draw_Fkey(unsigned short abs_x) {
	float x;
	float y;
	x = (float)abs_x * 2 / 320 - 1;
	y = -0.87; //187 * -2 / 200 + 1;
	glBegin(GL_LINES);
		glColor3ub(0, 0, 0);
		glVertex2f(x, y);
		glVertex2f(x + 0.102, y);
		glVertex2f(x + 0.102, y);
		glVertex2f(x + 0.102, y - 0.13);
		glVertex2f(x + 0.102, y - 0.13);
		glVertex2f(x, y - 0.13);
		glVertex2f(x, y - 0.13);
		glVertex2f(x, y);
	glEnd();
}
