#pragma once

#include "soundnode.h"

#define BIQUAD_DYNAMIC_SUBSAMPLE_MASK 0x3f

typedef enum {
	LPF = 0,
	HPF = 1,
	BPF = 2
} FilterType;

class Biquad : public SoundNode {
private:
	float a1;
	float a2;
	float b0;
	float b1;
	float b2;
	float x1;
	float x2;
	float y1;
	float y2;

	FilterType filterType;
	float freq;	//"voltage" - 1 = 440Hz
	float q;
	float freqMod;
	float qMod;

	SoundNode* valueInputNode;
	int valueInputIdx;

	SoundNode* frequencyInputNode;
	int frequencyInputIdx;

	SoundNode* qInputNode;
	int qInputIdx;

	float *samples;

	void refreshCoeffs();

public:
	Biquad();
	~Biquad();
	virtual void setup(float samplerate, int buffersize);	
	virtual float * getBuffer(int outputIdx);
	virtual void connect(int inputIdx, SoundNode *input, int outputIdx);
	virtual void render(uint32_t sampletime, uint32_t numSamples);
	void setLPF(float freq, float q);	//Freq as "voltage", 1 = 440Hz
	void setHPF(float freq, float q);	//Freq as "voltage", 1 = 440Hz
	void setBPF(float freq, float q);	//Freq as "voltage", 1 = 440Hz

};

