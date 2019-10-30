#ifndef DSY_SAI_H
#define DSY_SAI_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "stm32h7xx_hal.h"
#include "dsy_core_hw.h"

typedef enum
{
	DSY_AUDIO_INIT_SAI1,
	DSY_AUDIO_INIT_SAI2,
	DSY_AUDIO_INIT_BOTH,
	DSY_AUDIO_INIT_NONE,
	DSY_AUDIO_INIT_LAST,
} dsy_audio_sai;

// Currently Sample Rates are not correclty supported.
// We'll get there.
typedef enum
{
	DSY_AUDIO_SAMPLERATE_32K,
	DSY_AUDIO_SAMPLERATE_48K,
	DSY_AUDIO_SAMPLERATE_96K,
	DSY_AUDIO_SAMPLERATE_LAST,
}dsy_audio_samplerate;

typedef enum
{
	DSY_AUDIO_BITDEPTH_16,
	DSY_AUDIO_BITDEPTH_24,
	DSY_AUDIO_BITDEPTH_LAST
}dsy_audio_bitdepth;

typedef enum
{
	DSY_AUDIO_SYNC_MASTER,// No Crystal
	DSY_AUDIO_SYNC_SLAVE, // Crystal
	DSY_AUDIO_SYNC_LAST
} dsy_audio_sync;

typedef enum
{
	DSY_SAI_PIN_MCLK,
	DSY_SAI_PIN_FS,
	DSY_SAI_PIN_SCK,
	DSY_SAI_PIN_SIN,
	DSY_SAI_PIN_SOUT,
	DSY_SAI_PIN_LAST,
} dsy_sai_pin;

typedef enum
{
	DSY_AUDIO_NONE, // For unsupported, or custom devices.
	DSY_AUDIO_DEVICE_PCM3060,
	DSY_AUDIO_DEVICE_WM8731,
	DSY_AUDIO_DEVICE_LAST,
} dsy_audio_device;

enum
{
	DSY_SAI_1,
	DSY_SAI_2,
	DSY_SAI_LAST,
};

typedef struct
{
	dsy_audio_sai		 init;
	dsy_audio_samplerate samplerate[DSY_SAI_LAST];
	dsy_audio_bitdepth   bitdepth[DSY_SAI_LAST];
	dsy_audio_sync		 sync_config[DSY_SAI_LAST];
	dsy_audio_device	 device[DSY_SAI_LAST];
	dsy_gpio_pin		 sai1_pin_config[DSY_SAI_PIN_LAST];
	dsy_gpio_pin		 sai2_pin_config[DSY_SAI_PIN_LAST];
} dsy_sai_handle_t;

extern SAI_HandleTypeDef hsai_BlockA1;
extern SAI_HandleTypeDef hsai_BlockB1;
extern SAI_HandleTypeDef hsai_BlockA2;
extern SAI_HandleTypeDef hsai_BlockB2;

void dsy_sai_init(dsy_audio_sai	   init,
				  dsy_audio_samplerate sr[2],
				  dsy_audio_bitdepth   bitdepth[2],
				  dsy_audio_sync	   sync_config[2],
				  dsy_gpio_pin *	   sai1_pin_list,
				  dsy_gpio_pin *	   sai2_pin_list);

void dsy_sai_init_from_handle(dsy_sai_handle_t *hsai);

#ifdef __cplusplus
}
#endif
#endif //DSY_SAI_H
