/*
 * ShiftReg.c
 *
 *  Created on: 6 Aug 2026
 *      Author: pengyuliu
 */

#include "main.h"

uint16_t sr = 0xF000;

static void do_sr_display(uint16_t num);

void ShiftRegDisplay(void) {
	do_sr_display(sr);
}

static void do_sr_display(uint16_t num) {
	for (int i = 0; i < 16; i++) {
		int v = num & (1 << i) ? 1 : 0;
		HAL_GPIO_WritePin(GPIOB, SR_SER_Pin, v);
		HAL_GPIO_WritePin(GPIOC, SR_SCK_Pin, 1);
		HAL_GPIO_WritePin(GPIOC, SR_SCK_Pin, 0);
	}
	HAL_GPIO_WritePin(GPIOB, SR_RCK_Pin, 1);
	HAL_GPIO_WritePin(GPIOB, SR_RCK_Pin, 0);
}

void ShiftRegRightShift(void) {
	int lsb = sr & 1;
	sr >>= 1;
	if (lsb) {
		sr |= 0x8000;
	}
	do_sr_display(sr);
}

void ShiftRegReset(void) {
	sr = 0xF000;
	do_sr_display(0);
}
