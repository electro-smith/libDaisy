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

void nlfilt::init()
{
	point_ = 0;  // Set delay pointer
    set();
	//dsy_nlfilt_set(p);  // Setup Delay
	// For Now control pointers and i/o will be setup externally
}


/* Revised version due to Risto Holopainen 12 Mar 2004 */
/* Y{n} =tanh(a Y{n-1} + b Y{n-2} + d Y^2{n-L} + X{n} - C) */

void nlfilt::process_block(float *in, float *out, size_t size)
{
	//uint32_t offset = _h.insdshead->ksmps_offset;
	//uint32_t early  = _h.insdshead->ksmps_no_end;
	uint32_t offset = 0;
	uint32_t n, nsmps = size;
	int32_t     point = point_;
	int32_t     nm1 = point;
	int32_t     nm2 = point - 1;
	int32_t     nmL;
	float   ynm1, ynm2, ynmL;
	float   a = a_, b = b_, d = d_, C = C_;
	float   *fp = (float*) delay_;
	float   L = L_;
	float   maxamp, dvmaxamp, maxampd2;

	//if (UNLIKELY(fp == NULL)) goto err1;                   // RWD fix 
	//if(fp == NULL) { return NOT_OK; }
	//ar   = a_r;
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
	nsmps = size;
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
		float outv;
		yn = a * ynm1 + b * ynm2 + d * ynmL * ynmL - C;
		yn += in[n] * dvmaxamp; /* Must work in small amplitudes  */
		outv = yn * maxampd2; /* Write output */
		if (outv > maxamp)
			outv = maxampd2;
		else if (outv < -maxamp)
			outv = -maxampd2;
		out[n] = outv;
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
	point_ = point;
} 

int32_t nlfilt::set()
{
	// Initializes delay buffer.
	memset(delay_, 0, MAX_DELAY * sizeof(float));  // Memset 
    return OK;
}
