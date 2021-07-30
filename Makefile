TARGET = libdaisy

MODULE_DIR=src

#dev/sdram -- this will soon be removed in favor of startup initialization

C_MODULES = \
sys/dma \
hid/audio \
sys/fatfs \
per/sai \
per/sdmmc \
util/bsp_sd_diskio \
util/hal_map \
util/oled_fonts \
util/sd_diskio \
util/unique_id \
sys/system_stm32h7xx \
usbd/usbd_cdc_if \
usbd/usbd_desc \
usbd/usbd_conf 

CPP_MODULES = \
daisy_seed \
daisy_pod \
daisy_patch \
daisy_petal \
daisy_field \
daisy_versio \
sys/system \
dev/sr_595 \
dev/codec_ak4556 \
dev/codec_pcm3060 \
dev/codec_wm8731 \
dev/lcd_hd44780 \
hid/ctrl \
hid/encoder \
hid/gatein \
hid/led \
hid/parameter \
hid/rgb_led \
hid/switch \
hid/usb \
hid/wavplayer \
hid/logger \
per/adc \
per/dac \
per/gpio \
per/i2c \
per/qspi \
per/spi \
per/tim \
per/uart \
ui/UI \
ui/AbstractMenu \
ui/FullScreenItemMenu \
util/color \
util/MappedValue \
util/WaveTableLoader \

######################################
# building variables
######################################
DEBUG = 0
OPT = -O3

#######################################
# paths
#######################################

# Build path
BUILD_DIR = build

######################################
# source
######################################

# manually adding necessary HAL files
# generated by dump_filepath.py
C_SOURCES = 
C_SOURCES += \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cec.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_comp.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dcmi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dsi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hrtim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_irda.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_lptim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdios.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_msp_template.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nand.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nor.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ramecc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smbus.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spdifrx.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sram.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_swpmi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_timebase_rtc_alarm_template.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_timebase_rtc_wakeup_template.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_timebase_tim_template.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_wwdg.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_adc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_bdma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_comp.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_crc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dac.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_delayblock.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma2d.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_exti.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_hrtim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_i2c.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_lptim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_lpuart.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_mdma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_opamp.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_pwr.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rcc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rng.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rtc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_spi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_swpmi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_tim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usart.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_utils.c 

# Middleware sources
C_SOURCES += \
Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c \
Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
Middlewares/Third_Party/FatFs/src/diskio.c \
Middlewares/Third_Party/FatFs/src/ff.c \
Middlewares/Third_Party/FatFs/src/ff_gen_drv.c 

# C++ Source
CPP_SOURCES += $(addsuffix .cpp, $(addprefix $(MODULE_DIR)/, $(CPP_MODULES)))

# C sources
C_SOURCES += $(addsuffix .c, $(addprefix $(MODULE_DIR)/, $(C_MODULES)))
C_SOURCES += core/startup_stm32h750xx.c

#STARTUP_PATH = Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc
# ASM sources
#ASM_SOURCES =  \
#$(STARTUP_PATH)/startup_stm32h750xx.s

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
CXX = $(GCC_PATH)/$(PREFIX)g++
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
AR = $(GCC_PATH)/$(PREFIX)ar
GDB = $(GCC_PATH)/$(PREFIX)gdb
else
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
AR = $(PREFIX)ar
GDB = $(PREFIX)gdb
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=fpv5-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = -mthumb $(FLOAT-ABI) $(FPU) $(CPU)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DCORE_CM7  \
-DSTM32H750xx \
-DSTM32H750IB \
-DARM_MATH_CM7 \
-Dflash_layout \
-DHSE_VALUE=16000000 \
-DUSE_HAL_DRIVER \
-DUSE_FULL_LL_DRIVER \
-DDATA_IN_D2_SRAM 
# ^ added for easy startup access


C_INCLUDES = \
-I$(MODULE_DIR) \
-I$(MODULE_DIR)/sys \
-I$(MODULE_DIR)/usbd \
-IDrivers/CMSIS/Include \
-IDrivers/CMSIS/Device/ST/STM32H7xx/Include \
-IDrivers/STM32H7xx_HAL_Driver/Inc \
-IDrivers/STM32H7xx_HAL_Driver/Inc/Legacy \
-IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc \
-IMiddlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
-IMiddlewares/Third_Party/FatFs/src \
-I$(MODULE_DIR) \
-I. 

# suppressions for warnings introduced by HAL/FatFS
WARNINGS += -Wall -Wno-attributes -Wno-strict-aliasing -Wno-maybe-uninitialized -Wno-missing-attributes -Wno-stringop-overflow #-Werror
CPP_WARNINGS += -Wno-register

# compile gcc flags
ASFLAGS = $(MCU) $(AS_INCLUDES) $(AS_DEFS) -ggdb $(WARNINGS) $(OPT) -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_INCLUDES) $(C_DEFS) -ggdb $(WARNINGS) $(OPT) -fasm -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -ggdb
OPT = -O0
C_DEFS += -DDEBUG=1
else
C_DEFS += -DNDEBUG=1 -DRELEASE=1
endif

ifdef VOLATILE
C_DEFS += -DVECT_TAB_SRAM \
-DBOOT_VOLATILE
endif

CFLAGS += \
-finline-functions

# C++ Flags
CPPFLAGS = $(CFLAGS) $(CPP_WARNINGS)
CPPFLAGS += \
-fno-exceptions \
-fno-rtti 

C_STANDARD = -std=gnu11
CPP_STANDARD += -std=gnu++14

# default action: build all
all: $(BUILD_DIR)/$(TARGET).a

#######################################
# build the application
#######################################

# list of C program objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of CPP program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(CPP_SOURCES:.cpp=.o))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.s=.o))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# Prunes duplicates, and orders lexically (for archive build)
SORTED_OBJECTS = $(sort $(OBJECTS))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(C_STANDARD) -c $< -o $@ -MD -MP -MF $(BUILD_DIR)/$(notdir $(<:.c=.dep))

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CPP_STANDARD) -c $< -o $@ -MD -MP -MF $(BUILD_DIR)/$(notdir $(<:.cpp=.dep))

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	mkdir -p $(@D)
	$(AS) -c $(CFLAGS) $< -o $@ -MD -MP -MF $(BUILD_DIR)/$(notdir $(<:.s =.dep))

$(BUILD_DIR)/$(TARGET).a: $(SORTED_OBJECTS) Makefile
	$(AR) -r $@ $(SORTED_OBJECTS)

$(BUILD_DIR):
	mkdir $@        

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
#######################################

# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.dep)
