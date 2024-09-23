# Core Makefile for libDaisy based projects
# =========================================
# All build options belong into the Makefile.conf
# Make sure not to accidentally overwrite any of the build options imported from Makefile.conf, or standard MAKE
# variables like CFLAGS.
HERE:=$(dir $(lastword $(MAKEFILE_LIST)))
include $(HERE)/Makefile.conf

#######################################
# Toolchain
#######################################

# GCC_DIR used to be called GCC_PATH
# lets keep some compat for a while
ifneq ($(GCC_PATH),)
  $(error GCC_PATH is deprecated, use GCC_DIR instead)
  GCC_DIR = $GCC_PATH
endif

CC = $(GCC_DIR)$(GCC_PREFIX)gcc
AS = $(GCC_DIR)$(GCC_PREFIX)gcc -x assembler-with-cpp
CXX = $(GCC_DIR)$(GCC_PREFIX)g++
GDB = $(GCC_DIR)$(GCC_PREFIX)gdb
CP = $(GCC_DIR)$(GCC_PREFIX)objcopy
SZ = $(GCC_DIR)$(GCC_PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

OCDFLAGS = -f $(OCD_PGM_DEVICE) -f target/$(OCD_CHIPSET).cfg
OCD = openocd

ifeq ($(TARGET),)
  $(error Dont know what program to compile. Please specify TARGET)
else
  $(info Building for TARGET: $(TARGET))
endif


#######################################
# Core Sources/Includes
#######################################

ifeq ($(LIBDAISY_DIR),)
  $(error dont know where my libDaisy is. Please specify LIBDAISY_DIR)
else
  $(info Using LIBDAISY_DIR: $(LIBDAISY_DIR))
endif

SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core/


ifeq ($(STARTUP_CODE),C)
  C_SOURCES += $(SYSTEM_FILES_DIR)startup_stm32h750xx.c
else
  ASM_SOURCES += $(SYSTEM_FILES_DIR)startup_stm32h750xx.s
endif

# Expose platform configuration to all compilation units (CMSIS DSP components)
INCLUDES += -include stm32h7xx.h

INCLUDES += \
  -I$(SYSTEM_FILES_DIR)

INCLUDES += \
  -I$(LIBDAISY_DIR) \
  -I$(LIBDAISY_DIR)/src/ \
  -I$(LIBDAISY_DIR)/src/sys \
  -I$(LIBDAISY_DIR)/src/usbd \
  -I$(LIBDAISY_DIR)/src/usbh \
  -I$(LIBDAISY_DIR)/Drivers/CMSIS_5/CMSIS/Core/Include/ \
  -I$(LIBDAISY_DIR)/Drivers/CMSIS-DSP/Include \
  -I$(LIBDAISY_DIR)/Drivers/CMSIS-Device/ST/STM32H7xx/Include \
  -I$(LIBDAISY_DIR)/Drivers/STM32H7xx_HAL_Driver/Inc/ \
  -I$(LIBDAISY_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
  -I$(LIBDAISY_DIR)/Middlewares/ST/STM32_USB_Host_Library/Core/Inc \
  -I$(LIBDAISY_DIR)/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc \
  -I$(LIBDAISY_DIR)/Middlewares/ST/STM32_USB_Host_Library/Class/MIDI/Inc \

LIBS += -ldaisy -lc -lm -lnosys
LIB_DIRS += -L$(LIBDAISY_DIR)/build

# unfortunately libDaisy introduced a header file dependency to FATFS at some point.
# So, it must be considered core
FATFS_DIR ?= $(LIBDAISY_DIR)/Middlewares/Third_Party/FatFs/src
INCLUDES += -I$(FATFS_DIR)


#######################################
# Boot Management
#######################################

INTERNAL_ADDRESS = 0x08000000
QSPI_ADDRESS = 0x90040000

# For the time being, we'll keep the Daisy Bootloader's PID as df11
# DAISY_PID = a360
DAISY_PID = df11
STM_PID = df11

ifeq ($(APP_TYPE), BOOT_NONE)
  LINKERSCRIPT = $(SYSTEM_FILES_DIR)STM32H750IB_flash.lds
  USBPID = $(STM_PID)
  FLASH_ADDRESS = $(INTERNAL_ADDRESS)
else ifeq ($(APP_TYPE), BOOT_SRAM)
  LINKERSCRIPT = $(SYSTEM_FILES_DIR)STM32H750IB_sram.lds
  USBPID = $(DAISY_PID)
  FLASH_ADDRESS = $(QSPI_ADDRESS)
  DEFINES += -DBOOT_APP
else ifeq ($(APP_TYPE), BOOT_QSPI)
  LINKERSCRIPT = $(SYSTEM_FILES_DIR)STM32H750IB_qspi.lds
  USBPID = $(DAISY_PID)
  FLASH_ADDRESS = $(QSPI_ADDRESS)
  DEFINES += -DBOOT_APP
else ifeq ($(APP_TYPE), CUSTOM)
  ifeq ($(LINKERSCRIPT),)
	  $(error APP_TYPE is custom, please specify at least LINKERSCRIPT)
	  $(error For DFU to work, also specify FLASH_ADDRESS and USBPID)
  endif
else
  $(error Unkown app type "$(APP_TYPE)")
endif


######################################
# Optional Dependencies
######################################

ifneq ($(DAISYSP_DIR),)
  INCLUDES += -I$(DAISYSP_DIR)/Source
  LIBS += -ldaisysp
  LIB_DIRS += -L$(DAISYSP_DIR)/build
endif

ifeq ($(USE_DAISYSP_LGPL),1)
  INCLUDES += -I$(DAISYSP_DIR)/DaisySP-LGPL/Source
  DEFINES +=  \
  -DUSE_DAISYSP_LGPL
  LIBS += -ldaisysp-lgpl
  LIB_DIRS += -L$(DAISYSP_DIR)/DaisySP-LGPL/build
endif

ifeq ($(USE_FATFS),1)
  FATFS_SOURCES = \
    $(FATFS_DIR)/diskio.c \
    $(FATFS_DIR)/ff.c \
    $(FATFS_DIR)/ff_gen_drv.c \
    $(FATFS_DIR)/option/ccsbcs.c
  C_SOURCES += $(FATFS_SOURCES)
endif


ifeq ($(DEBUG),1)
  ASFLAGS += -g -ggdb
  OPT ?= -O0 -g
  DEFINES += -DDEBUG
else
  OPT ?= -O2
endif


#######################################
# Compose Flags
#######################################

CC_ARGS = \
  $(C_STANDARD) \
  $(MCU) \
  $(OPT) \
  $(DAISY_CFLAGS) \
  $(DAISY_DEFINES) \
  $(INCLUDES) \
  $(CFLAGS)

CXX_ARGS = \
  $(CXX_STANDARD) \
  $(MCU) \
  $(OPT) \
  $(DAISY_CXXFLAGS) \
  $(DAISY_DEFINES) \
  $(INCLUDES) \
  $(CXXFLAGS)

AS_ARGS = \
  $(MCU) \
  $(OPT) \
  $(DAISY_ASFLAGS) \
  $(ASFLAGS)

LD_ARGS = \
  $(MCU) \
  $(GCC_SPECS) \
  -T$(LINKERSCRIPT) \
  $(LIB_DIRS) \
  $(LIBS) \
  $(LINKER_OPTS) \
  $(LDFLAGS)

#######################################
# Evaluate all objects from sources
#######################################

ifeq ($(CXX_SOURCES),)
  $(error No sources to compile. Please populate CXX_SOURCES with your projects source files)
else
  $(info Compiling sources:)
  $(info ASM_SOURCES: $(ASM_SOURCES))
  $(info C_SOURCES  : $(C_SOURCES))
  $(info CXX_SOURCES: $(CXX_SOURCES))
endif

# split c++ sources into suffix .cpp and user suffix
CXX_SOURCES_SUFFIX_USER=$(filter %$(USER_SUFFIX_CXX), $(CXX_SOURCES))
CXX_SOURCES_SUFFIX_CPP=$(filter %.cpp, $(CXX_SOURCES))
# list of objects
# .c files
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# c++ files with .cpp suffix
OBJECTS += $(filter .cpp, addprefix $(BUILD_DIR)/,$(notdir $(CXX_SOURCES_SUFFIX_CPP:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CXX_SOURCES_SUFFIX_CPP)))
# c++ files with user suffix
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CXX_SOURCES_SUFFIX_USER:$(USER_SUFFIX_CXX)=.o)))
vpath %$(USER_SUFFIX_CXX) $(sort $(dir $(CXX_SOURCES_SUFFIX_USER)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

ASMLISTING = -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $<).lst
#######################################
# Recipes
#######################################

.PHONY: all clean openocd debug debug_client program program-dfu program-boot bulu

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

# Link (elf from obj)
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CXX) $(OBJECTS) $(LD_ARGS) -o $@

# hex from elf
$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

# bin from elf
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

# obj from c
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CC_ARGS) $(ASMLISTING) $< -o $@

# obj from c++ suffix .cpp
$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CXX) -c $(CXX_ARGS) $(ASMLISTING) $< -o $@

# obj from c++ user suffix
$(BUILD_DIR)/%.o: %$(USER_SUFFIX_CXX) Makefile | $(BUILD_DIR)
	$(CXX) -c $(CXX_ARGS) $(ASMLISTING) $< -o $@

# obj from asm
$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(AS_ARGS) $< -o $@

# create build dir
$(BUILD_DIR):
	mkdir $@

clean:
	-rm -fR $(BUILD_DIR)

#######################################
# openocd recipes
#######################################

openocd:
	$(OCD) -s $(OCD_SCRIPTS_DIR) $(OCDFLAGS)

debug:
	@if ! nc -z localhost 3333; then \
        echo "\n\t[Error] OpenOCD is not running! Start it with: 'make openocd'\n"; exit 1; \
    else \
        $(GDB)  -ex "target extended localhost:3333" \
            -ex "monitor arm semihosting enable" \
            -ex "monitor reset halt" \
            -ex "load" \
            -ex "monitor reset init" \
            $(GDBFLAGS) build/$(TARGET).elf; \
    fi

debug_client:
	ddd --eval-command="target remote localhost:3333" --debugger $(GDB) $(TARGET).elf

ifeq ($(APP_TYPE), BOOT_NONE)
program:
	$(OCD) -s $(OCD_SCRIPTS_DIR) $(OCDFLAGS) -c "program ./build/$(TARGET).elf verify reset exit"
else
program:
	$(error Cannot program via openocd with an app type of "$(APP_TYPE)". Try program-dfu instead)
endif


#######################################
# dfu-util
#######################################

program-dfu:
	dfu-util -a 0 -s $(FLASH_ADDRESS):leave -D $(BUILD_DIR)/$(TARGET).bin -d ,0483:$(USBPID)

program-boot:
	dfu-util -a 0 -s $(INTERNAL_ADDRESS):leave -D $(SYSTEM_FILES_DIR)$(BOOT_BIN) -d ,0483:$(STM_PID)


#######################################
# Build Debugging
#######################################

effective-build-settings:
	@echo TARGET ?= $(TARGET)
	@echo CXX_SOURCES += $(CXX_SOURCES)
	@echo LIBDAISY_DIR ?= $(LIBDAISY_DIR)
	@echo DEBUG ?= $(DEBUG)
	@echo BUILD_DIR ?= $(BUILD_DIR)
	@echo USER_SUFFIX_CXX ?= $(USER_SUFFIX_CXX)
	@echo C_SOURCES += $(C_SOURCES)
	@echo DAISYSP_DIR ?= $(DAISYSP_DIR)
	@echo USE_DAISYSP_LGPL ?= $(USE_DAISYSP_LGPL)
	@echo USE_FATFS ?= $(USE_FATFS)
	@echo GCC_DIR ?= $(GCC_DIR)
	@echo GCC_PREFIX ?= $(GCC_PREFIX)
	@echo OCD_PGM_DEVICE ?= $(OCD_PGM_DEVICE)
	@echo OCD_CHIPSET ?= $(OCD_CHIPSET)
	@echo OCD_SCRIPTS_DIR ?= $(OCD_SCRIPTS_DIR)
	@echo MCU ?= $(MCU)
	@echo STARTUP_CODE ?= $(STARTUP_CODE)
	@echo C_STANDARD ?= $(C_STANDARD)
	@echo CXX_STANDARD ?= $(CXX_STANDARD)
	@echo DAISY_DEFINES ?= $(DAISY_DEFINES)
	@echo DAISY_CFLAGS ?= $(DAISY_CFLAGS)
	@echo DAISY_CXXFLAGS ?= $(DAISY_CXXFLAGS)
	@echo DAISY_ASFLAGS ?= $(DAISY_ASFLAGS)
	@echo GCC_SPECS ?= $(GCC_SPECS)
	@echo LINKER_OPTS ?= $(LINKER_OPTS)
	@echo BOOT_BIN ?= $(BOOT_BIN)
	@echo APP_TYPE ?= $(APP_TYPE)
	@echo LINKERSCRIPT ?= $(LINKERSCRIPT)


-include $(wildcard $(BUILD_DIR)/*.d)
# *** EOF ***
