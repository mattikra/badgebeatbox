#pragma once

#include <stdint.h>
#include <math.h>
#include "esp_attr.h"

#define VOICE_INPUT_VELOCITY 0
#define VOICE_INPUT_PITCH 1
#define VOICE_INPUT_ENVELOPE 2
#define VOICE_INPUT_FM 3
#define CONTROL_INPUT_VALUE 0
#define FILTER_INPUT_SAMPLES 0
#define FILTER_INPUT_FREQ 1
#define FILTER_INPUT_Q 2

#define VOICE_OUTPUT_SAMPLES 0
#define FILTER_OUTPUT_SAMPLES 0
#define CONTROL_OUTPUT_VALUE 0
#define SEQUENCER_OUTPUT_VELOCITY 0
#define SEQUENCER_OUTPUT_PITCH 1


void generatePitchLookup(float samplerate);
float pitchLookup(float pitch);
float freqLookup(float pitch);

class SoundNode {
protected:
	float samplerate;
	int buffersize;
public:
	virtual void setup(float samplerate, int buffersize) = 0;	
	virtual float * getBuffer(int outputIdx) = 0;
	virtual void connect(int inputIdx, SoundNode *input, int outputIdx) = 0;
	virtual void render(uint32_t sampletime, uint32_t numSamples) = 0;
};