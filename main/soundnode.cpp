#include "soundnode.h"

#define NUM_PITCHES 2048
#define PITCHES_MASK 2047
#define PITCH_LOOKUP_BASE 0
#define PITCH_LOOKUP_OCTAVES 8.0
#define PITCH_LOOKUP_SCALE (NUM_PITCHES / PITCH_LOOKUP_OCTAVES)

static float pitches[NUM_PITCHES];
static float frequencies[NUM_PITCHES];

void generatePitchLookup(float samplerate) {
	for (int i=0; i<NUM_PITCHES; i++) {
		float pitch = (((float)i) / PITCH_LOOKUP_SCALE) + PITCH_LOOKUP_BASE;
		pitches[i] = (55.0/samplerate) * pow(2, pitch);
		frequencies[i] = 55.0 * pow(2, pitch);
	}
}

float pitchLookup(float pitch) {
	int idx = (pitch-PITCH_LOOKUP_BASE) * PITCH_LOOKUP_SCALE;
	return pitches[idx&PITCHES_MASK];
}

float freqLookup(float pitch) {
	int idx = (pitch-PITCH_LOOKUP_BASE) * PITCH_LOOKUP_SCALE;
	return frequencies[idx&PITCHES_MASK];
}

