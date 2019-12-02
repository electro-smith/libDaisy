#include <string.h>
#include <math.h>
#include "nlfilt.h"
#define OK 0
#define NOT_OK 1
#define FL (float)
#define TANH tanhf
#define MAX_DELAY   (1024)
#define MAXAMP      (FL(64000.0f))

using namespace daisysp;

void nlfilt::init(size_t size)
{
	_size = size;
	_point = 0;  // Set delay pointer
	dsy_nlfilt_set(p);  // Setup Delay
	// For Now control pointers and i/o will be setup externally
}

void nlfilt::process()
{
    nlfilt2();
}

int32_t nlfilt::set()
{
	// Initializes delay buffer.
	memset(_delay, 0, MAX_DELAY * sizeof(float));  // Memset 
    return OK;
}

/* Revised version due to Risto Holopainen 12 Mar 2004 */
/* Y{n} =tanh(a Y{n-1} + b Y{n-2} + d Y^2{n-L} + X{n} - C) */

int32_t nlfilt::nlfilt2()
{
	float   *ar;
	//uint32_t offset = _h.insdshead->ksmps_offset;
	//uint32_t early  = _h.insdshead->ksmps_no_end;
	uint32_t offset = 0;
	uint32_t n, nsmps = _size;
	int32_t     point = _point;
	int32_t     nm1 = point;
	int32_t     nm2 = point - 1;
	int32_t     nmL;
	float   ynm1, ynm2, ynmL;
	float   a = *_a, b = *_b, d = *_d, C = *_C;
	float   *in = _in;
	float   *fp = (float*) _delay;
	float   L = *_L;
	float   maxamp, dvmaxamp, maxampd2;

	//if (UNLIKELY(fp == NULL)) goto err1;                   // RWD fix 
	if(fp == NULL) { return NOT_OK; }
	ar   = _ar;
	/* L is k-rate so need to check */
	if (L < FL(1.0f))
		L = FL(1.0f);
	else if (L >= MAX_DELAY) {
		L = (float) MAX_DELAY;
	}
	nmL = point - (int32_t)(L) - 1;
	if ((nm1 < 0)) nm1 += MAX_DELAY; /* Deal with the wrapping */
	if ((nm2 < 0)) nm2 += MAX_DELAY;
	if ((nmL < 0)) nmL += MAX_DELAY;
	ynm1 = fp[nm1]; /* Pick up running values */
	ynm2 = fp[nm2];
	ynmL = fp[nmL];
	nsmps = _size;
	//maxamp = csound->e0dbfs * FL(1.953125);     // 64000 with default 0dBFS 
	//maxamp = 64000.0f; // Taken from comments above. Not sure how, though.
	maxamp = 1.935125f;
	dvmaxamp = FL(1.0f) / maxamp;
	maxampd2 = maxamp * FL(0.5F);
	// Not entirely sure if this will come into play since offset/early are hard-set to 0
	/*
	if (UNLIKELY(offset)) memset(ar, '\0', offset*sizeof(float));
	if (UNLIKELY(early)) {
	  nsmps -= early;
	  memset(&ar[nsmps], '\0', early*sizeof(float));
	}
	*/
	for(n = offset ; n < nsmps ; n++) {
		float yn;
		float out;
		yn = a * ynm1 + b * ynm2 + d * ynmL * ynmL - C;
		yn += in[n] * dvmaxamp; /* Must work in small amplitudes  */
		out = yn * maxampd2; /* Write output */
		if (out > maxamp)
			out = maxampd2;
		else if (out < -maxamp)
			out = -maxampd2;
		ar[n] = out;
		if ((++point == MAX_DELAY)) {
			point = 0;
		}
		yn = TANH(yn);
		fp[point] = yn; /* and delay line */
		if ((++nmL == MAX_DELAY)) {
			nmL = 0;
		}
		ynm2 = ynm1; /* Shuffle along */
		ynm1 = yn;
		ynmL = fp[nmL];
	}
	_point = point;
	return OK;
} /* end dsy_nlfilt2(p) */
