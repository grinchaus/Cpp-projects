#include "struct.h"

#ifndef MYFFTW
#define MYFFTW
void cleanFFTW(fftw_complex *memoryBlock, double *input1, double *input2);
int8_t findFFTW(structSamp *input1, structSamp *input2, size_t memoryCount, int32_t *delta);
#endif
