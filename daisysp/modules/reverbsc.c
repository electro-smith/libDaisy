// Reverb SC -- Ported from csound/soundpipe
// Original author(s):	Sean Costello, Istvan Varga
// Year:				1999, 2005	
// Ported by:			Stephen Hensley
// 

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "reverbsc.h"

#define REVSC_OK 0
#define REVSC_NOT_OK 1

#define DEFAULT_SRATE   48000.0
#define MIN_SRATE       5000.0
#define MAX_SRATE       1000000.0
#define MAX_PITCHMOD    20.0
#define DELAYPOS_SHIFT  28
#define DELAYPOS_SCALE  0x10000000
#define DELAYPOS_MASK   0x0FFFFFFF

#ifndef M_PI
#define M_PI		3.14159265358979323846	/* pi */
#endif

/* reverbParams[n][0] = delay time (in seconds)                     */
/* reverbParams[n][1] = random variation in delay time (in seconds) */
/* reverbParams[n][2] = random variation frequency (in 1/sec)       */
/* reverbParams[n][3] = random seed (0 - 32767)                     */

static const float reverbParams[8][4] = {
	{ (2473.0 / DEFAULT_SRATE), 0.0010, 3.100, 1966.0 },
	{ (2767.0 / DEFAULT_SRATE), 0.0011, 3.500, 29491.0 },
	{ (3217.0 / DEFAULT_SRATE), 0.0017, 1.110, 22937.0 },
	{ (3557.0 / DEFAULT_SRATE), 0.0006, 3.973, 9830.0 },
	{ (3907.0 / DEFAULT_SRATE), 0.0010, 2.341, 20643.0 },
	{ (4127.0 / DEFAULT_SRATE), 0.0011, 1.897, 22937.0 },
	{ (2143.0 / DEFAULT_SRATE), 0.0017, 0.891, 29491.0 },
	{ (1933.0 / DEFAULT_SRATE), 0.0006, 3.221, 14417.0 }
};

static int delay_line_max_samples(float sr, float iPitchMod, int n);
static int init_delay_line(dsy_reverbsc_t *p, dsy_reverbsc_dl_t *lp, int n);
static int delay_line_bytes_alloc(float sr, float iPitchMod, int n);
static const float outputGain  = 0.35;
static const float jpScale     = 0.25;

int dsy_reverbsc_init(dsy_reverbsc_t *p, float sr)
{
	p->iSampleRate = sr;
	p->sampleRate = sr;
	p->feedback = 0.97;
	p->lpfreq = 10000;
	p->iPitchMod = 1;
	p->iSkipInit = 0;
	p->dampFact = 1.0;
	p->prv_LPFreq = 0.0;
	p->initDone = 1;
	int i, nBytes = 0;
	for (i = 0; i < 8; i++) {
		nBytes += delay_line_bytes_alloc(sr, 1, i);
	}
	//sp_auxdata_alloc(&p->aux, nBytes);
	nBytes = 0;
	for (i = 0; i < 8; i++) {
		p->delayLines[i].buf = (p->aux) + nBytes;
		init_delay_line(p, &p->delayLines[i], i);
		nBytes += delay_line_bytes_alloc(sr, 1, i);
	}
    return 0;
}

static int delay_line_max_samples(float sr, float iPitchMod, int n)
{
	float maxDel;

	maxDel = reverbParams[n][0];
	maxDel += (reverbParams[n][1] * (float) iPitchMod * 1.125);
	return (int)(maxDel * sr + 16.5);
}

static int delay_line_bytes_alloc(float sr, float iPitchMod, int n)
{
	int nBytes = 0;

	nBytes += (delay_line_max_samples(sr, iPitchMod, n) * (int) sizeof(float));
	return nBytes;
}

static void next_random_lineseg(dsy_reverbsc_t *p, dsy_reverbsc_dl_t *lp, int n)
{
	float prvDel, nxtDel, phs_incVal;

	/* update random seed */
	if (lp->seedVal < 0)
		lp->seedVal += 0x10000;
	lp->seedVal = (lp->seedVal * 15625 + 1) & 0xFFFF;
	if (lp->seedVal >= 0x8000)
		lp->seedVal -= 0x10000;
	/* length of next segment in samples */
	lp->randLine_cnt = (int)((p->sampleRate / reverbParams[n][2]) + 0.5);
	prvDel = (float) lp->writePos;
	prvDel -= ((float) lp->readPos
	           + ((float) lp->readPosFrac / (float) DELAYPOS_SCALE));
	while (prvDel < 0.0)
		prvDel += lp->bufferSize;
	prvDel = prvDel / p->sampleRate; /* previous delay time in seconds */
	nxtDel = (float) lp->seedVal * reverbParams[n][1] / 32768.0;
	/* next delay time in seconds */
	nxtDel = reverbParams[n][0] + (nxtDel * (float) p->iPitchMod);
	/* calculate phase increment per sample */
	phs_incVal = (prvDel - nxtDel) / (float) lp->randLine_cnt;
	phs_incVal = phs_incVal * p->sampleRate + 1.0;
	lp->readPosFrac_inc = (int)(phs_incVal * DELAYPOS_SCALE + 0.5);
}

static int init_delay_line(dsy_reverbsc_t *p, dsy_reverbsc_dl_t *lp, int n)
{
	float readPos;
	/* int     i; */

	/* calculate length of delay line */
	lp->bufferSize = delay_line_max_samples(p->sampleRate, 1, n);
	lp->dummy = 0;
	lp->writePos = 0;
	/* set random seed */
	lp->seedVal = (int)(reverbParams[n][3] + 0.5);
	/* set initial delay time */
	readPos = (float) lp->seedVal * reverbParams[n][1] / 32768;
	readPos = reverbParams[n][0] + (readPos * (float) p->iPitchMod);
	readPos = (float) lp->bufferSize - (readPos * p->sampleRate);
	lp->readPos = (int) readPos;
	readPos = (readPos - (float) lp->readPos) * (float) DELAYPOS_SCALE;
	lp->readPosFrac = (int)(readPos + 0.5);
	/* initialise first random line segment */
	next_random_lineseg(p, lp, n);
	/* clear delay line to zero */
	lp->filterState = 0.0;
	memset(lp->buf, 0, sizeof(float) * lp->bufferSize);
	return REVSC_OK;
}


int dsy_reverbsc_process(dsy_reverbsc_t *p, float *in1, float *in2, float *out1, float *out2)
{
	float ainL, ainR, aoutL, aoutR;
	float vm1, v0, v1, v2, am1, a0, a1, a2, frac;
	dsy_reverbsc_dl_t *lp;
	int readPos;
	uint32_t n;
	int bufferSize; /* Local copy */
	float dampFact = p->dampFact;

	if (p->initDone <= 0) return REVSC_NOT_OK;

	/* calculate tone filter coefficient if frequency changed */

	if (p->lpfreq != p->prv_LPFreq) {
		p->prv_LPFreq = p->lpfreq;
		dampFact = 2.0 - cos(p->prv_LPFreq * (2 * M_PI) / p->sampleRate);
		dampFact = p->dampFact = dampFact - sqrt(dampFact * dampFact - 1.0);
	}

	/* calculate "resultant junction pressure" and mix to input signals */

	ainL = aoutL = aoutR = 0.0;
	for (n = 0; n < 8; n++) {
		ainL += p->delayLines[n].filterState;
	}
	ainL *= jpScale;
	ainR = ainL + *in2;
	ainL = ainL + *in1;

	/* loop through all delay lines */

	for (n = 0; n < 8; n++) {
		lp = &p->delayLines[n];
		bufferSize = lp->bufferSize;

		/* send input signal and feedback to delay line */

		lp->buf[lp->writePos] = (float)((n & 1 ? ainR : ainL)
		                         - lp->filterState);
		if (++lp->writePos >= bufferSize) {
			lp->writePos -= bufferSize;
		}

		/* read from delay line with cubic interpolation */

		if (lp->readPosFrac >= DELAYPOS_SCALE) {
			lp->readPos += (lp->readPosFrac >> DELAYPOS_SHIFT);
			lp->readPosFrac &= DELAYPOS_MASK;
		}
		if (lp->readPos >= bufferSize)
			lp->readPos -= bufferSize;
		readPos = lp->readPos;
		frac = (float) lp->readPosFrac * (1.0 / (float) DELAYPOS_SCALE);

		/* calculate interpolation coefficients */

		a2 = frac * frac; a2 -= 1.0; a2 *= (1.0 / 6.0);
		a1 = frac; a1 += 1.0; a1 *= 0.5; am1 = a1 - 1.0;
		a0 = 3.0 * a2; a1 -= a0; am1 -= a2; a0 -= frac;

		/* read four samples for interpolation */

		if (readPos > 0 && readPos < (bufferSize - 2)) {
			vm1 = (float)(lp->buf[readPos - 1]);
			v0  = (float)(lp->buf[readPos]);
			v1  = (float)(lp->buf[readPos + 1]);
			v2  = (float)(lp->buf[readPos + 2]);
		}
		else {

			/* at buffer wrap-around, need to check index */

			if (--readPos < 0) readPos += bufferSize;
			vm1 = (float) lp->buf[readPos];
			if (++readPos >= bufferSize) readPos -= bufferSize;
			v0 = (float) lp->buf[readPos];
			if (++readPos >= bufferSize) readPos -= bufferSize;
			v1 = (float) lp->buf[readPos];
			if (++readPos >= bufferSize) readPos -= bufferSize;
			v2 = (float) lp->buf[readPos];
		}
		v0 = (am1 * vm1 + a0 * v0 + a1 * v1 + a2 * v2) * frac + v0;

		/* update buffer read position */

		lp->readPosFrac += lp->readPosFrac_inc;

		/* apply feedback gain and lowpass filter */

		v0 *= (float) p->feedback;
		v0 = (lp->filterState - v0) * dampFact + v0;
		lp->filterState = v0;

		/* mix to output */

		if (n & 1) {
			aoutR += v0;
		}
		else {
			aoutL += v0;
		}

		/* start next random line segment if current one has reached endpoint */

		if (--(lp->randLine_cnt) <= 0) {
			next_random_lineseg(p, lp, n);
		}
	}
	/* someday, use aoutR for multimono out */

	*out1  = aoutL * outputGain;
	*out2 = aoutR * outputGain;
	return REVSC_OK;
}
