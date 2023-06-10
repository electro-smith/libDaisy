add_library(FatFs STATIC
  src/diskio.c
  src/ff.c
  src/ff_gen_drv.c
)

target_include_directories(FatFs PUBLIC
  src
  ${MODULE_DIR}/sys
  ${MODULE_DIR} # for util/bsp_sd_diskio.h
)