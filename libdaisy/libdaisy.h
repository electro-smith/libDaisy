#ifndef DSY_LIBDAISY_H
#define DSY_LIBDAISY_H

#include "dsy_system.h" // Always include

// TODO: Maybe move these into separate libdaisy_conf.h file that can be added to user projects.
#define DSY_USE_QSPI_DRV


#ifdef DSY_USE_QSPI_DRV
#include "dsy_qspi.h"
#endif
#endif
