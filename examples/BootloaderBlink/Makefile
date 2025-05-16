# Project Name
TARGET = BootloaderBlink.bin

# Configure for debugging
# common configurations:
# use DEBUG = 1 and OPT = -Og for debugging
# or DEBUG = 0 and OPT = -O3 for performance
DEBUG = 1
OPT = -O0
APP_TYPE=BOOT_SRAM

# Sources
CPP_SOURCES = main.cpp

# Libraries
LIBDAISY_DIR = ../../
#DAISYSP_DIR = # specify path to external DaisySP

# (optional) Includes FatFS source files within project.
#USE_FATFS = 1

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core

# Specify alternate bootloader configuration
# This only changes the configuration programmed
# via the "make program-boot" command.
# All configurations are located in libDaisy/core/

#BOOT_BIN=$(SYSTEM_FILES_DIR)/dsy_bootloader_v6_2-intdfu-2000ms.bin # default
#BOOT_BIN=$(SYSTEM_FILES_DIR)/dsy_bootloader_v6_2-intdfu-10ms.bin
#BOOT_BIN=$(SYSTEM_FILES_DIR)/dsy_bootloader_v6_2-extdfu-2000ms.bin
#BOOT_BIN=$(SYSTEM_FILES_DIR)/dsy_bootloader_v6_2-extdfu-10ms.bin

include $(SYSTEM_FILES_DIR)/Makefile
