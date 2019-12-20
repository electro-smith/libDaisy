#include <string.h>
#include "daisy_seed.h"
//#include "daisysp.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// 1MB test
#define TEST_BUFF_SIZE (1024 * 1024)

using namespace daisy;
//using namespace daisysp;

//void SystemClock_Config(void);
//void Error_Handler();
//extern "C"
//{
//void SysTick_Handler(void)
//{
//    HAL_IncTick();
//    HAL_SYSTICK_IRQHandler();
//}
//
//void HardFault_Handler()
//{
//    asm("bkpt 255");
//}
//}

//static uint32_t DSY_SDRAM_BSS test_buff[TEST_BUFF_SIZE];

static daisy_handle hw;

UsbHandle usb_handle;
int main(void)
{
    // Initialize Hardware

    daisy_seed_init(&hw);
    hw.usb_handle.Init(UsbHandle::FS_BOTH);

    //	// Test RAM
    //	for(uint32_t i = 0; i < TEST_BUFF_SIZE; i++)
    //	{
    //		test_buff[i] = i;
    //	}
    //	for(uint32_t i = 0; i < TEST_BUFF_SIZE; i++)
    //	{
    //		if(test_buff[i] != i)
    //		{
    //			asm("bkpt 255");
    //		}
    //	}
    //	dsy_tim_start();
    int hello_count = 0;
    char     buff[512];
    char*   dynbuff;
    dynbuff = (char*)malloc(sizeof(char) * 32);
    while(1) 
    {
        //dsy_tim_delay_ms(500);
        HAL_Delay(500);
//        dsy_gpio_toggle(&hw.led);
//        dsy_gpio_toggle(&hw.testpoint);
        sprintf(buff, "Tick:\t%d\n", hello_count);
        hw.usb_handle.TransmitInternal((uint8_t*)buff, strlen(buff));
    }
}

//void SystemClock_Config(void)
//{
//    RCC_OscInitTypeDef       RCC_OscInitStruct   = {0};
//    RCC_ClkInitTypeDef       RCC_ClkInitStruct   = {0};
//    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
//
//    /** Supply configuration update enable 
//  */
//    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
//    /** Configure the main internal regulator output voltage 
//  */
//    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
//
//    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
//    /** Macro to configure the PLL clock source 
//  */
//    __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
//    /** Initializes the CPU, AHB and APB busses clocks 
//  */
//    RCC_OscInitStruct.OscillatorType
//        = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
//    RCC_OscInitStruct.HSEState      = RCC_HSE_ON;
//    RCC_OscInitStruct.HSI48State    = RCC_HSI48_ON;
//    RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;
//    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
//    RCC_OscInitStruct.PLL.PLLM      = 4;
//    RCC_OscInitStruct.PLL.PLLN      = 200;
//    RCC_OscInitStruct.PLL.PLLP      = 2;
//    RCC_OscInitStruct.PLL.PLLQ      = 5;
//    RCC_OscInitStruct.PLL.PLLR      = 2;
//    RCC_OscInitStruct.PLL.PLLRGE    = RCC_PLL1VCIRANGE_2;
//    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
//    RCC_OscInitStruct.PLL.PLLFRACN  = 0;
//    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//    {
//        Error_Handler();
//    }
//    /** Initializes the CPU, AHB and APB busses clocks 
//  */
//    RCC_ClkInitStruct.ClockType
//        = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
//          | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
//    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
//    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
//    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
//    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
//    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
//    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
//    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
//
//    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
//    {
//        Error_Handler();
//    }
//    PeriphClkInitStruct.PeriphClockSelection
//        = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_RNG | RCC_PERIPHCLK_SPI1
//          | RCC_PERIPHCLK_SAI2 | RCC_PERIPHCLK_SAI1 | RCC_PERIPHCLK_SDMMC
//          | RCC_PERIPHCLK_I2C2 | RCC_PERIPHCLK_ADC | RCC_PERIPHCLK_I2C1
//          | RCC_PERIPHCLK_USB | RCC_PERIPHCLK_QSPI | RCC_PERIPHCLK_FMC;
//    PeriphClkInitStruct.PLL2.PLL2M            = 4;
//    PeriphClkInitStruct.PLL2.PLL2N            = 100;
//    PeriphClkInitStruct.PLL2.PLL2P            = 1;
//    PeriphClkInitStruct.PLL2.PLL2Q            = 2;
//    PeriphClkInitStruct.PLL2.PLL2R            = 2;
//    PeriphClkInitStruct.PLL2.PLL2RGE          = RCC_PLL2VCIRANGE_2;
//    PeriphClkInitStruct.PLL2.PLL2VCOSEL       = RCC_PLL2VCOWIDE;
//    PeriphClkInitStruct.PLL2.PLL2FRACN        = 0;
//    PeriphClkInitStruct.PLL3.PLL3M            = 6;
//    PeriphClkInitStruct.PLL3.PLL3N            = 295;
//    PeriphClkInitStruct.PLL3.PLL3P            = 64;
//    PeriphClkInitStruct.PLL3.PLL3Q            = 2;
//    PeriphClkInitStruct.PLL3.PLL3R            = 1;
//    PeriphClkInitStruct.PLL3.PLL3RGE          = RCC_PLL3VCIRANGE_1;
//    PeriphClkInitStruct.PLL3.PLL3VCOSEL       = RCC_PLL3VCOWIDE;
//    PeriphClkInitStruct.PLL3.PLL3FRACN        = 0;
//    PeriphClkInitStruct.FmcClockSelection     = RCC_FMCCLKSOURCE_D1HCLK;
//    PeriphClkInitStruct.QspiClockSelection    = RCC_QSPICLKSOURCE_D1HCLK;
//    PeriphClkInitStruct.SdmmcClockSelection   = RCC_SDMMCCLKSOURCE_PLL;
//    PeriphClkInitStruct.Sai1ClockSelection    = RCC_SAI1CLKSOURCE_PLL3;
//    PeriphClkInitStruct.Sai23ClockSelection   = RCC_SAI23CLKSOURCE_PLL3;
//    PeriphClkInitStruct.Spi123ClockSelection  = RCC_SPI123CLKSOURCE_PLL;
//    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
//    PeriphClkInitStruct.RngClockSelection     = RCC_RNGCLKSOURCE_HSI48;
//    PeriphClkInitStruct.I2c123ClockSelection  = RCC_I2C123CLKSOURCE_D2PCLK1;
//    PeriphClkInitStruct.UsbClockSelection     = RCC_USBCLKSOURCE_HSI48;
//    PeriphClkInitStruct.AdcClockSelection     = RCC_ADCCLKSOURCE_PLL2;
//    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
//    {
//        Error_Handler();
//    }
//    /** Enable USB Voltage detector 
//  */
//    HAL_PWREx_EnableUSBVoltageDetector();
//}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
//void Error_Handler(void)
//{
//    /* USER CODE BEGIN Error_Handler_Debug */
//    /* User can add his own implementation to report the HAL error return state */
//
//    /* USER CODE END Error_Handler_Debug */
//}
