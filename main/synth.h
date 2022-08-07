#pragma once 

#include <stdint.h>

/** this is the main entry point for the platform independent synth. */

/** do all synth wiring, setup etc. */
void setup(float samplerate, int buffersize);

/** platform independent render callback. float, mono */
void render(float *buffer, int numSamples); 

/** platform independent render callback, stereo 16 bit signed */
void render2I16(int16_t *buffer, int numSamples);

/** change track in a quite random way */
void permutate(uint32_t seed, bool sim);
