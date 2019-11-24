/* 
 * This Source is a heavily modified version of the original
 * source from Csound.
 *
 * I've left the original license in place directly below.
 */

/*
    nlfilt.c:
    Copyright (C) 1996 John ffitch, Richard Dobson
    This file is part of Csound.
    The Csound Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    Csound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
    02110-1301 USA
*/

/* Y{n} =a Y{n-1} + b Y{n-2} + d Y^2{n-L} + X{n} - C */

/***************************************************************\
*       nlfilt.c                                                *
*       Non-linear filter (Excitable region)                    *
*       5 June 1996 John ffitch                                 *
*       See paper by Dobson and ffitch, ICMC'96                 *
\***************************************************************/

#include <string.h>
#include <math.h>
#include "nlfilt.h"
#define OK 0
#define NOT_OK 1
#define FL (float)
#define TANH tanhf
#define MAX_DELAY   (1024)
#define MAXAMP      (FL(64000.0f))


// Private Functions
static int32_t nlfilt_set(nlfilt *p);
static int32_t nlfilt2(nlfilt *p);

void nlfilt_init(nlfilt *p, size_t size)
{
	p->size = size;
	p->point = 0;  // Set delay pointer
	nlfilt_set(p);  // Setup Delay
	// For Now control pointers and i/o will be setup externally
}

void nlfilt_process(nlfilt *p)
{
	nlfilt2(p);
}

static int32_t nlfilt_set(nlfilt *p)
{
	// Initializes delay buffer.
	memset(p->delay, 0, MAX_DELAY * sizeof(float));  // Memset 
	/*
	for (uint16_t i = 0; i < MAX_DELAY; i++) // Loop
	{
	    p->delay[i] = 0.0f;
	}
	*/
	return OK;
} /* end nlfset(p) */


/* Revised version due to Risto Holopainen 12 Mar 2004 */
/* Y{n} =tanh(a Y{n-1} + b Y{n-2} + d Y^2{n-L} + X{n} - C) */

static int32_t nlfilt2(nlfilt *p)
{
	float   *ar;
	//uint32_t offset = p->h.insdshead->ksmps_offset;
	//uint32_t early  = p->h.insdshead->ksmps_no_end;
	uint32_t offset = 0;
	uint32_t n, nsmps = p->size;
	int32_t     point = p->point;
	int32_t     nm1 = point;
	int32_t     nm2 = point - 1;
	int32_t     nmL;
	float   ynm1, ynm2, ynmL;
	float   a = *p->a, b = *p->b, d = *p->d, C = *p->C;
	float   *in = p->in;
	float   *fp = (float*) p->delay;
	float   L = *p->L;
	float   maxamp, dvmaxamp, maxampd2;

	//if (UNLIKELY(fp == NULL)) goto err1;                   /* RWD fix */
	if(fp == NULL) { return NOT_OK; }
	ar   = p->ar;
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
	nsmps = p->size;
	//maxamp = csound->e0dbfs * FL(1.953125);     /* 64000 with default 0dBFS */
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
	p->point = point;
	return OK;
} /* end nlfilt2(p) */
