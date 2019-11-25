/*
 * Port
 *
 * This code has been extracted from the Csound opcode "portk".
 * It has been modified to work as a Soundpipe module.
 *
 * Original Author(s): Robbin Whittle, John ffitch
 * Year: 1995, 1998
 * Location: Opcodes/biquad.c
 *
 */


#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#define ROOT2 (1.4142135623730950488)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "port.h"

void dsy_port_init(dsy_port *p, int sr, float htime)
{
    p->yt1 = 0;
    p->prvhtim = -100.0;
    p->htime = htime;

    p->sr = sr;
    p->onedsr = 1.0/p->sr;
}

float dsy_port_compute(dsy_port *p, float in)
{
    if(p->prvhtim != p->htime) {
        p->c2 = pow(0.5, p->onedsr / p->htime);
        p->c1 = 1.0 - p->c2;
        p->prvhtim = p->htime;
    }

    return p->yt1 = p->c1 * in + p->c2 * p->yt1;
}
