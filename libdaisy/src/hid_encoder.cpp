#include "hid_encoder.h"

void	 dsy_encoder_init(dsy_encoder *p) 
{
	
	for(uint8_t i = 0; i < DSY_ENCODER_PIN_LAST; i++) 
	{
		p->pins[i].pin.port = p->pin_config[i].port;
		p->pins[i].pin.pin = p->pin_config[i].pin;
		p->pins[i].mode		= DSY_GPIO_MODE_INPUT;
		p->pins[i].pull		= DSY_GPIO_PULLUP;
		dsy_gpio_init(&p->pins[i]);
		p->states[i] = 0;
	}
	p->time = 0;
}
void dsy_encoder_debounce(dsy_encoder *p)
{
	uint8_t a, b;
	// Debounce Click
	p->states[DSY_ENCODER_PIN_CLICK]
		= (p->states[DSY_ENCODER_PIN_CLICK] << 1)
		  | dsy_gpio_read(&p->pins[DSY_ENCODER_PIN_CLICK]);
	// Debounce Quadrature States
	p->states[DSY_ENCODER_PIN_A]
		= (p->states[DSY_ENCODER_PIN_A] << 1)
		  | dsy_gpio_read(&p->pins[DSY_ENCODER_PIN_A]);
	p->states[DSY_ENCODER_PIN_B]
		= (p->states[DSY_ENCODER_PIN_B] << 1)
		  | dsy_gpio_read(&p->pins[DSY_ENCODER_PIN_B]);
	// Check Encoder click time
	if(p->states[DSY_ENCODER_PIN_CLICK] == 0x80)
	{
		p->time = 0;
	}
	else if(p->states[DSY_ENCODER_PIN_CLICK] == 0x00)
	{
		p->time += 1;
	}

	p->inc = 0;
	a	  = p->states[DSY_ENCODER_PIN_A];
	b	  = p->states[DSY_ENCODER_PIN_B];
	if((a & 0x03) == 0x02 && (b & 0x03) == 0x00)
	{
		p->inc = 1;
	}
	else if((b & 0x03) == 0x02 && (a & 0x03) == 0x00)
	{
		p->inc = -1;
	}
}
int32_t dsy_encoder_inc(dsy_encoder *p)
{
	return p->inc;
}
uint8_t dsy_encoder_state(dsy_encoder *p)
{
	return p->states[DSY_ENCODER_PIN_CLICK] == 0x00;
}
uint8_t dsy_encoder_rising_edge(dsy_encoder *p)
{
	return p->states[DSY_ENCODER_PIN_CLICK] == 0x80;
}
uint8_t dsy_encoder_falling_edge(dsy_encoder *p)
{
	return p->states[DSY_ENCODER_PIN_CLICK] == 0x7F;
}
uint32_t dsy_encoder_time_held(dsy_encoder *p)
{
	return p->time;
}
