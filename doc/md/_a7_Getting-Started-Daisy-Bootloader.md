# Getting Started - Daisy Bootloader

In the context of embedded applications, a bootloader is a small program that runs on bootup and manages loading and even updating target applications. The update routine of the STM32H7's system bootloader can be accessed on the Daisy by resetting the device with the BOOT button held down.
However, the built-in bootloader can only reprogram the chip's internal flash (128kB). With this setup, it's not so easy to store larger programs in non-volatile memory, and it's even harder to get them running. That's where the Daisy bootloader comes in.

## Advantages of the Daisy bootloader

With the Daisy bootloader, you can easily run significantly larger programs (up to 480kB on SRAM and almost 8MB on QSPI flash). Execution from SRAM has comparable speed to the internal flash, while the QSPI flash is naturally a fair bit slower. The update process for end-users can also be made extremely simple with SD cards or USB drives; simply starting up the Daisy with media plugged in will launch a search for valid executable binaries.

## Flashing the bootloader

With the latest version of libDaisy, you should be able to flash the bootloader using the `program-boot` rule within any project. The bootloader will reside in the internal flash like any normal application, so you'll need to go through the [normal DFU procedure](https://github.com/electro-smith/DaisyWiki/wiki/1.-Setting-Up-Your-Development-Environment#4a-flashing-the-daisy-via-usb) before running `make program-boot`.

## Generating programs for the bootloader

To generate programs that can run from the bootloader, you only need one additional line in your makefile:

~~~makefile
APP_TYPE = BOOT_SRAM
~~~

The valid `APP_TYPE`s are `BOOT_SRAM` (runs program on the internal SRAM), `BOOT_QSPI` (runs programs on the QSPI flash chip), and `BOOT_NONE` (which just behaves like normal and is unable to run from the bootloader). Once you've added the desired app type, you'll need to recompile your project. It should be obvious from the memory usage output where your program will run. For example, when compiling for SRAM, you might see something like:

~~~sh
Memory region         Used Size  Region Size  %age Used
       FLASH:          0 GB       128 KB      0.00%
     DTCMRAM:        7440 B       128 KB      5.68%
        SRAM:       46000 B       512 KB      8.77%
      RAM_D2_DMA:     16 KB        32 KB     50.00%
      RAM_D2:          0 GB       256 KB      0.00%
      RAM_D3:          0 GB        64 KB      0.00%
     ITCMRAM:          0 GB        64 KB      0.00%
       SDRAM:          0 GB        64 MB      0.00%
   QSPIFLASH:          0 GB      7936 KB      0.00%
~~~

Notice the zero bytes used in FLASH, which is the chip's internal flash storage. Since the bootloader lives there, any program that you intend to run with it can't use that region.

Compiling for QSPI will move the program over into the QSPIFLASH region:

~~~sh
Memory region         Used Size  Region Size  %age Used
       FLASH:          0 GB       128 KB      0.00%
     DTCMRAM:          0 GB       128 KB      0.00%
        SRAM:        7440 B       512 KB      1.42%
  RAM_D2_DMA:         16 KB        32 KB     50.00%
      RAM_D2:          0 GB       256 KB      0.00%
      RAM_D3:          0 GB        64 KB      0.00%
     ITCMRAM:          0 GB        64 KB      0.00%
       SDRAM:          0 GB        64 MB      0.00%
   QSPIFLASH:       46000 B      7936 KB      0.57%
~~~

## Flashing with the bootloader

With your program compiled, you have a few options available for programming it using the bootloader:

### DFU

If the bootloader's LED is pulsing in the grace period, and the Daisy is connected to your computer via USB, you can run `make program-dfu`. The `APP_TYPE` will automatically adjust the DFU command to write to the correct address. Note that you'll have to change the app type back to `BOOT_NONE` or remove it to flash non-bootloader programs.

### SD Card

If your Daisy is connected to a micro SD card slot, you can just drag and drop the `.bin` file generated in your project's build folder onto the card. If you then plug it into the Daisy and restart it, the bootloader will automatically flash it to the QSPI chip.

At this time, the SD card needs to be fully wired up with all of the 4-bit data lines connected.

### USB Drive

If your Daisy's external USB pins (D29 and D30) are connected to a USB port, you can follow the same process as SD cards above.

## Custom linkers

From the two examples above, you'll notice that the `BOOT_SRAM` type does not use any SRAM as RAM. Instead, it's all placed in DTCMRAM, which is only 128kB. The `BOOT_QSPI` does use the SRAM, but the program's execution can be a fair bit slower running from QSPI. Essentially, each configuration has its own trade-offs. If you want to customize those trade-offs, you can write your own linker to determine where each part of your program is placed. 

For a starting point, you can look through the linkers provided in libDaisy's `core` folder. Non-bootloader programs running on internal flash are linked with the `STM32H750IB_flash.lds` script, while the other two `.lds` scripts handle their respective configurations. Right now, the Daisy bootloader will only accept QSPI and SRAM as valid locations for programs, so custom linkers must place the `.isr_vector` and `.text` sections in these regions.

## Detailed Behavior

Once flashed, the bootloader has a grace period of 2.5 seconds on startup indicated by sinusoidal LED blinks. During this time, it will listen for DFU transactions over USB and search any connected media for valid binaries. Once this period elapses, the bootloader will attempt to load a program and jump to it. If no program is present, it will simply wait in the grace period until a DFU transaction occurs. You can extend the grace period indefinitely by pressing the BOOT button (the bootloader will acknowledge the extension with a few rapid blinks).

Programs are stored on the QSPI flash chip that comes with every Daisy. The first four 64kB sectors are left untouched, though these may be used in the future for additional features. Sectors are erased in 64kB chunks, so if you plan to use the space beyond the program, ensure that the first address lies on a 64k boundary beyond the size of the program code. In the STM32H7's address space, QSPI flash lies at address `0x90000000`, so programs are stored at `0x90040000`.

The bootloader needs a portion of the SRAM for its own processes. It uses 32kB located at the end of the region, so any programs run from SRAM cannot be greater than 480kB. You are free to use this space for any normal RAM sections, however.

For SD cards and USB drives, the executable binary search is not very sophisticated -- it simply scans through the files in the root directory of the connected media looking for a file with an extension of `.bin`. Once a `.bin` file is found, the search will cease and the binary will be verified as executable. If that check passes, the bootloader will compare the file to what's already stored in QSPI and flash it if the file isn't the same.

SD Cards, if present, are checked before USB drives. If a binary is found on the SD card, the USB drive will be skipped whether it was valid or not.

If any error is encountered in the bootloading process, the Daisy will emit a single SOS pattern on the user LED and then continue as normal. The primary cause of errors is invalid programs, which can be encountered after a DFU transaction or in a media search. If you see an SOS, double check any connected media for stray `.bin` files and make sure you're not uploading a program that's supposed to run on the internal flash.
