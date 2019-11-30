#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#define ROOT2 (1.4142135623730950488)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "port.h"

using namespace daisysp;

void port::init(int sr, float htime)
{
    _yt1 = 0;
    _prvhtim = -100.0;
    _htime = htime;

    _sr = sr;
    _onedsr = 1.0/_sr;
}

float port::process(float in)
{
    if(_prvhtim != _htime) {
        _c2 = pow(0.5, _onedsr / _htime);
        _c1 = 1.0 - _c2;
        _prvhtim = _htime;
    }

    return _yt1 = _c1 * in + _c2 * _yt1;
}
