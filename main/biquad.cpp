#include <stdio.h>
#include "biquad.h"

Biquad::Biquad() {
	a1 = 0.0f;
	a2 = 0.0f;
	b0 = 0.0f;
	b1 = 0.0f;
	b2 = 1.0f;
	x1 = 0.0f;
	x2 = 0.0f;
	y1 = 0.0f;
	y2 = 0.0f;
	samples = NULL;
	valueInputNode = NULL;
	q = 1.0;
	freq = 1000;
	filterType = LPF;
	refreshCoeffs();
	freqMod = 0;
	qMod = 0;
}

Biquad::~Biquad() {
	if (samples) {
		free(samples);
		samples = NULL;
	}
}

void Biquad::setup(float samplerate, int buffersize) {
	this->samplerate = samplerate;
	this->buffersize = buffersize;
	samples = (float*)malloc(buffersize * sizeof(float));
}

float * Biquad::getBuffer(int outputIdx) {
	switch (outputIdx) {
		case FILTER_OUTPUT_SAMPLES:
#ifdef DEBUG
	if (!samples) printf("Biquad: No buffer, not set up?");
#endif
			return samples;
			break;
		default:
			return NULL;
	}
}

void Biquad::connect(int inputIdx, SoundNode *input, int outputIdx) {
	switch (inputIdx) {
		case FILTER_INPUT_SAMPLES:
			valueInputNode = input;
			valueInputIdx = outputIdx;
			break;
		case FILTER_INPUT_FREQ:
			frequencyInputNode = input;
			frequencyInputIdx = outputIdx;
			break;
		case FILTER_INPUT_Q:
			qInputNode = input;
			qInputIdx = outputIdx;
			break;
	}
}

void IRAM_ATTR Biquad::render(uint32_t sampletime, uint32_t numSamples) {
#ifdef DEBUG
	if (!valueInputNode) {
		printf("Biquad: No input values");
		return;
	}
#endif
	float *src = valueInputNode->getBuffer(valueInputIdx);
	float *qMods = qInputNode ? (qInputNode->getBuffer(qInputIdx)) : NULL;
	float *freqMods = frequencyInputNode ? (frequencyInputNode->getBuffer(frequencyInputIdx)) : NULL;
	bool dynamicFilter = qMods || freqMods;
	for (int i=0; i<numSamples; i++) {
		if (dynamicFilter && (!(i & BIQUAD_DYNAMIC_SUBSAMPLE_MASK))) {
			if (qMods) {
				qMod = qMods[i];
			}
			if (freqMods) {
				freqMod = freqMods[i];
			}
			refreshCoeffs();
		}
		float sample = src[i];
		float result = b0 * src[i] + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
		samples[i] = result;
  		x2 = x1;
	  	x1 = sample;
		y2 = y1;
		y1 = result;
	}	
}

void Biquad::refreshCoeffs() {
	float frequency = freqLookup(freq + freqMod);
	float q = this->q + qMod;
	switch (filterType) {
		case HPF:
			{
				float omega = 2 * M_PI * frequency / samplerate;
			  	float sn = sin(omega);
  				float cs = cos(omega);
  				float alpha = sn / (2 * q); //sinh(M_LN2 /2 * bandwidth * omega / sn);

				b0 = (1 + cs) /2;
 				b1 = -(1 + cs);
    			b2 = (1 + cs) /2;
    			float a0 = 1 + alpha;
    			a1 = -2 * cs;
    			a2 = 1 - alpha;

			    b0 /= a0;
    			b1 /= a0;
    			b2 /= a0;
    			a1 /= a0;
    			a2 /= a0;
			}
			break;
		case LPF:
			{
				float omega = 2 * M_PI * frequency / samplerate;
  				float sn = sin(omega);
  				float cs = cos(omega);
  				float alpha = sn / (2 * q);

				b0 = (1 - cs) /2;
 				b1 = 1 - cs;
    			b2 = (1 - cs) /2;
    			float a0 = 1 + alpha;
    			a1 = -2 * cs;
    			a2 = 1 - alpha;

    			b0 /= a0;
    			b1 /= a0;
    			b2 /= a0;
    			a1 /= a0;
    			a2 /= a0;
			}
			break;
		case BPF:
			{
				float omega = 2 * M_PI * frequency / samplerate;
  				float sn = sin(omega);
  				float cs = cos(omega);
  				float alpha = sn / (2 * q); //sinh(M_LN2 /2 * bandwidth * omega / sn);

				b0 = sn / 2;
 				b1 = 0;
    			b2 = -b0;
    			float a0 = 1 + alpha;
    			a1 = -2 * cs;
    			a2 = 1 - alpha;

    			b0 /= a0;
    			b1 /= a0;
    			b2 /= a0;
    			a1 /= a0;
    			a2 /= a0;
			}
			break;
		default:
			break;
	}
}

void Biquad::setLPF(float frequency, float q) {
	filterType = LPF;
	this->freq = frequency;
	this->q = q;
	refreshCoeffs();
}

void Biquad::setHPF(float frequency, float q) {
	filterType = HPF;
	this->freq = frequency;
	this->q = q;
	refreshCoeffs();
}

void Biquad::setBPF(float frequency, float q) {
	filterType = BPF;
	this->freq = frequency;
	this->q = q;
	refreshCoeffs();
}
