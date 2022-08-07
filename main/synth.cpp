#include "synth.h"
#include "sequencer.h"
#include "voice.h"
#include "mixer.h"
#include "adsr.h"
#include "biquad.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "notes.h"

float globalVolume = 0.5f;

Sequencer sequencer;
Voice v1;
ADSR env1;
ADSR fm1;
Voice v2;
ADSR env2;
Biquad filter2;
Voice v3;
Biquad filter3;
ADSR env3;
Voice v4;
ADSR env4;
ADSR filterenv4;
Biquad filter4;
Mixer mixer;

float veloc1[SEQUENCER_STEPS] = {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1};
float pitch1[SEQUENCER_STEPS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

float veloc2[SEQUENCER_STEPS] = {0,0,1,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,0,0,0,1,0};
float pitch2[SEQUENCER_STEPS] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

float veloc3[SEQUENCER_STEPS] = {0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0};
float pitch3[SEQUENCER_STEPS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

float veloc4[SEQUENCER_STEPS] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
float pitch4[SEQUENCER_STEPS] = {C2,C2,C2,DIS3,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,DIS3,C2,C2,C2,C2,C2,C2,C2,C2,C2,C2,G3,C4};

void setup(float samplerate, int buffersize) {
	generatePitchLookup(samplerate);
	sequencer.setup(samplerate, buffersize);
	v1.setup(samplerate, buffersize);
	v2.setup(samplerate, buffersize);
	v3.setup(samplerate, buffersize);
	v4.setup(samplerate, buffersize);
	env1.setup(samplerate, buffersize);
	env2.setup(samplerate, buffersize);
	env3.setup(samplerate, buffersize);
	env4.setup(samplerate, buffersize);
	fm1.setup(samplerate, buffersize);
	filter2.setup(samplerate, buffersize);
	filter3.setup(samplerate, buffersize);
	filter4.setup(samplerate, buffersize);
	filterenv4.setup(samplerate, buffersize);
	mixer.setup(samplerate, buffersize);

	sequencer.setBPM(140);
	sequencer.setVoice(0, &v1); 
	sequencer.setVoice(1, &v2); 
	sequencer.setVoice(2, &v3); 
	sequencer.setVoice(3, &v4); 	
	memcpy(sequencer.getTrack(0)->velocity, veloc1, SEQUENCER_STEPS*sizeof(float));
	memcpy(sequencer.getTrack(0)->pitch   , pitch1, SEQUENCER_STEPS*sizeof(float));
	memcpy(sequencer.getTrack(1)->velocity, veloc2, SEQUENCER_STEPS*sizeof(float));
	memcpy(sequencer.getTrack(1)->pitch   , pitch2, SEQUENCER_STEPS*sizeof(float));
	memcpy(sequencer.getTrack(2)->velocity, veloc3, SEQUENCER_STEPS*sizeof(float));
	memcpy(sequencer.getTrack(2)->pitch   , pitch3, SEQUENCER_STEPS*sizeof(float));
	memcpy(sequencer.getTrack(3)->velocity, veloc4, SEQUENCER_STEPS*sizeof(float));
	memcpy(sequencer.getTrack(3)->pitch   , pitch4, SEQUENCER_STEPS*sizeof(float));

	for (int i=0; i<SEQUENCER_STEPS; i++) {
		sequencer.getTrack(3)->velocity[i] = (float)(rand()) / RAND_MAX;
	}

	v1.connect(VOICE_INPUT_PITCH, &sequencer, SEQUENCER_OUTPUT_PITCH);
	env1.connect(VOICE_INPUT_VELOCITY, &sequencer, SEQUENCER_OUTPUT_VELOCITY);
	v1.connect(VOICE_INPUT_ENVELOPE, &env1, CONTROL_OUTPUT_VALUE);
	fm1.connect(VOICE_INPUT_VELOCITY, &sequencer, SEQUENCER_OUTPUT_VELOCITY);
	v1.connect(VOICE_INPUT_FM, &fm1, CONTROL_OUTPUT_VALUE);
	env1.setADSR(0.001, 0.1, 0.0, 0.1);
	fm1.setADSR(0.001, 0.05, 0.0, 0.05);
	fm1.setMagnitude(4);

	v2.connect(VOICE_INPUT_PITCH, &sequencer, SEQUENCER_OUTPUT_PITCH);
	env2.connect(VOICE_INPUT_VELOCITY, &sequencer, SEQUENCER_OUTPUT_VELOCITY);
	v2.connect(VOICE_INPUT_ENVELOPE, &env2, CONTROL_OUTPUT_VALUE);
	v2.setWaveType(WAVE_NOISE);
	env2.setADSR(0.001, 0.03, 0.0, 0.03);
	filter2.setHPF(E7, 3);
	filter2.connect(FILTER_INPUT_SAMPLES, &v2, FILTER_OUTPUT_SAMPLES);

	v3.connect(VOICE_INPUT_PITCH, &sequencer, SEQUENCER_OUTPUT_PITCH);
	env3.connect(VOICE_INPUT_VELOCITY, &sequencer, SEQUENCER_OUTPUT_VELOCITY);
	v3.connect(VOICE_INPUT_ENVELOPE, &env3, CONTROL_OUTPUT_VALUE);
	v3.setWaveType(WAVE_NOISE);
	env3.setADSR(0.001, 0.15, 0.0, 0.15);
	filter3.setBPF(G5, 2);
	filter3.connect(FILTER_INPUT_SAMPLES, &v3, FILTER_OUTPUT_SAMPLES);
	
	v4.connect(VOICE_INPUT_PITCH, &sequencer, SEQUENCER_OUTPUT_PITCH);
	env4.connect(VOICE_INPUT_VELOCITY, &sequencer, SEQUENCER_OUTPUT_VELOCITY);
	v4.connect(VOICE_INPUT_ENVELOPE, &env4, CONTROL_OUTPUT_VALUE);
	env4.setADSR(0.001, 0.14, 0.0, 0.14);
	v4.setTune(-0.5);
	v4.setWaveType(WAVE_SAWTOOTH);
	v4.setOsc2(WAVE_SAWTOOTH, 1.0, 0.05);

	filterenv4.connect(CONTROL_INPUT_VALUE, &sequencer, SEQUENCER_OUTPUT_VELOCITY);
	filterenv4.setADSR(0.001, 0.2, 0.0, 0.2);
	filterenv4.setMagnitude(3);
	filterenv4.setDynamicFraction(0.9);
	filter4.setLPF(C3, 3);
	filter4.connect(FILTER_INPUT_SAMPLES, &v4, FILTER_OUTPUT_SAMPLES);
	filter4.connect(FILTER_INPUT_FREQ, &filterenv4, CONTROL_OUTPUT_VALUE);

	mixer.connect(0, &v1, VOICE_OUTPUT_SAMPLES);
	mixer.setChannelVolume(0, 0.6);
	mixer.connect(1, &filter2, VOICE_OUTPUT_SAMPLES);
	mixer.setChannelVolume(1, 0.15);
	mixer.connect(2, &filter3, VOICE_OUTPUT_SAMPLES);
	mixer.setChannelVolume(2, 0.3);
	mixer.connect(3, &filter4, VOICE_OUTPUT_SAMPLES);
	mixer.setChannelVolume(3, 0.3);
	
}

static uint32_t sampleIdx = 0;

void render(float *buffer, int numSamples) {
	sequencer.render(sampleIdx, numSamples);	//also renders voices
	filter2.render(sampleIdx, numSamples);
	filter3.render(sampleIdx, numSamples);
	filterenv4.render(sampleIdx, numSamples);
	filter4.render(sampleIdx, numSamples);
	mixer.render(sampleIdx, numSamples);
	sampleIdx += numSamples;
	float * result = mixer.getBuffer(VOICE_OUTPUT_SAMPLES);
	memcpy(buffer, result, numSamples * sizeof(float));
}

void render2I16(int16_t *buffer, int numSamples) {
	sequencer.render(sampleIdx, numSamples);	//also renders voices
	filter2.render(sampleIdx, numSamples);
	filter3.render(sampleIdx, numSamples);
	filterenv4.render(sampleIdx, numSamples);	//NOTE: THIS ONLY WORKS FOR THE LAST TRACK! FIXME!
	filter4.render(sampleIdx, numSamples);
	mixer.render(sampleIdx, numSamples);
	sampleIdx += numSamples;
	float * result = mixer.getBuffer(VOICE_OUTPUT_SAMPLES);
	float vol = 16000.0f * globalVolume;
	for (int i=0; i<numSamples; i++) {
		int16_t val = (int16_t)(result[i] * vol);
		buffer[2*i] = val;
		buffer[2*i+1] = val;
	}
}

#define CHANGE_TUNE_WEIGHT 10
#define CHANGE_BPM_WEIGHT 10
#define MUTE_CHANNEL_WEIGHT 10
#define CHANGE_MELODY_WEIGHT 20
#define CHANGE_ACCENT_WEIGHT 20
#define CHANGE_KICK_WEIGHT 1
#define CHANGE_SNARE_WEIGHT 1
#define CHANGE_HIHAT_WEIGHT 10
#define CHANGE_KICK_FM_WEIGHT 10
#define DO_NOTHING_WEIGHT 100


#define ALL_EVENTS_WEIGHT (CHANGE_TUNE_WEIGHT +\
CHANGE_BPM_WEIGHT +\
MUTE_CHANNEL_WEIGHT +\
CHANGE_MELODY_WEIGHT +\
CHANGE_ACCENT_WEIGHT +\
CHANGE_KICK_WEIGHT +\
CHANGE_SNARE_WEIGHT +\
CHANGE_HIHAT_WEIGHT +\
CHANGE_KICK_FM_WEIGHT +\
DO_NOTHING_WEIGHT)

#define MAX_MUTED_CHANNEL_EVENTS 5

static int mutedChannel = 0; 
static int mutedChannelEvents = 0;

void muteChannel(int idx, bool mute) { 	//idx is 1-based
	switch (idx) {
		case 1: v1.mute(mute); break;
		case 2: v2.mute(mute); break;
		case 3: v3.mute(mute); break;
		case 4: v4.mute(mute); break;
		default: break;
	}
}


void permutate(uint32_t seed, bool sim) {
	//unmute channel if it was muted for a while
	if (mutedChannel) {
		if (mutedChannelEvents > MAX_MUTED_CHANNEL_EVENTS) {
			muteChannel(mutedChannel, false);
			mutedChannel = false;
		} else {
			mutedChannelEvents++;
		}
	}
	srandom(seed);
	random();
	uint32_t rndff = random();
	uint32_t choice = rndff % ALL_EVENTS_WEIGHT;
	float rnd01 = (float)random() / (float)(0xffffffff);
	printf("choice is %i rnd01 is %f\n", choice, rnd01);
	if (choice < CHANGE_TUNE_WEIGHT) {
		float newTune = rnd01 - 0.5;
		printf("changing tune to %f\n", newTune);
		v4.setTune(newTune);
		return;
	}
	choice -= CHANGE_TUNE_WEIGHT;

	if (choice < CHANGE_BPM_WEIGHT) {
		float newBPM = 110 + 50 * rnd01;
		printf("setting BPM to %f\n", newBPM);
		sequencer.setBPM(newBPM);
	}
	choice -= CHANGE_BPM_WEIGHT;

	if (choice < MUTE_CHANNEL_WEIGHT) {
		int channelToMute = (int)(rnd01 * 3.99999) + 1;
		printf("muting channel %i\n", channelToMute);
		if (mutedChannel) {
			muteChannel(mutedChannel, false);
		}
		muteChannel(channelToMute, true);
		mutedChannel = channelToMute;
		mutedChannelEvents = 0;
		return;
	}
	choice -= MUTE_CHANNEL_WEIGHT;

	if (choice < CHANGE_MELODY_WEIGHT) {
		int bar = sequencer.getCurrentBar();
		if (sim) bar = rndff % SEQUENCER_STEPS;
		int semitone = (int)(12.9f*rnd01);
		float newTone = A2 + (1.0f/12.0f) * (float)semitone;
		printf("setting melody pitch %i to %f\n",bar, newTone);
		sequencer.getTrack(3)->pitch[bar] = newTone;
		return;
	}
	choice -= CHANGE_MELODY_WEIGHT;

	if (choice < CHANGE_ACCENT_WEIGHT) {
		int bar = sequencer.getCurrentBar();
		if (sim) bar = rndff % SEQUENCER_STEPS;
		float newVelocity = (rnd01 <= 0.5f) ? (rnd01 * 2.0f) : 0.0f;
		printf("setting melody velocity %i to %f\n",bar, newVelocity);
		sequencer.getTrack(3)->velocity[bar] = newVelocity;
		return;
	}
	choice -= CHANGE_ACCENT_WEIGHT;

	if (choice < CHANGE_KICK_WEIGHT) {
		int bar = sequencer.getCurrentBar();
		if (sim) bar = rndff % SEQUENCER_STEPS;
		float newVelocity = (rnd01 <= 0.25f) ? 1.0f : 0.0f;
		printf("setting kick %i to %f\n",bar, newVelocity);
		sequencer.getTrack(0)->velocity[bar] = newVelocity;
		return;
	}	
	choice -= CHANGE_KICK_WEIGHT;

	if (choice < CHANGE_SNARE_WEIGHT) {
		int bar = sequencer.getCurrentBar();
		if (sim) bar = rndff % SEQUENCER_STEPS;
		float newVelocity = (rnd01 <= 0.15f) ? 1.0f : 0.0f;
		printf("setting snare %i to %f\n",bar, newVelocity);
		sequencer.getTrack(2)->velocity[bar] = newVelocity;
		return;
	}
	choice -= CHANGE_SNARE_WEIGHT;
	
	if (choice < CHANGE_HIHAT_WEIGHT) {
		int bar = sequencer.getCurrentBar();
		if (sim) bar = rndff % SEQUENCER_STEPS;
		float newVelocity = (rnd01 <= 0.4f) ? 1.0f : 0.0f;
		printf("setting hihat %i to %f\n",bar, newVelocity);
		sequencer.getTrack(1)->velocity[bar] = newVelocity;
		return;
	}
	choice -= CHANGE_HIHAT_WEIGHT;

	if (choice < CHANGE_KICK_FM_WEIGHT) {
		float newMagnitude = 1.7 + rnd01;
		printf("setting kick FM to %f\n", newMagnitude);
		fm1.setMagnitude(newMagnitude);
		return;
	}
	choice -= CHANGE_KICK_FM_WEIGHT;

	//else : DO NOTHING
}
