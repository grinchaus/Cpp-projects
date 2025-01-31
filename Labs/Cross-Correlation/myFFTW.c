#include "myFFTW.h"

#include "struct.h"

void cleanFFTW(fftw_complex *memoryBlock, double *input1, double *input2)
{
	if (memoryBlock != NULL)
	{
		fftw_free(memoryBlock);
	}
	if (input1 != NULL)
	{
		free(input1);
	}
	if (input2 != NULL)
	{
		free(input2);
	}
}

int8_t findFFTW(structSamp *input1, structSamp *input2, size_t memoryCount, int32_t *delta)
{
	fftw_complex *memoryBlock = (fftw_complex *)fftw_alloc_real((sizeof(fftw_complex) * 3 + sizeof(double)) * memoryCount);
	if (memoryBlock == NULL)
	{
		cleanFFTW(NULL, input1->sample, input2->sample);
		fputs("It was not possible to allocate memory for working with cross-correlation\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	}
	fftw_complex *fft1 = memoryBlock;
	fftw_complex *fft2 = fft1 + memoryCount;
	fftw_complex *mass = fft2 + memoryCount;
	double *out = mass + memoryCount;

	fftw_plan planFirst = fftw_plan_dft_r2c_1d((int32_t)memoryCount, input1->sample, fft1, FFTW_ESTIMATE);
	if (planFirst == NULL)
	{
		cleanFFTW(memoryBlock, input1->sample, input2->sample);
		fputs("Error creating FFTW first plan\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	}
	fftw_execute(planFirst);

	fftw_plan planSecond = fftw_plan_dft_r2c_1d((int32_t)memoryCount, input2->sample, fft2, FFTW_ESTIMATE);
	if (planSecond == NULL)
	{
		fftw_destroy_plan(planFirst);
		cleanFFTW(memoryBlock, input1->sample, input2->sample);
		fputs("Error creating FFTW second plan\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	}
	fftw_execute(planSecond);

	for (size_t i = 0; i < memoryCount; i++)
	{
		mass[i][0] = fft1[i][0] * fft2[i][0] + fft1[i][1] * fft2[i][1];
		mass[i][1] = fft1[i][1] * fft2[i][0] - fft1[i][0] * fft2[i][1];
	}
	fftw_plan planRes = fftw_plan_dft_c2r_1d((int32_t)memoryCount, mass, out, FFTW_ESTIMATE);
	if (planRes == NULL)
	{
		fftw_destroy_plan(planFirst);
		fftw_destroy_plan(planSecond);
		cleanFFTW(memoryBlock, input1->sample, input2->sample);
		fputs("Error creating FFTW res plan\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	}
	fftw_execute(planRes);

	double max = -1;
	size_t index = -1;
	for (size_t i = 0; i < memoryCount; i++)
	{
		if (out[i] > max)
		{
			max = out[i];
			index = i;
		}
	}
	*delta = (int32_t)((index > memoryCount / 2) ? index - memoryCount : index);
	fftw_destroy_plan(planFirst);
	fftw_destroy_plan(planSecond);
	fftw_destroy_plan(planRes);
	cleanFFTW(memoryBlock, input1->sample, input2->sample);
	return SUCCESS;
}
