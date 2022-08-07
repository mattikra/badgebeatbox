#include "voice.h"
#include "synth.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

float squareFunc(float phase) {
	return (phase < 0.5f) ? 1.0f : -1.0f;
}

float sawFunc(float phase) {
	return (phase * 2.0f) - 1.0f;
}

float triFunc(float phase) {
	return (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - (4.0f * phase));
}

float sineFunc(float phase) {
	return sin(phase * 2.0f * M_PI);
}

float noiseFunc(float phase) {
	return ((float)(rand())/(float)(RAND_MAX)) * 2.0f - 1.0f;
}

float silenceFunc(float phase) {
	return 0.0f;
}


Voice::Voice() {
	muted = false;
	tune = 0;
	phase = 0;
	samples = NULL;
	samples = NULL;
	pitchInputNode = NULL;
	velocityInputNode = NULL;
	envelopeInputNode = NULL;
	fmInputNode = NULL;
	setWaveType(WAVE_SINE);
	setOsc2(WAVE_NONE, 0.0f, 0.0f);
}

Voice::~Voice() {
	if (samples) {
		free(samples);
		samples = NULL;
	}
}

void Voice::setup(float samplerate, int buffersize) {
	this->samplerate = samplerate;
	this->buffersize = buffersize;
	samples = (float*)malloc(buffersize * sizeof(float));
}

float * Voice::getBuffer(int outputIdx) {
	switch (outputIdx) {
		case VOICE_OUTPUT_SAMPLES:
#ifdef DEBUG
	if (!samples) printf("Voice: No buffer, not set up?");
#endif
			return samples;
			break;
		default:
			return NULL;
	}
}

void Voice::connect(int inputIdx, SoundNode *input, int outputIdx) {
	switch (inputIdx) {
		case VOICE_INPUT_VELOCITY:
			velocityInputNode = input;
			velocityInputIdx = outputIdx;
			break;
		case VOICE_INPUT_PITCH:
			pitchInputNode = input;
			pitchInputIdx = outputIdx;
			break;
		case VOICE_INPUT_ENVELOPE:
			envelopeInputNode = input;
			envelopeInputIdx = outputIdx;
			break;
		case VOICE_INPUT_FM:
			fmInputNode = input;
			fmInputIdx = outputIdx;
			break;
	}
}

void IRAM_ATTR Voice::render(uint32_t sampletime, uint32_t numSamples) {
	if (muted) return;
#ifdef DEBUG
	if (!pitchInputNode) {
		printf("Voice: no pitchInputNode!\n");
		return;
	}
	if ((!envelopeInputNode) && (!velocityInputNode)) {
		printf("Voice: no envelope or velocity input!\n");
		return;
	}
#endif
	if (envelopeInputNode) {
		envelopeInputNode->render(sampletime, numSamples);
	}
	if (fmInputNode) {
		fmInputNode->render(sampletime, numSamples);
	}
	float* pitches = pitchInputNode->getBuffer(pitchInputIdx);
	float* envelopes = envelopeInputNode ? 
		envelopeInputNode->getBuffer(envelopeInputIdx) : 
		velocityInputNode->getBuffer(velocityInputIdx);
	float *fms = fmInputNode ? fmInputNode->getBuffer(fmInputIdx) : NULL;
	for (int i=0; i<numSamples; i++) {
		float pitch = pitches[i];
		float envelope = envelopes[i];
		float fm = fms ? fms[i] : 0.0f;
		phase += pitchLookup(pitch+fm+tune);
		phase -= (int)phase;
		float val = (*waveFunc)(phase);
		if (osc2Vol != 0.0f) {
			osc2Phase += pitchLookup(pitch+fm+tune+osc2Detune);
			osc2Phase -= (int)osc2Phase;
			val += osc2Vol * (*osc2WaveFunc)(osc2Phase);
		}
		samples[i] = val * envelope;
	}
}

WaveFunc funcForWaveType(WaveType waveType) {
		switch (waveType) {
		case WAVE_SQUARE:
			return squareFunc;
			break;
		case WAVE_SAWTOOTH:
			return sawFunc;
			break;
		case WAVE_TRIANGLE:
			return triFunc;
			break;
		case WAVE_SINE:
			return sineFunc;
			break;
		case WAVE_NOISE:
			return noiseFunc;
			break;
		case WAVE_NONE:
			return silenceFunc;
			break;
	}
	return silenceFunc;
}
void Voice::setWaveType(WaveType waveType) {
	waveFunc = funcForWaveType(waveType);
}

void Voice::setTune(float tune) {
	this->tune = tune;
}

void Voice::setOsc2(WaveType waveType, float vol, float detune) {
	osc2WaveFunc = funcForWaveType(waveType);
	osc2Vol = vol;
	osc2Detune = detune;
}


void Voice::mute(bool mute) {
	if (mute && !muted) {
		muted = true;
		if (samples) {
			for (int i=0; i<buffersize; i++) {
				samples[i] = 0.0f;
			}
		}
		muted = true;
	} else if (!mute && muted) {
		muted = false;
	}
}


