#pragma once

#include "soundnode.h"

class ADSR : public SoundNode {
	float magnitude;
	float fixedMagScale;
	float dynamicMagScale;
	float velocityScale;	//trigger velocity scaled normalized peak magnitude (excl magnitude itself)
	float attack;
	float decay;
	float sustain;
	float release;
	float attackRate;
	float decayRate;
	float releaseRate;
	bool wasOn;
	char state;
	float volume;
	SoundNode *velocityInputNode;
	int velocityInputIdx;
	float *samples;
public:	
	ADSR();
	~ADSR();
	virtual void setup(float samplerate, int buffersize);	
	virtual float * getBuffer(int outputIdx);
	virtual void connect(int inputIdx, SoundNode *input, int outputIdx);
	virtual void render(uint32_t sampletime, uint32_t numSamples);
	void setADSR(float attack, float decay, float sustain, float release);
	void setMagnitude(float val);
	void setDynamicFraction(float val);
};