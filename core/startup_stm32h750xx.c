/*
	This file contains the entry point (Reset_Handler) of your firmware project.
	The reset handled initializes the RAM and calls system library initializers as well as
	the platform-specific initializer and the main() function.
*/

#include <stddef.h>
extern void *_estack;

void Reset_Handler();
void Default_Handler();
//#define DEBUG_DEFAULT_INTERRUPT_HANDLERS
#ifdef DEBUG_DEFAULT_INTERRUPT_HANDLERS
void __attribute__ ((weak)) NMI_Handler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void NMI_Handler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) HardFault_Handler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void HardFault_Handler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) MemManage_Handler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void MemManage_Handler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) BusFault_Handler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void BusFault_Handler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) UsageFault_Handler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void UsageFault_Handler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SVC_Handler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SVC_Handler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DebugMon_Handler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DebugMon_Handler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) PendSV_Handler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void PendSV_Handler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SysTick_Handler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SysTick_Handler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) WWDG_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void WWDG_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) PVD_AVD_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void PVD_AVD_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TAMP_STAMP_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TAMP_STAMP_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) RTC_WKUP_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void RTC_WKUP_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) FLASH_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void FLASH_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) RCC_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void RCC_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) EXTI0_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void EXTI0_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) EXTI1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void EXTI1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) EXTI2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void EXTI2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) EXTI3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void EXTI3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) EXTI4_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void EXTI4_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA1_Stream0_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA1_Stream0_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA1_Stream1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA1_Stream1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA1_Stream2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA1_Stream2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA1_Stream3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA1_Stream3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA1_Stream4_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA1_Stream4_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA1_Stream5_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA1_Stream5_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA1_Stream6_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA1_Stream6_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) ADC_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void ADC_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) FDCAN1_IT0_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void FDCAN1_IT0_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) FDCAN2_IT0_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void FDCAN2_IT0_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) FDCAN1_IT1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void FDCAN1_IT1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) FDCAN2_IT1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void FDCAN2_IT1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) EXTI9_5_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void EXTI9_5_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM1_BRK_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM1_BRK_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM1_UP_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM1_UP_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM1_TRG_COM_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM1_TRG_COM_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM1_CC_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM1_CC_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM4_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM4_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) I2C1_EV_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void I2C1_EV_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) I2C1_ER_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void I2C1_ER_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) I2C2_EV_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void I2C2_EV_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) I2C2_ER_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void I2C2_ER_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SPI1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SPI1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SPI2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SPI2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) USART1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void USART1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) USART2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void USART2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) USART3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void USART3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) EXTI15_10_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void EXTI15_10_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) RTC_Alarm_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void RTC_Alarm_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM8_BRK_TIM12_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM8_BRK_TIM12_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM8_UP_TIM13_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM8_UP_TIM13_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM8_TRG_COM_TIM14_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM8_TRG_COM_TIM14_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM8_CC_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM8_CC_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA1_Stream7_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA1_Stream7_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) FMC_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void FMC_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SDMMC1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SDMMC1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM5_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM5_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SPI3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SPI3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) UART4_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void UART4_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) UART5_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void UART5_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM6_DAC_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM6_DAC_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM7_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM7_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA2_Stream0_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA2_Stream0_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA2_Stream1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA2_Stream1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA2_Stream2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA2_Stream2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA2_Stream3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA2_Stream3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA2_Stream4_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA2_Stream4_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) ETH_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void ETH_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) ETH_WKUP_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void ETH_WKUP_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) FDCAN_CAL_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void FDCAN_CAL_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA2_Stream5_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA2_Stream5_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA2_Stream6_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA2_Stream6_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA2_Stream7_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA2_Stream7_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) USART6_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void USART6_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) I2C3_EV_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void I2C3_EV_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) I2C3_ER_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void I2C3_ER_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) OTG_HS_EP1_OUT_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void OTG_HS_EP1_OUT_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) OTG_HS_EP1_IN_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void OTG_HS_EP1_IN_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) OTG_HS_WKUP_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void OTG_HS_WKUP_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) OTG_HS_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void OTG_HS_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DCMI_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DCMI_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) CRYP_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void CRYP_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) HASH_RNG_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void HASH_RNG_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) FPU_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void FPU_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) UART7_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void UART7_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) UART8_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void UART8_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SPI4_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SPI4_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SPI5_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SPI5_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SPI6_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SPI6_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SAI1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SAI1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) LTDC_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void LTDC_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) LTDC_ER_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void LTDC_ER_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMA2D_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMA2D_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SAI2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SAI2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) QUADSPI_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void QUADSPI_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) LPTIM1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void LPTIM1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) CEC_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void CEC_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) I2C4_EV_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void I2C4_EV_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) I2C4_ER_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void I2C4_ER_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SPDIF_RX_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SPDIF_RX_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) OTG_FS_EP1_OUT_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void OTG_FS_EP1_OUT_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) OTG_FS_EP1_IN_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void OTG_FS_EP1_IN_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) OTG_FS_WKUP_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void OTG_FS_WKUP_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) OTG_FS_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void OTG_FS_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMAMUX1_OVR_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMAMUX1_OVR_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) HRTIM1_Master_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void HRTIM1_Master_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) HRTIM1_TIMA_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void HRTIM1_TIMA_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) HRTIM1_TIMB_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void HRTIM1_TIMB_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) HRTIM1_TIMC_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void HRTIM1_TIMC_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) HRTIM1_TIMD_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void HRTIM1_TIMD_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) HRTIM1_TIME_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void HRTIM1_TIME_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) HRTIM1_FLT_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void HRTIM1_FLT_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DFSDM1_FLT0_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DFSDM1_FLT0_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DFSDM1_FLT1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DFSDM1_FLT1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DFSDM1_FLT2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DFSDM1_FLT2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DFSDM1_FLT3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DFSDM1_FLT3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SAI3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SAI3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SWPMI1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SWPMI1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM15_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM15_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM16_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM16_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) TIM17_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void TIM17_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) MDIOS_WKUP_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void MDIOS_WKUP_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) MDIOS_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void MDIOS_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) JPEG_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void JPEG_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) MDMA_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void MDMA_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SDMMC2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SDMMC2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) HSEM1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void HSEM1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) ADC3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void ADC3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) DMAMUX2_OVR_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void DMAMUX2_OVR_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) BDMA_Channel0_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void BDMA_Channel0_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) BDMA_Channel1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void BDMA_Channel1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) BDMA_Channel2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void BDMA_Channel2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) BDMA_Channel3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void BDMA_Channel3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) BDMA_Channel4_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void BDMA_Channel4_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) BDMA_Channel5_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void BDMA_Channel5_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) BDMA_Channel6_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void BDMA_Channel6_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) BDMA_Channel7_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void BDMA_Channel7_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) COMP1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void COMP1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) LPTIM2_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void LPTIM2_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) LPTIM3_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void LPTIM3_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) LPTIM4_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void LPTIM4_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) LPTIM5_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void LPTIM5_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) LPUART1_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void LPUART1_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) CRS_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void CRS_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) ECC_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void ECC_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) SAI4_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void SAI4_IRQHandler();
	asm("bkpt 255");
}

void __attribute__ ((weak)) WAKEUP_PIN_IRQHandler() 
{
	//If you hit the breakpoint below, one of the interrupts was unhandled in your code. 
	//Define the following function in your code to handle it:
	//	extern "C" void WAKEUP_PIN_IRQHandler();
	asm("bkpt 255");
}

#else
void NMI_Handler()                    __attribute__ ((weak, alias ("Default_Handler")));
void HardFault_Handler()              __attribute__ ((weak, alias ("Default_Handler")));
void MemManage_Handler()              __attribute__ ((weak, alias ("Default_Handler")));
void BusFault_Handler()               __attribute__ ((weak, alias ("Default_Handler")));
void UsageFault_Handler()             __attribute__ ((weak, alias ("Default_Handler")));
void SVC_Handler()                    __attribute__ ((weak, alias ("Default_Handler")));
void DebugMon_Handler()               __attribute__ ((weak, alias ("Default_Handler")));
void PendSV_Handler()                 __attribute__ ((weak, alias ("Default_Handler")));
void SysTick_Handler()                __attribute__ ((weak, alias ("Default_Handler")));
void WWDG_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void PVD_AVD_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void TAMP_STAMP_IRQHandler()          __attribute__ ((weak, alias ("Default_Handler")));
void RTC_WKUP_IRQHandler()            __attribute__ ((weak, alias ("Default_Handler")));
void FLASH_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void RCC_IRQHandler()                 __attribute__ ((weak, alias ("Default_Handler")));
void EXTI0_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void EXTI1_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void EXTI2_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void EXTI3_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void EXTI4_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Stream0_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Stream1_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Stream2_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Stream3_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Stream4_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Stream5_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Stream6_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void ADC_IRQHandler()                 __attribute__ ((weak, alias ("Default_Handler")));
void FDCAN1_IT0_IRQHandler()          __attribute__ ((weak, alias ("Default_Handler")));
void FDCAN2_IT0_IRQHandler()          __attribute__ ((weak, alias ("Default_Handler")));
void FDCAN1_IT1_IRQHandler()          __attribute__ ((weak, alias ("Default_Handler")));
void FDCAN2_IT1_IRQHandler()          __attribute__ ((weak, alias ("Default_Handler")));
void EXTI9_5_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void TIM1_BRK_IRQHandler()            __attribute__ ((weak, alias ("Default_Handler")));
void TIM1_UP_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void TIM1_TRG_COM_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void TIM1_CC_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void TIM2_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void TIM3_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void TIM4_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void I2C1_EV_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void I2C1_ER_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void I2C2_EV_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void I2C2_ER_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void SPI1_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void SPI2_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void USART1_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void USART2_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void USART3_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void EXTI15_10_IRQHandler()           __attribute__ ((weak, alias ("Default_Handler")));
void RTC_Alarm_IRQHandler()           __attribute__ ((weak, alias ("Default_Handler")));
void TIM8_BRK_TIM12_IRQHandler()      __attribute__ ((weak, alias ("Default_Handler")));
void TIM8_UP_TIM13_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void TIM8_TRG_COM_TIM14_IRQHandler()  __attribute__ ((weak, alias ("Default_Handler")));
void TIM8_CC_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Stream7_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void FMC_IRQHandler()                 __attribute__ ((weak, alias ("Default_Handler")));
void SDMMC1_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void TIM5_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void SPI3_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void UART4_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void UART5_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void TIM6_DAC_IRQHandler()            __attribute__ ((weak, alias ("Default_Handler")));
void TIM7_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Stream0_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Stream1_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Stream2_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Stream3_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Stream4_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void ETH_IRQHandler()                 __attribute__ ((weak, alias ("Default_Handler")));
void ETH_WKUP_IRQHandler()            __attribute__ ((weak, alias ("Default_Handler")));
void FDCAN_CAL_IRQHandler()           __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Stream5_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Stream6_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Stream7_IRQHandler()        __attribute__ ((weak, alias ("Default_Handler")));
void USART6_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void I2C3_EV_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void I2C3_ER_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void OTG_HS_EP1_OUT_IRQHandler()      __attribute__ ((weak, alias ("Default_Handler")));
void OTG_HS_EP1_IN_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void OTG_HS_WKUP_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void OTG_HS_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void DCMI_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void CRYP_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void HASH_RNG_IRQHandler()            __attribute__ ((weak, alias ("Default_Handler")));
void FPU_IRQHandler()                 __attribute__ ((weak, alias ("Default_Handler")));
void UART7_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void UART8_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void SPI4_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void SPI5_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void SPI6_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void SAI1_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void LTDC_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void LTDC_ER_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void DMA2D_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void SAI2_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void QUADSPI_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void LPTIM1_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void CEC_IRQHandler()                 __attribute__ ((weak, alias ("Default_Handler")));
void I2C4_EV_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void I2C4_ER_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void SPDIF_RX_IRQHandler()            __attribute__ ((weak, alias ("Default_Handler")));
void OTG_FS_EP1_OUT_IRQHandler()      __attribute__ ((weak, alias ("Default_Handler")));
void OTG_FS_EP1_IN_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void OTG_FS_WKUP_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void OTG_FS_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void DMAMUX1_OVR_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void HRTIM1_Master_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void HRTIM1_TIMA_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void HRTIM1_TIMB_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void HRTIM1_TIMC_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void HRTIM1_TIMD_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void HRTIM1_TIME_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void HRTIM1_FLT_IRQHandler()          __attribute__ ((weak, alias ("Default_Handler")));
void DFSDM1_FLT0_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void DFSDM1_FLT1_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void DFSDM1_FLT2_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void DFSDM1_FLT3_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void SAI3_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void SWPMI1_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void TIM15_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void TIM16_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void TIM17_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void MDIOS_WKUP_IRQHandler()          __attribute__ ((weak, alias ("Default_Handler")));
void MDIOS_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void JPEG_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void MDMA_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void SDMMC2_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void HSEM1_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void ADC3_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void DMAMUX2_OVR_IRQHandler()         __attribute__ ((weak, alias ("Default_Handler")));
void BDMA_Channel0_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void BDMA_Channel1_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void BDMA_Channel2_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void BDMA_Channel3_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void BDMA_Channel4_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void BDMA_Channel5_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void BDMA_Channel6_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void BDMA_Channel7_IRQHandler()       __attribute__ ((weak, alias ("Default_Handler")));
void COMP1_IRQHandler()               __attribute__ ((weak, alias ("Default_Handler")));
void LPTIM2_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void LPTIM3_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void LPTIM4_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void LPTIM5_IRQHandler()              __attribute__ ((weak, alias ("Default_Handler")));
void LPUART1_IRQHandler()             __attribute__ ((weak, alias ("Default_Handler")));
void CRS_IRQHandler()                 __attribute__ ((weak, alias ("Default_Handler")));
void ECC_IRQHandler()                 __attribute__ ((weak, alias ("Default_Handler")));
void SAI4_IRQHandler()                __attribute__ ((weak, alias ("Default_Handler")));
void WAKEUP_PIN_IRQHandler()          __attribute__ ((weak, alias ("Default_Handler")));
#endif

void * g_pfnVectors[0xa6] __attribute__ ((section (".isr_vector"), used)) = 
{
	&_estack,
	&Reset_Handler,
	&NMI_Handler,
	&HardFault_Handler,
	&MemManage_Handler,
	&BusFault_Handler,
	&UsageFault_Handler,
	NULL,
	NULL,
	NULL,
	NULL,
	&SVC_Handler,
	&DebugMon_Handler,
	NULL,
	&PendSV_Handler,
	&SysTick_Handler,
	&WWDG_IRQHandler,
	&PVD_AVD_IRQHandler,
	&TAMP_STAMP_IRQHandler,
	&RTC_WKUP_IRQHandler,
	&FLASH_IRQHandler,
	&RCC_IRQHandler,
	&EXTI0_IRQHandler,
	&EXTI1_IRQHandler,
	&EXTI2_IRQHandler,
	&EXTI3_IRQHandler,
	&EXTI4_IRQHandler,
	&DMA1_Stream0_IRQHandler,
	&DMA1_Stream1_IRQHandler,
	&DMA1_Stream2_IRQHandler,
	&DMA1_Stream3_IRQHandler,
	&DMA1_Stream4_IRQHandler,
	&DMA1_Stream5_IRQHandler,
	&DMA1_Stream6_IRQHandler,
	&ADC_IRQHandler,
	&FDCAN1_IT0_IRQHandler,
	&FDCAN2_IT0_IRQHandler,
	&FDCAN1_IT1_IRQHandler,
	&FDCAN2_IT1_IRQHandler,
	&EXTI9_5_IRQHandler,
	&TIM1_BRK_IRQHandler,
	&TIM1_UP_IRQHandler,
	&TIM1_TRG_COM_IRQHandler,
	&TIM1_CC_IRQHandler,
	&TIM2_IRQHandler,
	&TIM3_IRQHandler,
	&TIM4_IRQHandler,
	&I2C1_EV_IRQHandler,
	&I2C1_ER_IRQHandler,
	&I2C2_EV_IRQHandler,
	&I2C2_ER_IRQHandler,
	&SPI1_IRQHandler,
	&SPI2_IRQHandler,
	&USART1_IRQHandler,
	&USART2_IRQHandler,
	&USART3_IRQHandler,
	&EXTI15_10_IRQHandler,
	&RTC_Alarm_IRQHandler,
	NULL,
	&TIM8_BRK_TIM12_IRQHandler,
	&TIM8_UP_TIM13_IRQHandler,
	&TIM8_TRG_COM_TIM14_IRQHandler,
	&TIM8_CC_IRQHandler,
	&DMA1_Stream7_IRQHandler,
	&FMC_IRQHandler,
	&SDMMC1_IRQHandler,
	&TIM5_IRQHandler,
	&SPI3_IRQHandler,
	&UART4_IRQHandler,
	&UART5_IRQHandler,
	&TIM6_DAC_IRQHandler,
	&TIM7_IRQHandler,
	&DMA2_Stream0_IRQHandler,
	&DMA2_Stream1_IRQHandler,
	&DMA2_Stream2_IRQHandler,
	&DMA2_Stream3_IRQHandler,
	&DMA2_Stream4_IRQHandler,
	&ETH_IRQHandler,
	&ETH_WKUP_IRQHandler,
	&FDCAN_CAL_IRQHandler,
	NULL,
	NULL,
	NULL,
	NULL,
	&DMA2_Stream5_IRQHandler,
	&DMA2_Stream6_IRQHandler,
	&DMA2_Stream7_IRQHandler,
	&USART6_IRQHandler,
	&I2C3_EV_IRQHandler,
	&I2C3_ER_IRQHandler,
	&OTG_HS_EP1_OUT_IRQHandler,
	&OTG_HS_EP1_IN_IRQHandler,
	&OTG_HS_WKUP_IRQHandler,
	&OTG_HS_IRQHandler,
	&DCMI_IRQHandler,
	&CRYP_IRQHandler,
	&HASH_RNG_IRQHandler,
	&FPU_IRQHandler,
	&UART7_IRQHandler,
	&UART8_IRQHandler,
	&SPI4_IRQHandler,
	&SPI5_IRQHandler,
	&SPI6_IRQHandler,
	&SAI1_IRQHandler,
	&LTDC_IRQHandler,
	&LTDC_ER_IRQHandler,
	&DMA2D_IRQHandler,
	&SAI2_IRQHandler,
	&QUADSPI_IRQHandler,
	&LPTIM1_IRQHandler,
	&CEC_IRQHandler,
	&I2C4_EV_IRQHandler,
	&I2C4_ER_IRQHandler,
	&SPDIF_RX_IRQHandler,
	&OTG_FS_EP1_OUT_IRQHandler,
	&OTG_FS_EP1_IN_IRQHandler,
	&OTG_FS_WKUP_IRQHandler,
	&OTG_FS_IRQHandler,
	&DMAMUX1_OVR_IRQHandler,
	&HRTIM1_Master_IRQHandler,
	&HRTIM1_TIMA_IRQHandler,
	&HRTIM1_TIMB_IRQHandler,
	&HRTIM1_TIMC_IRQHandler,
	&HRTIM1_TIMD_IRQHandler,
	&HRTIM1_TIME_IRQHandler,
	&HRTIM1_FLT_IRQHandler,
	&DFSDM1_FLT0_IRQHandler,
	&DFSDM1_FLT1_IRQHandler,
	&DFSDM1_FLT2_IRQHandler,
	&DFSDM1_FLT3_IRQHandler,
	&SAI3_IRQHandler,
	&SWPMI1_IRQHandler,
	&TIM15_IRQHandler,
	&TIM16_IRQHandler,
	&TIM17_IRQHandler,
	&MDIOS_WKUP_IRQHandler,
	&MDIOS_IRQHandler,
	&JPEG_IRQHandler,
	&MDMA_IRQHandler,
	NULL,
	&SDMMC2_IRQHandler,
	&HSEM1_IRQHandler,
	NULL,
	&ADC3_IRQHandler,
	&DMAMUX2_OVR_IRQHandler,
	&BDMA_Channel0_IRQHandler,
	&BDMA_Channel1_IRQHandler,
	&BDMA_Channel2_IRQHandler,
	&BDMA_Channel3_IRQHandler,
	&BDMA_Channel4_IRQHandler,
	&BDMA_Channel5_IRQHandler,
	&BDMA_Channel6_IRQHandler,
	&BDMA_Channel7_IRQHandler,
	&COMP1_IRQHandler,
	&LPTIM2_IRQHandler,
	&LPTIM3_IRQHandler,
	&LPTIM4_IRQHandler,
	&LPTIM5_IRQHandler,
	&LPUART1_IRQHandler,
	NULL,
	&CRS_IRQHandler,
	&ECC_IRQHandler,
	&SAI4_IRQHandler,
	NULL,
	NULL,
	&WAKEUP_PIN_IRQHandler,
};

void SystemInit();
void __libc_init_array();
int main();

extern void *_sidata, *_sdata, *_edata;
extern void *_sbss, *_ebss;

void __attribute__((naked, noreturn)) Reset_Handler()
{
	//Normally the CPU should will setup the based on the value from the first entry in the vector table.
	//If you encounter problems with accessing stack variables during initialization, ensure the line below is enabled.
	#ifdef sram_layout
	asm ("ldr sp, =_estack");
	#endif

	void **pSource, **pDest;
	for (pSource = &_sidata, pDest = &_sdata; pDest != &_edata; pSource++, pDest++)
		*pDest = *pSource;

	for (pDest = &_sbss; pDest != &_ebss; pDest++)
		*pDest = 0;

	#ifndef BOOT_APP
	SystemInit();
	#endif
	__libc_init_array();


	(void)main();
	for (;;) ;
}

void __attribute__((naked, noreturn)) Default_Handler()
{
	//If you get stuck here, your code is missing a handler for some interrupt.
	//Define a 'DEBUG_DEFAULT_INTERRUPT_HANDLERS' macro via VisualGDB Project Properties and rebuild your project.
	//This will pinpoint a specific missing vector.
	for (;;) ;
}
