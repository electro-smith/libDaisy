#pragma once
#ifndef DSY_LINE_H
#define DSY_LINE_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
typedef struct
{
	float start, end, dur;
	float inc, val, sr;
	uint8_t finished;
} dsy_line;
void dsy_line_init(dsy_line *p, float sr);
void dsy_line_start(dsy_line *p, float start, float end, float dur);
float dsy_line_process(dsy_line *p, uint8_t *finished);
#ifdef __cplusplus
}
#endif
#endif
