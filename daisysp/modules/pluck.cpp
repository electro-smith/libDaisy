#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "pluck.h"

#define PLUKMIN 64
#define INTERPFACTOR 256.0f
#define INTERPFACTOR_I 255

using namespace daisysp;

void pluck::reinit()
{
    int n;
    float val = 0;
    float *ap = _buf;
    //_npts = (int32_t)roundf(_decay * (float)(_maxpts - PLUKMIN) + PLUKMIN);
    _npts = (int32_t)(_decay * (float)(_maxpts - PLUKMIN) + PLUKMIN);
    //_sicps = ((float)_npts * INTERPFACTOR + INTERPFACTOR/2.0f) * (1.0f / _sr);
    _sicps = ((float)_npts * 256.0f + 128.0f) * (1.0f / _sample_rate);
    for (n=_npts; n--; ) {   
        val = (float) ((float) rand() / RAND_MAX);
        *ap++ = (val * 2.0f) - 1.0f;
    }
    _phs256 = 0;
}

void pluck::init(float sample_rate, float *buf, int32_t npts, int32_t mode)
{
    _amp = 0.5f;
    _freq = 300;
    _decay = 1.0f;
    _sample_rate = sample_rate;
    _mode = mode;

    _maxpts = npts;
    _npts = npts;
    _buf = buf;

    reinit();
    /* tuned pitch convt */
    _sicps = (npts * 256.0f + 128.0f) * (1.0f / _sample_rate);
    _init = 1;
}

void pluck::process(float *trig, float *out)
{
    float *fp;
    int32_t phs256, phsinc, ltwopi, offset;
    float coeff, inv_coeff;
    float frac, diff;
    float dampmin = 0.42f;

    if(*trig != 0) {
        _init = 0;
        reinit();
    }

    if(_init) {
        *out = 0;
        return;
    }
    // Set Coeff for mode.
    switch(_mode) {
        case PLUCK_MODE_RECURSIVE:
            coeff = ((0.5f - dampmin) * _damp) + dampmin;
            break;
        case PLUCK_MODE_WEIGHTED_AVERAGE:
            coeff = 0.05f + (_damp * 0.90f);
            break;
        default:
            coeff = 0.5f;
            break;
    }
    inv_coeff = 1.0f - coeff;
    
    phsinc = (int32_t)(_freq * _sicps);
    phs256 = _phs256;
    ltwopi = _npts << 8;
    offset = phs256 >> 8;
    fp = (float *)_buf + offset;     /* lookup position   */
    diff = fp[1] - fp[0];
    frac = (float)(phs256 & 255) / 256.0f; /*  w. interpolation */
    *out = (fp[0] + diff*frac) * _amp; /*  gives output val */
    if ((phs256 += phsinc) >= ltwopi) {
        int nn;
        float preval;
        phs256 -= ltwopi;               
        fp=_buf;
        preval = fp[0];                
        fp[0] = fp[_npts];
        fp++;
        nn = _npts;
        do {          
            /* 1st order recursive filter*/
            //preval = (*fp + preval) * coeff;
            /* weighted average - stretches decay times */
            switch(_mode) {
                case PLUCK_MODE_RECURSIVE:
                    preval = (*fp + preval) * coeff;
                    break;
                case PLUCK_MODE_WEIGHTED_AVERAGE:
                    preval = (*fp * coeff) + (preval * (1.0f - coeff));
                    break;
                default:
                    break;
            }
            *fp++ = preval;
        } while (--nn);
    }
    _phs256 = phs256;
}
