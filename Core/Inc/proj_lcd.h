/*
 * proj_lcd.h
 *
 *  Created on: 3 Aug 2026
 *      Author: matth
 */

#ifndef INC_PROJ_LCD_H_
#define INC_PROJ_LCD_H_

#define LCD_CLEAR 0b00000001
#define LCD_NEWLINE 0xC0
#define DEBOUNCE_MS 50

#include "main.h"
#include <stdio.h>
#include "stdbool.h"

extern bool updateLCD;

void LCD_put_nibble(uint8_t byte);

void LCD_pulse();

void LCD_send_byte(uint8_t c, int rs);

void LCD_send_cmd(uint8_t cmd);

void LCD_send_data(uint8_t data);

void LCD_init();

void LCD_send_string(char *s);

void LCD_DisplayTime(uint32_t time);

#endif /* INC_PROJ_LCD_H_ */
