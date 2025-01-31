#include "return_codes.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <fftw3.h>
#include <inttypes.h>
#include <stdlib.h>

#ifndef MYSTRUCT
#define MYSTRUCT
typedef struct structSamp
{
	double *sample;
	size_t size;
	size_t kol;
} structSamp;

typedef struct structFile
{
	AVFormatContext *formatContext;
	AVCodecParameters *codecParameters;
	const AVCodec *codec;
	AVCodecContext *codecContext;
	AVPacket *packet;
	AVFrame *frame;
	int32_t bestStream;
	int32_t sampleRate;
	int8_t channel;
} structFile;
#endif
