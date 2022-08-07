#include <stdio.h>
#include "mixer.h"
#include "soundnode.h"

Mixer::Mixer() {
	samples = NULL;
	for (int i=0; i<MIXER_CHANNELS; i++) {
		inputNodes[i] = NULL;
		inputIndexes[i] = 0;
		volumes[i] = 0.0f;
	}
}

Mixer::~Mixer() {
	if (samples) {
		free(samples);
		samples = NULL;
	}
}

void Mixer::setup(float samplerate, int buffersize) {
	this->samplerate = samplerate;
	this->buffersize = buffersize;
	samples = (float*)malloc(buffersize * sizeof(float));
}

float * Mixer::getBuffer(int outputIdx) {
	switch (outputIdx) {
		case VOICE_OUTPUT_SAMPLES:
#ifdef DEBUG
	if (!samples) printf("Mixer: No buffer, not set up?");
#endif		
			return samples;
			break;
		default:
			return NULL;
	}
}

void Mixer::connect(int inputIdx, SoundNode *input, int outputIdx) {
	if ((inputIdx >= 0) && (inputIdx < MIXER_CHANNELS)) {
		inputNodes[inputIdx] = input;
		inputIndexes[inputIdx] = outputIdx;
	}
}

void IRAM_ATTR Mixer::render(uint32_t sampletime, uint32_t numSamples) {
	for (int i=0; i<numSamples; i++) {
		samples[i] = 0.0f;
	}
	for (int j = 0; j < MIXER_CHANNELS; j++) {
		SoundNode *input = inputNodes[j];
		float vol = volumes[j];
		if (input && (vol != 0.0f)) {
			float *src = input->getBuffer(inputIndexes[j]);
			for (int i=0; i<numSamples; i++) {
				samples[i] += vol * src[i];
			}
		}
	}
}

void Mixer::setChannelVolume(int channel, float volume) {
	if ((channel >= 0) && (channel < MIXER_CHANNELS)) {
		volumes[channel] = volume;
	}
}
