#include "myFFMPEG.h"

#include "struct.h"

int8_t getMemory(size_t memoryCount, structSamp *input)
{
	input->size += memoryCount;
	double *checkNull = input->sample;
	input->sample = (double *)realloc(checkNull, input->size * sizeof(double));
	if (input->sample == NULL)
	{
		free(checkNull);
		fputs("Memory allocation for reading samples failed\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	}
	return SUCCESS;
}

void check_format(structFile *samp, structSamp *input1, structSamp *input2)
{
	enum AVSampleFormat format = samp->frame->format;
	if (format == AV_SAMPLE_FMT_U8 || format == AV_SAMPLE_FMT_U8P)
	{
		uint8_t *pointer2 = NULL, *pointer = (uint8_t *)samp->frame->data[samp->channel];
		if (input2 != NULL)
		{
			pointer2 = (uint8_t *)samp->frame->data[samp->channel + 1];
		}
		for (size_t i = 0; i < samp->frame->nb_samples; i++)
		{
			input1->sample[(input1->kol)++] = pointer[i];
			if (input2 != NULL)
			{
				input2->sample[(input2->kol)++] = pointer2[i];
			}
		}
	}
	else if (format == AV_SAMPLE_FMT_S16 || format == AV_SAMPLE_FMT_S16P)
	{
		int16_t *pointer2 = NULL, *pointer = (int16_t *)samp->frame->data[samp->channel];
		if (input2 != NULL)
		{
			pointer2 = (int16_t *)samp->frame->data[samp->channel + 1];
		}
		for (size_t i = 0; i < samp->frame->nb_samples; i++)
		{
			input1->sample[(input1->kol)++] = pointer[i];
			if (input2 != NULL)
			{
				input2->sample[(input2->kol)++] = pointer2[i];
			}
		}
	}
	else if (format == AV_SAMPLE_FMT_S32 || format == AV_SAMPLE_FMT_S32P)
	{
		int32_t *pointer2 = NULL, *pointer = (int32_t *)samp->frame->data[samp->channel];
		if (input2 != NULL)
		{
			pointer2 = (int32_t *)samp->frame->data[samp->channel + 1];
		}
		for (size_t i = 0; i < samp->frame->nb_samples; i++)
		{
			input1->sample[(input1->kol)++] = pointer[i];
			if (input2 != NULL)
			{
				input2->sample[(input2->kol)++] = pointer2[i];
			}
		}
	}
	else if (format == AV_SAMPLE_FMT_S64 || format == AV_SAMPLE_FMT_S64P)
	{
		int64_t *pointer2 = NULL, *pointer = (int64_t *)samp->frame->data[samp->channel];
		if (input2 != NULL)
		{
			pointer2 = (int64_t *)samp->frame->data[samp->channel + 1];
		}
		for (size_t i = 0; i < samp->frame->nb_samples; i++)
		{
			input1->sample[(input1->kol)++] = (double)pointer[i];
			if (input2 != NULL)
			{
				input2->sample[(input2->kol)++] = (double)pointer2[i];
			}
		}
	}
	else if (format == AV_SAMPLE_FMT_FLT || format == AV_SAMPLE_FMT_FLTP)
	{
		float *pointer2 = NULL, *pointer = (float *)samp->frame->data[samp->channel];
		if (input2 != NULL)
		{
			pointer2 = (float *)samp->frame->data[samp->channel + 1];
		}
		for (size_t i = 0; i < samp->frame->nb_samples; i++)
		{
			input1->sample[(input1->kol)++] = pointer[i];
			if (input2 != NULL)
			{
				input2->sample[(input2->kol)++] = pointer2[i];
			}
		}
	}
	else if (format == AV_SAMPLE_FMT_DBL || format == AV_SAMPLE_FMT_DBLP)
	{
		double *pointer2 = NULL, *pointer = (double *)samp->frame->data[samp->channel];
		if (input2 != NULL)
		{
			pointer2 = (double *)samp->frame->data[samp->channel + 1];
		}
		for (size_t i = 0; i < samp->frame->nb_samples; i++)
		{
			input1->sample[(input1->kol)++] = pointer[i];
			if (input2 != NULL)
			{
				input2->sample[(input2->kol)++] = pointer2[i];
			}
		}
	}
}

int8_t decode_packet(structFile *samp, structSamp *input1, structSamp *input2)
{
	int32_t response = avcodec_send_packet(samp->codecContext, samp->packet);
	if (response < 0)
	{
		fputs("Error while sending a packet to the decoder\n", stderr);
		return ERROR_FORMAT_INVALID;
	}
	while (1)
	{
		response = avcodec_receive_frame(samp->codecContext, samp->frame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
		{
			break;
		}
		else if (response < 0)
		{
			fputs("Error while receiving a frame from the decoder\n", stderr);
			return ERROR_FORMAT_INVALID;
		}
		if (input1->kol + samp->frame->nb_samples > input1->size - 1)
		{
			int8_t check = getMemory(samp->frame->nb_samples * 100, input1);
			if (check != SUCCESS)
			{
				av_frame_free(&samp->frame);
				return check;
			}
		}
		if (input2 != NULL)
		{
			if (input2->kol + samp->frame->nb_samples > input2->size - 1)
			{
				int8_t check = getMemory(samp->frame->nb_samples * 100, input2);
				if (check != SUCCESS)
				{
					av_frame_free(&samp->frame);
					return check;
				}
			}
		}
		check_format(samp, input1, input2);
		av_frame_unref(samp->frame);
	}
	return SUCCESS;
}

int8_t getStream(const char *argv, structFile *samp)
{
	samp->formatContext = avformat_alloc_context();
	if (!samp->formatContext)
	{
		fputs("ERROR: could not allocate memory for Format Context\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	}
	switch (avformat_open_input(&samp->formatContext, argv, NULL, NULL))
	{
	case AVERROR(ENOMEM):
		fputs("Could not allocate memory for open file\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	case AVERROR(EIO):
		fputs("I / O error\n", stderr);
		return ERROR_FORMAT_INVALID;
	case AVERROR(ENOENT):
		fputs("File or directory does not exist\n", stderr);
		return ERROR_CANNOT_OPEN_FILE;
	case AVERROR(ENOSYS):
		fputs("Function not supported on this system\n", stderr);
		return ERROR_UNSUPPORTED;
	case AVERROR(EINVAL):
		fputs("Invalid arguments\n", stderr);
		return ERROR_FORMAT_INVALID;
	case AVERROR(EISDIR):
		fputs("It is a directory, not a file\n", stderr);
		return ERROR_ARGUMENTS_INVALID;
	case AVERROR(EPERM):
		fputs("Operation not permitted\n", stderr);
		return ERROR_UNSUPPORTED;
	case AVERROR(EACCES):
		fputs("Access denied\n", stderr);
		return ERROR_CANNOT_OPEN_FILE;
	}
	if (avformat_find_stream_info(samp->formatContext, NULL) < 0)
	{
		fputs("It was not possible to open the file and get samples from there\n", stderr);
		return ERROR_CANNOT_OPEN_FILE;
	}
	samp->bestStream = av_find_best_stream(samp->formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (samp->bestStream < 0)
	{
		fputs("The audio stream could not be found\n", stderr);
		return ERROR_FORMAT_INVALID;
	}
	samp->codecParameters = samp->formatContext->streams[samp->bestStream]->codecpar;
	samp->sampleRate = samp->codecParameters->sample_rate;
	samp->codec = avcodec_find_decoder(samp->codecParameters->codec_id);
	if (samp->codec == NULL)
	{
		fputs("Couldn't find a suitable decoder\n", stderr);
		return ERROR_FORMAT_INVALID;
	}
	samp->codecContext = avcodec_alloc_context3(samp->codec);
	if (!samp->codecContext)
	{
		fputs("Failed to allocate memory for codec initialization\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	}
	if (avcodec_parameters_to_context(samp->codecContext, samp->codecParameters) < 0 ||
		avcodec_open2(samp->codecContext, samp->codec, NULL) < 0)
	{
		fputs("Failed to convert codec parameters and open it\n", stderr);
		return ERROR_FORMAT_INVALID;
	}
	samp->packet = av_packet_alloc();
	samp->frame = av_frame_alloc();
	if (samp->packet == NULL || samp->frame == NULL)
	{
		fputs("Failed to allocate memory for packets and frames\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	}
	return SUCCESS;
}

int8_t getMemoryInicilization(structSamp *input)
{
	input->kol = 0;
	input->size = 10000;
	input->sample = malloc(input->size * sizeof(double));
	if (input->sample == NULL)
	{
		fputs("Failed to allocate memory for an array of samples\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	}
	return SUCCESS;
}

int8_t getSamples(structFile *samp, structSamp *input1, structSamp *input2)
{
	int8_t check = getMemoryInicilization(input1);
	if (check != SUCCESS)
	{
		return check;
	}
	if (input2 != NULL)
	{
		check = getMemoryInicilization(input2);
		if (check != SUCCESS)
		{
			return check;
		}
	}
	while (av_read_frame(samp->formatContext, samp->packet) >= 0)
	{
		if (samp->packet->stream_index == samp->bestStream)
		{
			check = decode_packet(samp, input1, input2);
			if (check != SUCCESS)
			{
				av_packet_free(&samp->packet);
				return check;
			}
		}
		av_packet_unref(samp->packet);
	}
	return SUCCESS;
}

int8_t pushNull(structSamp *input, size_t memoryCount)
{
	if (input->size - 1 < memoryCount)
	{
		int8_t check = getMemory(memoryCount - input->size, input);
		if (check != SUCCESS)
		{
			return check;
		}
	}
	while (input->kol < memoryCount)
	{
		input->sample[input->kol++] = 0;
	}
	return SUCCESS;
}

void cleanFile(structFile *sample, structSamp *input)
{
	if (sample != NULL)
	{
		if (sample->formatContext != NULL)
		{
			avformat_free_context(sample->formatContext);
		}
		if (sample->codecContext != NULL)
		{
			avcodec_free_context(&sample->codecContext);
		}
		if (sample->frame != NULL)
		{
			av_frame_free(&sample->frame);
		}
		if (sample->packet != NULL)
		{
			av_packet_free(&sample->packet);
		}
	}
	if (input != NULL)
	{
		free(input->sample);
	}
}

int8_t openFile(const char *argv1, const char *argv2, structSamp *input1, structSamp *input2, int32_t *sampleRate)
{
	av_log_set_level(AV_LOG_QUIET);
	int8_t check;
	structFile obj1, obj2;
	structFile *sample1 = &obj1, *sample2 = &obj2;
	check = getStream(argv1, sample1);
	if (check != SUCCESS)
	{
		cleanFile(sample1, NULL);
		return check;
	}
	sample1->channel = 0;
	if (argv2 == NULL)
	{
		if (sample1->codecContext->ch_layout.nb_channels < 2)
		{
			fputs("No second channel was found in the single input file\n", stderr);
			cleanFile(sample1, NULL);
			return ERROR_FORMAT_INVALID;
		}
		check = getSamples(sample1, input1, input2);
		if (check != SUCCESS)
		{
			cleanFile(sample1, input1);
			cleanFile(NULL, input2);
			return check;
		}
		cleanFile(sample1, NULL);
	}
	else
	{
		check = getStream(argv2, sample2);
		if (check != SUCCESS)
		{
			cleanFile(sample2, NULL);
			return check;
		}
		sample2->channel = 0;
		check = getSamples(sample1, input1, NULL);
		if (check != SUCCESS)
		{
			cleanFile(sample1, input1);
			return check;
		}
		check = getSamples(sample2, input2, NULL);
		if (check != SUCCESS)
		{
			cleanFile(sample2, input2);
			return check;
		}
		cleanFile(sample1, NULL);
		cleanFile(sample2, NULL);
	}

	*sampleRate = sample1->sampleRate;

	size_t memoryCount = input1->kol + input2->kol - 1;

	check = pushNull(input1, memoryCount);
	if (check != SUCCESS)
	{
		cleanFile(NULL, input1);
		return check;
	}
	check = pushNull(input2, memoryCount);
	if (check != SUCCESS)
	{
		cleanFile(NULL, input2);
		return check;
	}
	return SUCCESS;
}
