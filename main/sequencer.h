#pragma once
#include "soundnode.h"

#define NUM_TRACKS 4
#define SEQUENCER_STEPS 32

class Voice;

class SequencerTrack {
public:
	SequencerTrack();
	float pitch[SEQUENCER_STEPS];
	float velocity[SEQUENCER_STEPS];
};

class Sequencer : public SoundNode {
	float *pitchBuffer;
	float *velocityBuffer;
	SequencerTrack tracks[NUM_TRACKS];
	Voice *voices[NUM_TRACKS];
	float bpm;
	int bar;
	float remainder;
public:
	Sequencer();
	~Sequencer();
	virtual void setup(float samplerate, int buffersize);	
	virtual float * getBuffer(int outputIdx);
	virtual void connect(int inputIdx, SoundNode *input, int outputIdx);
	virtual void render(uint32_t sampletime, uint32_t numSamples);
  void setVoice(int idx, Voice *voice); 
  Voice * getVoice(int idx); 
	SequencerTrack * getTrack(int idx); 
	void setBPM(float bpm);
  float getBPM();
	int getCurrentBar();
};