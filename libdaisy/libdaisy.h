#ifndef DSY_LIBDAISY_H
#define DSY_LIBDAISY_H

#include<stdint.h>


// Always include
#include "dsy_system.h"
#include "dsy_core_hw.h"

//enum
//{
//	DSY_SYS_BOARD_DAISY,
//	DSY_SYS_BOARD_DAISY_SEED,
//	DSY_SYS_BOARD_AUDIO_BB,
//	DSY_SYS_BOARD_EURORACK_TESTER,
//	DSY_SYS_BOARD_LAST,
//};


// TODO: Maybe move these into separate libdaisy_conf.h file that can be added to user projects.
#define DSY_USE_QSPI_DRV
#define DSY_USE_SDRAM_DRV
#define DSY_USE_AUDIO_DRV
#define DSY_USE_ADC_DRV
#define DSY_USE_DAC_DRV
#define DSY_USE_LEDDRIVER_DRV


#ifdef DSY_USE_QSPI_DRV
#include "dsy_qspi.h"
#endif
#ifdef DSY_USE_SDRAM_DRV
#include "dsy_sdram.h"
#endif
#ifdef DSY_USE_AUDIO_DRV
#include "dsy_audio.h"
#endif
#ifdef DSY_USE_ADC_DRV
#include "dsy_adc.h"
#endif
#ifdef DSY_USE_DAC_DRV
#include "dsy_dac.h"
#endif
#ifdef DSY_USE_LEDDRIVER_DRV
#include "dsy_leddriver.h"
#endif

#include "dsy_sai.h"
#include "dsy_i2c.h"
#include "dsy_gpio.h"
#include "dsy_switch.h"

#endif
