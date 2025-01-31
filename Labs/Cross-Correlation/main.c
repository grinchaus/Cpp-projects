#include "myFFMPEG.h"
#include "myFFTW.h"
#include "struct.h"

int main(int argc, char **argv)
{
	structSamp input1 = { NULL, 0, 0 }, input2 = { NULL, 0, 0 };
	int8_t check = 0;
	int32_t sampleRate = 0;
	if (argc == 3)
	{
		check = openFile(argv[1], argv[2], &input1, &input2, &sampleRate);
	}
	else if (argc == 2)
	{
		check = openFile(argv[1], NULL, &input1, &input2, &sampleRate);
	}
	else
	{
		fputs("The input has an incorrect number of arguments\n", stderr);
		return ERROR_ARGUMENTS_INVALID;
	}
	if (check != SUCCESS)
	{
		return check;
	}
	int32_t delta;
	check = findFFTW(&input1, &input2, input1.size, &delta);
	if (check != SUCCESS)
	{
		return check;
	}
	printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n", delta, sampleRate, delta * 1000 / sampleRate);
	return 0;
}
