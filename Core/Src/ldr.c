#include "ldr.h"

/* Note that HAL_ADC_GetValue returns
 * 600 - 2500 for ambient light
 * below 500 when covered
 */
#define LDR_THRES 500

extern ADC_HandleTypeDef hadc2; // For PC4 LDR1
extern ADC_HandleTypeDef hadc3; // For PB1 LDR2

int ldr1_active(void)
{
    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
    uint16_t value = HAL_ADC_GetValue(&hadc2);
    HAL_ADC_Stop(&hadc2);

    return value < LDR_THRES ? 1 : 0;
}

int ldr2_active(void)
{
    HAL_ADC_Start(&hadc3);
    HAL_ADC_PollForConversion(&hadc3, HAL_MAX_DELAY);
    uint16_t value = HAL_ADC_GetValue(&hadc3);
    HAL_ADC_Stop(&hadc3);

    return value < LDR_THRES ? 1 : 0;
}





