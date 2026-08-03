/*
 * proj_keypad.c
 *
 *  Created on: 3 Aug 2026
 *      Author: matth
 */


#include "keypad.h"

// Row pins (Outputs)
GPIO_TypeDef* ROW_PORT[4] = {GPIOB, GPIOB, GPIOB, GPIOB};
uint16_t ROW_PIN[4] = {
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14
};

// Column pins (Inputs)
GPIO_TypeDef* COL_PORT[4] = {GPIOA, GPIOA, GPIOA, GPIOA};
uint16_t COL_PIN[4] = {
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11
};

// Key layout
const char keypad[4][4] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

char Keypad_GetKey(void)
{
    for(int row = 0; row < 4; row++)
    {
        // Set all rows HIGH
        for(int i = 0; i < 4; i++)
        {
            HAL_GPIO_WritePin(ROW_PORT[i], ROW_PIN[i], GPIO_PIN_SET);
        }

        // Pull current row LOW
        HAL_GPIO_WritePin(ROW_PORT[row], ROW_PIN[row], GPIO_PIN_RESET);

        HAL_Delay(1);

        // Check each column
        for(int col = 0; col < 4; col++)
        {
            if(HAL_GPIO_ReadPin(COL_PORT[col], COL_PIN[col]) == GPIO_PIN_RESET)
            {
                // Debounce
                HAL_Delay(20);

                while(HAL_GPIO_ReadPin(COL_PORT[col], COL_PIN[col]) == GPIO_PIN_RESET);

                HAL_Delay(20);

                return keypad[row][col];
            }
        }
    }

    return '\0';    // No key pressed
}
