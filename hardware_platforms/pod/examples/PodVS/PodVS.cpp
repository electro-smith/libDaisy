#include "daisy_pod.h"

daisy_pod hw;

static void audio(float *in, float *out, size_t size) 
{
	for(size_t i = 0; i < size; i += 2) 
	{
		out[i] = in[i];
		out[i + 1] = in[i + 1];
	}
}

int main(void)
{
	daisy_pod_init(&hw);
	while(1)
	{
	}
}
