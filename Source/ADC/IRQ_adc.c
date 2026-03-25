/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_adc.c
** Last modified Date:  2018-12-30
** Last Version:        V1.00
** Descriptions:        functions to manage A/D interrupts
** Correlated files:    adc.h
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "LPC17xx.h"
#include "../Main.h"

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/

uint16_t AD_current = 0;
extern uint16_t adc_speed_value;

void ADC_IRQHandler(void) {
    AD_current = ((LPC_ADC->ADGDR>>4) & 0xFFF);  // Read Conversion Result 0-4095
    
    // Map ADC value to speed: 0-4095 -> 1-5 squares/second
    // Speed 1 = ADC 0-819
    // Speed 2 = ADC 820-1638
    // Speed 3 = ADC 1639-2457
    // Speed 4 = ADC 2458-3276
    // Speed 5 = ADC 3277-4095
    adc_speed_value = (AD_current / 819) + 1;
    if (adc_speed_value > 5) adc_speed_value = 5;
    if (adc_speed_value < 1) adc_speed_value = 1;
}