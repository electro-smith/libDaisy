
Stephen's feedback upon useage of libdaisy for the benefit of the users.

 - The headers are not C++ compatible, need the standard #ifdef __cplusplus / extern "C" { wrappers
 - libdaisy.h is included with <> even though its in the same directory. This is safer to use "" as it doesn't depend on compiler settings or compiler include behavior. please dont _ever_ separate your include files from your source files. 
 - why not pass the audio callback to dsy_audio_init? not sure you would ever not want to set the callback if you are initing it. You can set a default argumen to empty(). Or not as you are C not C++ :(
 - audio sample rate should be configurable in code (this is stupidly hard to do well with cube but really easy setting the registers directly if you know the SAI input clock hz)
 - #pragma once is your friend
 - defining DMA1_Stream0_IRQHandler conflicts with a default cube generated project as HAL has its own internal definition of this (gonna be tricky to fix this cleanly you probaly need to overwrite the IRQ vector  with your differently named function since this function is technically dynamicly linked: g_pfnVectors[vector] = my_dma_callback;)
 -
 
