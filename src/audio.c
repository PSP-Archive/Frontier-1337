#include <SDL/SDL.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "main.h"
#include "audio.h"
#include "m68000.h"

#define SND_FREQ		44100	//22050 old (GLFrontier)
#define MAX_SAMPLES		33		// Converted frontier SFX to wav samples
#define MAX_CHANNELS	2


typedef struct wav_stream {
	Uint8 *buf;
	int buf_pos;
	Uint32 buf_len;
	int loop; // -1 no loop, otherwise specifies loop start pos
} wav_stream;

wav_stream sfx_buf[MAX_SAMPLES];
wav_stream wav_channels[MAX_CHANNELS];


bool bSoundWorking = TRUE;                // Is sound OK
volatile bool bPlayingBuffer = FALSE;     // Is playing buffer?

static OggVorbis_File music_file;
static int music_mode;
static bool music_playing = FALSE;
static int enabled_tracks;

static void play_music(char track) {
	char buf[32];
	FILE *f;

	if (music_playing == TRUE) ov_clear (&music_file);
	
	snprintf (buf, sizeof (buf), "data/music/%02d.ogg", track);

	f = fopen (buf, "rb");
	if (f == NULL) {
		music_playing = FALSE;
		return;
	}

	if (ov_open (f, &music_file, NULL, 0) < 0) {
		fprintf (stderr, "Libvorbis could not open '%s'. Is it an ogg file?\n", buf);
		fclose (f);
		music_playing = FALSE;
		return;
	}

	music_playing = TRUE;
}


int rand_tracknum() {
	char track;
	if (enabled_tracks == 0) return -1;
	do {
		track = rand () % 8;
	} while ((enabled_tracks & (1<<track)) == 0);
	return track;
}


void Call_PlayMusic() {
	// Playing mode in d0:
	// -2 = play random track once
	// -1 = play random tracks continuously
	// 0+ = play specific track once
	// d1:d2 is a mask of enabled tracks

	//printf ("Play track $%x. Enabled tracks $%x%x.\n", GetReg (0), GetReg (1), GetReg(2));
	music_mode = GetReg (0);

	enabled_tracks = 0;

	if (GetReg (1)&0xff000000)	enabled_tracks |= 0x1;
	if (GetReg (1)&0xff0000)	enabled_tracks |= 0x2;
	if (GetReg (1)&0xff00)		enabled_tracks |= 0x4;
	if (GetReg (1)&0xff)		enabled_tracks |= 0x8;
	if (GetReg (2)&0xff000000)	enabled_tracks |= 0x10;
	if (GetReg (2)&0xff0000)	enabled_tracks |= 0x20;
	if (GetReg (2)&0xff00)		enabled_tracks |= 0x40;
	if (GetReg (2)&0xff)		enabled_tracks |= 0x80;
	
	SDL_LockAudio ();
	if ((music_mode < 0) && (!music_playing)) {
		play_music (rand_tracknum ());
	} else {
		play_music (music_mode);
	}
	SDL_UnlockAudio ();
}

static void stop_music() {
	music_playing = FALSE;
	//printf ("Stop music.\n");
	ov_clear (&music_file);
}

void Call_StopMusic() {
	SDL_LockAudio ();
	stop_music ();
	SDL_UnlockAudio ();
}

void Call_IsMusicPlaying() {
	SetReg (0, music_playing);
}

void Call_PlaySFX() {
	int sample, chan;

	SDL_LockAudio ();
	
	sample = (short) GetReg (REG_D0);
	chan = (short) GetReg (REG_D1);
	//printf ("Playing sample %d on channel %d.\n", sample, chan);

	wav_channels[chan].buf_pos = 0;
	wav_channels[chan].buf_len = sfx_buf[sample].buf_len;
	wav_channels[chan].buf = sfx_buf[sample].buf;
	wav_channels[chan].loop = sfx_buf[sample].loop;

	SDL_UnlockAudio ();
}


//  SDL audio callback function - copy emulation sound to audio system.
void Audio_CallBack(void *userdata, Uint8 *pDestBuffer, int len) {
	Uint8 *pBuffer;
	int i, j;
	short sample;
	bool playing = FALSE;
	
	pBuffer = pDestBuffer;
	
	for (i=0; i<MAX_CHANNELS; i++) {
		if (wav_channels[i].buf != NULL) {
			playing = TRUE;
			break;
		}
	}

	memset (pDestBuffer, 0, len);

	if (music_playing) {
		i = 0;
		while (i < len) {
			int amt;
			int music_section;
			amt = ov_read (&music_file, (char *)&pDestBuffer[i], (len - i), 0, 2, 1, &music_section);
			i += amt;

			// end of stream
			if (amt == 0) {
				//printf ("ogg stream ended.\n");
				if (music_mode == -1) {
					play_music (rand_tracknum ());
				} else {
					stop_music ();
				}
				break;
			}
		}
	}

	if (!playing) return;
	
	for (i = 0; i < len; i+=4) {
		sample = 0;
		for (j=0; j<MAX_CHANNELS; j++) {
			if (wav_channels[j].buf == NULL) continue;
			sample += *(short *)(wav_channels[j].buf + wav_channels[j].buf_pos);
			wav_channels[j].buf_pos += 2;
			if (wav_channels[j].buf_pos >= wav_channels[j].buf_len) {
				// end of sample. either loop or terminate
				if (wav_channels[j].loop != -1) {
					wav_channels[j].buf_pos = wav_channels[j].loop;
				} else {
					wav_channels[j].buf = NULL;
				}
			}
		}
		// stereo!
		*((short*)pBuffer) += sample;
		pBuffer += 2;
		*((short*)pBuffer) += sample;
		pBuffer += 2;
	}
}


// Loaded samples must be SND_FREQ, 16-bit signed. Reject other frequencies but convert 8-bit unsigned.
void check_sample_format(SDL_AudioSpec *spec, Uint8 **buf, Uint32 *len, const char *filename) {
	Uint8 *old_buf = *buf;
	short *new_buf;
	int i;

	if (spec->freq != SND_FREQ) {
		printf ("Sample %s is the wrong sample rate (wanted %dHz). Ignoring.\n", filename, SND_FREQ);
		SDL_FreeWAV (*buf);
		*buf = NULL;
		return;
	}

	if (spec->format == AUDIO_U8) {
		new_buf = malloc ((*len)*2);
		for (i=0; i<(*len); i++) {
			new_buf[i] = (old_buf[i] ^ 128) << 8;
		}
		*len *= 2;
		SDL_FreeWAV (old_buf);
		*buf = (char *)new_buf;
	} else if (spec->format != AUDIO_S16) {
		printf ("Sample %s is not 16-bit-signed or 8-bit unsigned. Ignoring.", filename);
		SDL_FreeWAV (*buf);
		*buf = NULL;
		return;
	}
}


//  Initialize the audio subsystem. Return TRUE if all OK.
//  We use direct access to the sound buffer, set to a unsigned 8-bit mono stream.
void Audio_Init(void) {
	int i;
	char filename[32];
	SDL_AudioSpec desiredAudioSpec;
	desiredAudioSpec.freq = SND_FREQ;		// Audio frequency in samples per second
	desiredAudioSpec.format = AUDIO_S16;	// Audio data format
	desiredAudioSpec.channels = 2;			// old = mono
	desiredAudioSpec.samples = 1024;		// Audio buffer size in samples
	//desiredAudioSpec.size = 1024;			// Audio buffer size in bytes (calculated)
	desiredAudioSpec.callback = Audio_CallBack;
	desiredAudioSpec.userdata = NULL;

	if (SDL_OpenAudio(&desiredAudioSpec, NULL)) {	// Open audio device
		fprintf(stderr, "Can't use audio: %s\n", SDL_GetError());
		bSoundWorking = FALSE;
		return;
	}
	
	for (i=0; i<MAX_SAMPLES; i++) {
		snprintf (filename, sizeof (filename), "data/sfx/sfx_%02d.wav", i);
			
		if (SDL_LoadWAV (filename, &desiredAudioSpec, &sfx_buf[i].buf, &sfx_buf[i].buf_len) == NULL) {
			printf ("Error loading %s: %s\n", filename, SDL_GetError ());
			sfx_buf[i].buf = NULL;
		}
		
		//check_sample_format (&desiredAudioSpec, &sfx_buf[i].buf, &sfx_buf[i].buf_len, filename);
	  
		// 19 (hyperspace) and 23 (noise) loop
		if (i == 19) sfx_buf[i].loop = SND_FREQ; // loop to about 0.5 sec in
		else if (i == 23) sfx_buf[i].loop = 0;
		else sfx_buf[i].loop = -1;
	}
  
	// All OK
	bSoundWorking = TRUE;
	// And begin
	Audio_EnableAudio(TRUE);
}



// Free audio subsystem
void Audio_UnInit(void) {
	Audio_EnableAudio(FALSE);
	SDL_CloseAudio();
}



// Start/Stop sound buffer
void Audio_EnableAudio(bool bEnable) {
	if (bEnable && !bPlayingBuffer) {
		// Start playing
		SDL_PauseAudio(FALSE);
		bPlayingBuffer = TRUE;
	}
	else if (!bEnable && bPlayingBuffer) {
		// Stop from playing
		SDL_PauseAudio(!bEnable);
		bPlayingBuffer = bEnable;
	}
}

