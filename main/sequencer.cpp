#include "sequencer.h"
#include "voice.h"
#include <stdlib.h>
#include <stdio.h>

SequencerTrack::SequencerTrack() {
	for (int i=0; i<SEQUENCER_STEPS; i++) {
		velocity[i] = 0;
		pitch[i] = 3.0f;
	}
}

Sequencer::Sequencer() {
	pitchBuffer = NULL;
	velocityBuffer = NULL;
}

Sequencer::~Sequencer() {
	if (pitchBuffer) {
		free(pitchBuffer);
		pitchBuffer = NULL;
	}
	if (velocityBuffer) {
		free(velocityBuffer);
		velocityBuffer = NULL;
	}
}

void Sequencer::setup(float samplerate, int buffersize) {
	this->samplerate = samplerate;
	this->buffersize = buffersize;
	pitchBuffer = (float*)malloc(buffersize * sizeof(float));
	velocityBuffer = (float*)malloc(buffersize * sizeof(float));
	bpm = 120;
	bar = 0;
	remainder = 0;
}

float * Sequencer::getBuffer(int outputIdx) {
	switch (outputIdx) {
		case SEQUENCER_OUTPUT_VELOCITY:
#ifdef DEBUG
	if (!velocityBuffer) printf("Sequencer: No velocity buffer, not set up?");
#endif
			return velocityBuffer;
			break;
		case SEQUENCER_OUTPUT_PITCH:
#ifdef DEBUG
	if (!pitchBuffer) printf("Sequender: No pitch buffer, not set up?");
#endif
			return pitchBuffer;
			break;
		default:
			return NULL;
	}
}

void Sequencer::connect(int inputIdx, SoundNode *input, int outputIdx) {
	//nothing to connect
}

void IRAM_ATTR Sequencer::render(uint32_t sampletime, uint32_t numSamples) {
	float barsPerSample = ((bpm / 60) / samplerate) * 4.0f;
	for (int trackIdx=0; trackIdx<NUM_TRACKS; trackIdx++) {
		SoundNode *voice = voices[trackIdx];
		if (voice) {
			int bar = this->bar;
			float remainder = this->remainder;
			for (int sampleIdx=0; sampleIdx<numSamples; sampleIdx++) {
				remainder += barsPerSample;
				if (remainder >= 1.0) {
					remainder -= 1.0;
					bar = (bar + 1) % SEQUENCER_STEPS;
				}
				float gate = remainder < 0.6 ? 1.0 : 0.0;
				velocityBuffer[sampleIdx] = gate * tracks[trackIdx].velocity[bar];
				pitchBuffer[sampleIdx] = tracks[trackIdx].pitch[bar];
			}
			voice->render(sampletime, numSamples);
		}
	}
	this->remainder += numSamples * barsPerSample;
	while (this->remainder >= 1.0) {
		this->remainder -= 1.0;
		this->bar = (this->bar + 1) % SEQUENCER_STEPS;
	}
}

void Sequencer::setVoice(int idx, Voice *voice) {
	voices[idx] = voice;
}

Voice * Sequencer::getVoice(int idx) {
  return voices[idx];
}

SequencerTrack * Sequencer::getTrack(int idx) {
	return &(tracks[idx]);
}

void Sequencer::setBPM(float bpm) {
  this->bpm = bpm;
}

float Sequencer::getBPM() {
  return bpm;
}

int Sequencer::getCurrentBar() {
	return bar;
}
