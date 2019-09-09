#ifndef DSY_LIBDAISY_H
#define DSY_LIBDAISY_H

#include "dsy_system.h" // Always include
enum
{
	DSY_SYS_BOARD_DAISY,
	DSY_SYS_BOARD_DAISY_SEED,
	DSY_SYS_BOARD_AUDIO_BB,
	DSY_SYS_BOARD_EURORACK_TESTER,
	DSY_SYS_BOARD_LAST,
};

// TODO: Maybe move these into separate libdaisy_conf.h file that can be added to user projects.
#define DSY_USE_QSPI_DRV
#define DSY_USE_SDRAM_DRV
#define DSY_USE_AUDIO_DRV


#ifdef DSY_USE_QSPI_DRV
#include "dsy_qspi.h"
#endif
#ifdef DSY_USE_SDRAM_DRV
#include "dsy_sdram.h"
#endif
#ifdef DSY_USE_AUDIO_DRV
#include "dsy_audio.h"
#endif

#endif
