TARGET = FRONTIER
PSPSDK = $(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
SDL_CONFIG = $(PSPBIN)/sdl-config

DEFAULT_CFLAGS = -I/usr/local/pspdev/psp/include/SDL
CFLAGS = $(DEFAULT_CFLAGS) -O2 -g -Wall -ffast-math

#SDK 10
LIBS = -lSDL -lGL -lpspgu -lpspvfpu -lpspirkeyb -lpsprtc -lpsppower -lpspdebug -lpspaudio -lpsphprm -lvorbisidec

#SDK 11
#LIBS = -lSDL -lGL -lpspgu -lpspvfpu -lpspirkeyb -lpsprtc -lpsppower -lpspdebug -lpspaudio -lpsphprm -lvorbisidec -logg


OBJS = src/audio.o src/hostcall.o src/main.o src/screen.o src/input.o
OBJS += fe2.part1.o fe2.part2.o

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = FRONTIER 1337 - v0.3.0
PSP_EBOOT_ICON = icon.png
PSP_EBOOT_PIC1 = pic1.png

BUILD_PRX = 1
PSP_LARGE_MEMORY = 1
PSP_FW_VERSION = 371
include $(PSPSDK)/lib/build.mak

fe2.c.part1:
	$(CC) -D PART1 -D LITTLE_ENDIAN -O0 -fomit-frame-pointer -Wall -Wno-unused -s $(DEFAULT_CFLAGS) -g -c fe2.s.c -o fe2.part1.o

fe2.c.part2:
	$(CC) -D PART2 -D LITTLE_ENDIAN -O0 -fomit-frame-pointer -Wall -Wno-unused -s $(DEFAULT_CFLAGS) -g -c fe2.s.c -o fe2.part2.o
