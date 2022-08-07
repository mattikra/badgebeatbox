#pragma once

#include "soundnode.h"

#define MIXER_CHANNELS 4

class Mixer : public SoundNode {
	SoundNode *inputNodes[MIXER_CHANNELS];
	int inputIndexes[MIXER_CHANNELS];
	float volumes[MIXER_CHANNELS];
	float *samples;
public:
	Mixer();
	~Mixer();
	virtual void setup(float samplerate, int buffersize);	
	virtual float * getBuffer(int outputIdx);
	virtual void connect(int inputIdx, SoundNode *input, int outputIdx);
	virtual void render(uint32_t sampletime, uint32_t numSamples);
	void setChannelVolume(int channel, float volume);
};