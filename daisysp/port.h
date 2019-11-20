#ifndef DSY_PORT_H
#define DSY_PORT_H
typedef struct {
    float htime;
    float c1, c2, yt1, prvhtim;
    float sr, onedsr;
} dsy_port_t;

void dsy_port_init(dsy_port_t *p, int sr, float htime);
float dsy_port_compute(dsy_port_t *p, float in);
#endif
