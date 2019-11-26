#ifndef DSY_DCBLOCK_H
#define DSY_DCBLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float outputs;
    float inputs;
    float gain;
} dsy_dcblock;

void dsy_dcblock_init(dsy_dcblock *p, int sr);
float dsy_dcblock_process(dsy_dcblock *p, float in);

#ifdef __cplusplus
}
#endif
#endif
