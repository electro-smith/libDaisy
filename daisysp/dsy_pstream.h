// dsy_pstream.h
// Ported from Csound - October 2019
// 
// 
//	 pstream.h.Implementation of PVOCEX streaming opcodes.
//   (c) Richard Dobson August 2001
//   NB pvoc routines based on CARL distribution(Mark Dolson).
//   This file is licensed according to the terms of the GNU LGPL.
// 
// Definitions from Csound:
// pvsanal
// pvsfread
// pvsynth
// pvsadsyn
// pvscross
// pvsmaska
// = (overloaded, -- should probably just pvsset(&src, &dest))
// 
// More or less for starting purposes we really only need pvsanal, and pvsynth to get started
#pragma once
#ifndef DSY_PSTREAM_H
#define DSY_PSTREAM_H
#ifdef __cplusplus
extern "C" 
{
#endif
#include <stdint.h>
#define AUXCH float*
/* description of an fsig analysis frame*/
enum PVS_WINTYPE {
	PVS_WIN_HAMMING        = 0,
	PVS_WIN_HANN,
	PVS_WIN_KAISER,
	PVS_WIN_CUSTOM,
	PVS_WIN_BLACKMAN,
	PVS_WIN_BLACKMAN_EXACT,
	PVS_WIN_NUTTALLC3,
	PVS_WIN_BHARRIS_3,
	PVS_WIN_BHARRIS_MIN,
	PVS_WIN_RECT
};


enum PVS_ANALFORMAT {
	PVS_AMP_FREQ = 0,
	PVS_AMP_PHASE,
	PVS_COMPLEX,
	PVS_TRACKS          /* added VL, 24.06.2005 */
};

typedef struct {
	float re;
	float im;
} CMPLX;

typedef struct pvsdat {
	int32_t           N;
	int             sliding; /* Flag to indicate sliding case */
	int32_t           NB;
	int32_t           overlap;
	int32_t           winsize;
	int             wintype;
	int32_t           format; /* fixed for now to AMP:FREQ */
	uint32_t          framecount;
	AUXCH           frame; /* RWD MUST always be 32bit floats */
									/* But not in sliding case when float */
} PVSDAT;

/* may be no point supporting Kaiser in an opcode unless we can support
   the param too but we can have kaiser in a PVOCEX file. */

typedef struct {
	//OPDS    h;
	PVSDAT  *fsig; /* output signal is an analysis frame */
	float   *ain; /* input sig is audio */
	float   *fftsize; /* params */
	float   *overlap;
	float   *winsize;
	float   *wintype;
	float   *format; /* always PVS_AMP_FREQ at present */
	float   *init; /* not yet implemented */
	/* internal */
	int32_t    buflen;
	float   fund, arate;
	float   RoverTwoPi, TwoPioverR, Fexact;
	float   *nextIn;
	int32_t    nI, Ii, IOi; /* need all these ?; double as N and NB */
	int32_t    inptr;

	AUXCH   input;
	AUXCH   overlapbuf;
	AUXCH   analbuf;
	AUXCH   analwinbuf; /* prewin in SDFT case */
	AUXCH   oldInPhase;
	AUXCH           trig;
	double          *cosine, *sine;
	void    *setup;
} PVSANAL;

typedef struct {
	//OPDS    h;
	float   *aout; /* audio output signal */
	PVSDAT  *fsig; /* input signal is an analysis frame */
	float   *init; /* not yet implemented */
	/* internal */
	/* check these against fsig vals */
	int32_t    overlap, winsize, fftsize, wintype, format;
	/* can we allow variant window tpes?  */
	int32_t    buflen;
	float   fund, arate;
	float   RoverTwoPi, TwoPioverR, Fexact;
	float   *nextOut;
	int32_t    nO, Ii, IOi; /* need all these ?*/
	int32_t    outptr;
	int32_t    bin_index; /* for phase normalization across frames */
	/* renderer gets all format info from fsig */

	AUXCH   output;
	AUXCH   overlapbuf;
	AUXCH   synbuf;
	AUXCH   analwinbuf; /* may get away with a local alloc and free */
	AUXCH   synwinbuf;
	AUXCH   oldOutPhase;

	void    *setup;
} PVSYNTH;

/* for pvadsyn */

typedef struct {
	//OPDS    h;
	float   *aout;
	PVSDAT  *fsig;
	float   *n_oscs;
	float   *kfmod;
	float   *ibin; /* default  0 */
	float   *ibinoffset; /* default 1  */
	float   *init; /* not yet implemented  */
	/* internal */
	int32_t    outptr;
	uint32_t   lastframe;
	/* check these against fsig vals */
	int32_t    overlap, winsize, fftsize, wintype, format, noscs;
	int32_t    maxosc;
	float   one_over_overlap, pi_over_sr, one_over_sr;
	float   fmod;
	AUXCH   a;
	AUXCH   x;
	AUXCH   y;
	AUXCH   amps;
	AUXCH   lastamps;
	AUXCH   freqs;
	AUXCH   outbuf;
} PVADS;

/* for pvscross */
typedef struct {
	//OPDS h;
	PVSDAT  *fout;
	PVSDAT  *fsrc;
	PVSDAT  *fdest;
	float   *kamp1;
	float   *kamp2;
	/* internal */
	int32_t    overlap, winsize, fftsize, wintype, format;
	uint32_t   lastframe;
} PVSCROSS;

///* for pvsmaska */
//typedef struct {
//	//OPDS    h;
//	PVSDAT  *fout;
//	PVSDAT  *fsrc;
//	float   *ifn;
//	float   *kdepth;
//	/* internal*/
//	int32_t    overlap, winsize, fftsize, wintype, format;
//	uint32_t   lastframe;
//	int             nwarned, pwarned; /* range errors for kdepth */
//	FUNC    *maskfunc;
//} PVSMASKA;
//
///* for pvsftw, pvsftr */
//
//typedef struct {
//	//OPDS    h;
//	float   *kflag;
//	PVSDAT  *fsrc;
//	float   *ifna; /* amp, required */
//	float   *ifnf; /* freq: optional*/
//	/* internal */
//	int32_t    overlap, winsize, fftsize, wintype, format;
//	uint32_t   lastframe;
//	FUNC    *outfna, *outfnf;
//} PVSFTW;
//
//typedef struct {
//	//OPDS    h;
//	/* no output var*/
//	PVSDAT  *fdest;
//	float   *ifna; /* amp, may be 0 */
//	float   *ifnf; /* freq: optional*/
//	/* internal */
//	int32_t    overlap, winsize, fftsize, wintype, format;
//	uint32_t   lastframe;
//	FUNC    *infna, *infnf;
//	float   *ftablea, *ftablef;
//} PVSFTR;
//
///* for pvsfread */
///*  wsig pvsread ktimpt,ifilcod */
//typedef struct {
//	//OPDS h;
//	PVSDAT  *fout;
//	float   *kpos;
//	float   *ifilno;
//	float   *ichan;
//	/* internal */
//	int     ptr;
//	int32_t   overlap, winsize, fftsize, wintype, format;
//	uint32_t  chans, nframes, lastframe, chanoffset, blockalign;
//	float   arate;
//	float   *membase; /* RWD MUST be 32bit: reads file */
//} PVSFREAD;

/* for pvsinfo */

typedef struct {
	//OPDS    h;
	float   *ioverlap;
	float   *inumbins;
	float   *iwinsize;
	float   *iformat;
	/* internal*/
	PVSDAT  *fsrc;
} PVSINFO;

typedef struct {
	//OPDS    h;
	PVSDAT  *fout;
	PVSDAT  *fsrc;
} FASSIGN;

#ifdef __cplusplus
}
#endif
#endif
