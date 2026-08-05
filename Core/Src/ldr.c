#include "ldr.h"

extern ADC_HandleTypeDef hadc2; // For PC4 LDR1
extern ADC_HandleTypeDef hadc3; // For PB1 LDR2

uint16_t Read_LDR_PC4_LDR1(void)
{
    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
    uint16_t value = HAL_ADC_GetValue(&hadc2);
    HAL_ADC_Stop(&hadc2);

    return value;
}

uint16_t Read_LDR_PB1_LDR2(void)
{
    HAL_ADC_Start(&hadc3);
    HAL_ADC_PollForConversion(&hadc3, HAL_MAX_DELAY);
    uint16_t value = HAL_ADC_GetValue(&hadc3);
    HAL_ADC_Stop(&hadc3);

    return value;
}





