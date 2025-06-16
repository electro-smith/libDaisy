enable_language(C ASM)
add_library(RTT STATIC
	RTT/SEGGER_RTT.c
	RTT/SEGGER_RTT_printf.c
	RTT/SEGGER_RTT_ASM_ARMv7M.S
)

target_include_directories(RTT PUBLIC
	Config
	RTT
)