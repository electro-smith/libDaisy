add_library(FatFs STATIC
  src/diskio.c
  src/ff.c
  src/ff_gen_drv.c

  # due to config (in src/sys) which has _USE_LFN = 1 
  src/option/unicode.c
)

target_include_directories(FatFs PUBLIC
  src
  ${MODULE_DIR}/sys
  ${MODULE_DIR} # for util/bsp_sd_diskio.h
)