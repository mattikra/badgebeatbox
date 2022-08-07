#pragma once

#include "soundnode.h"

typedef enum {
	WAVE_SQUARE = 0,
	WAVE_SAWTOOTH = 1,
	WAVE_TRIANGLE = 3,
	WAVE_SINE = 4,
	WAVE_NOISE = 5,
	WAVE_NONE = 6
} WaveType;

typedef float (*WaveFunc)(float phase);

class Voice : public SoundNode {
private:
	bool muted;
	float tune;
	float phase;
	float *samples;
	SoundNode *pitchInputNode;
	int pitchInputIdx;
	SoundNode *velocityInputNode;
	int velocityInputIdx;
	SoundNode *envelopeInputNode;
	int envelopeInputIdx;
	SoundNode *fmInputNode;
	int fmInputIdx;

	WaveFunc waveFunc;
	WaveFunc osc2WaveFunc;
	float osc2Vol;
	float osc2Detune;
	float osc2Phase;

public:
	Voice();
	~Voice();
	virtual void setup(float samplerate, int buffersize);	
	virtual float * getBuffer(int outputIdx);
	virtual void connect(int inputIdx, SoundNode *input, int outputIdx);
	virtual void render(uint32_t sampletime, uint32_t numSamples);
	void setWaveType(WaveType waveType);
	void setTune(float tune);
	void setOsc2(WaveType waveType, float vol, float detune);
	void mute(bool mute);
};
