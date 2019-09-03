#ifndef CODEC_FRAME_H
#define CODEC_FRAME_H
#include <stddef.h>
typedef struct {
	short l;
	short r;
}codec_frame_t;
typedef void (*sa_audio_callback)(codec_frame_t* , codec_frame_t* , size_t);
#endif
