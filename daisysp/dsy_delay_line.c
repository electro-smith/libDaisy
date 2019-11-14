#include "dsy_delay_line.h"
// Todo: implement interpolation between integer delays

void dsy_delay_init(dsy_delay_t *p, float* buff, size_t buff_size, float samplerate)
{
	p->interp = 0;
	p->size = buff_size;
	p->delay = 1;
	p->line = buff;
	p->sr = samplerate;
	// Clear Delay Line
	dsy_delay_reset(p);
}

void dsy_delay_reset(dsy_delay_t *p)
{
	for (size_t i = 0; i < p->size; i++)
	{
		p->line[i] = 0.0f;	
	}
	p->write_ptr = 0;
}
void dsy_delay_set_delay_samps(dsy_delay_t *p, size_t size)
{
	p->delay = size < p->size ? size : size;
}
void dsy_delay_set_delay_sec(dsy_delay_t *p, float sec)
{
	float max_sec = p->size / p->sr;
	p->delay = (size_t)(sec * p->sr);
	p->delay_frac = (sec * p->sr) - p->delay;
	if (sec > max_sec)
	{
		p->delay = (size_t)max_sec;	
		p->delay_frac = max_sec - p->delay;
	}
}
void dsy_delay_line_write(dsy_delay_t *p, float val)
{
	p->line[p->write_ptr] = val;	
	p->write_ptr = (p->write_ptr - 1 + p->size) % p->size;
}
float dsy_delay_line_read(dsy_delay_t *p)
{
	return p->line[(p->write_ptr + p->delay) % p->size];
}
