#ifndef HATARI_MAIN_H
#define HATARI_MAIN_H

typedef int bool;

#ifndef MIN
	#define MIN(a,b)	((a)<(b) ? (a) : (b))
#endif
#ifndef MAX
	#define MAX(a,b)	((a)>(b) ? (a) : (b))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#ifndef FALSE
	#define FALSE 0
	#define TRUE (!0)
#endif

// 68000 operand sizes
#define SIZE_BYTE  1
#define SIZE_WORD  2
#define SIZE_LONG  4

// 68000 Register defines
enum {
  REG_D0,	// D0...
  REG_D1,
  REG_D2,
  REG_D3,
  REG_D4,
  REG_D5,
  REG_D6,
  REG_D7,	// ..D7
  REG_A0,	// A0..
  REG_A1,
  REG_A2,
  REG_A3,
  REG_A4,
  REG_A5,
  REG_A6,
  REG_A7,	// ..A7(also SP)
};

#define SCREENBYTES_LINE	320
#endif
