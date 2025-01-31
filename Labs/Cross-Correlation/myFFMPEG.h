#include "struct.h"

#ifndef MYFFMPEG
#define MYFFMPEG
int8_t getMemory(size_t memoryCount, structSamp *input);
void check_format(structFile *samp, structSamp *input1, structSamp *input2);
int8_t getMemoryInicilization(structSamp *input);
int8_t decode_packet(structFile *samp, structSamp *input1, structSamp *input2);
int8_t getStream(const char *argv, structFile *samp);
int8_t getSamples(structFile *samp, structSamp *input1, structSamp *input2);
int8_t pushNull(structSamp *input, size_t memoryCount);
void cleanFile(structFile *sample, structSamp *input);
int8_t openFile(const char *argv1, const char *argv2, structSamp *input1, structSamp *input2, int32_t *sampleRate);
#endif
