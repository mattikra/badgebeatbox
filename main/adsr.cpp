#include "adsr.h"
#include <stdio.h>

ADSR::ADSR() {
	magnitude = 1.0f;
	attack = 0.0f;
	decay = 1.0f;
	sustain = 0.5f;
	release = 1.0f;
	wasOn = false;
	state = 'I';
	volume = 0.0f;
	samples = NULL;
	setADSR(0.03f, 0.3f, 0.5f, 0.3f);
	setDynamicFraction(0.0f);
	velocityScale = 0.0f;
}

ADSR::~ADSR() {
	if (samples) {
		free(samples);
		samples = NULL;
	}
}

void ADSR::setup(float samplerate, int buffersize) {
	this->samplerate = samplerate;
	this->buffersize = buffersize;
	samples = (float*)malloc(buffersize * sizeof(float));
	setADSR(attack, decay, sustain, release);
}

float * ADSR::getBuffer(int outputIdx) {
	switch (outputIdx) {
		case CONTROL_OUTPUT_VALUE:
			return samples;
			break;
		default:
			return NULL;
	}
}

void ADSR::connect(int inputIdx, SoundNode *input, int outputIdx) {
	switch (inputIdx) {
		case VOICE_INPUT_VELOCITY:
			velocityInputNode = input;
			velocityInputIdx = outputIdx;
			break;
		default:
			break;
	}
}

void IRAM_ATTR ADSR::render(uint32_t sampletime, uint32_t numSamples) {
#ifdef DEBUG
	if (!velocityInputNode) {
		printf("ADSR: No velocities connected!\n")
		return;
	}
#endif
	float *velocities = velocityInputNode->getBuffer(velocityInputIdx);
	//note: we don't use sampletime but assume we're called in sequence
	for (int i=0; i<numSamples; i++) {
		float velocity = velocities[i];
		bool on = velocity > 0.0f;
		bool triggered = on && !wasOn;
		bool released = !on && wasOn;
		wasOn = on;
		if (triggered) {
			state = 'A';
			velocityScale = velocity * dynamicMagScale + fixedMagScale;
		}
		if (released) {
			state = 'R';
		}
		switch (state) {
			case 'A':
				volume = volume + attackRate;
				if (volume >= velocityScale) {
					volume = velocityScale;
					state = 'D';
				}
				break;
			case 'D':
				volume = volume - decayRate;
				if (volume <= sustain) {
					volume = sustain;
					state = 'S';
				}
				break;
			case 'R':
				volume -= releaseRate;
				if (volume <= 0.0f) {
					volume = 0.0f;
					state = 'I';
				}
			default:
				break;
		}
		samples[i] = volume * magnitude;
	}
}

void ADSR::setADSR(float attack, float decay, float sustain, float release) {
	this->attack = attack;
	this->decay = decay;
	this->sustain = sustain;
	this->release = release;
	attackRate = 1.0 / (float)(attack * samplerate);
	decayRate = (1.0) / (float)(decay * samplerate);
	releaseRate = (1.0) / (float)(release * samplerate);
	printf("a %f sr %f ar %f dr %f rr %f\n",attack, samplerate, attackRate, decayRate, releaseRate);
}

void ADSR::setMagnitude(float val) {
	magnitude = val;
}

void ADSR::setDynamicFraction(float val) {
	fixedMagScale = (1.0-val);
	dynamicMagScale = val;
}

