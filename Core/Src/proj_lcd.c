/*
 * proj_lcd.c
 *
 *  Created on: 3 Aug 2026
 *      Author: matth
 */


#include "lcd.h"

// Write less significant nibble in `byte` to LCD
void LCD_put_nibble(uint8_t byte) {
// Write most significant bit first
//    HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, (byte & 0x8) ? 1 : 0);
//    HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, (byte & 0x4) ? 1 : 0);
//    HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, (byte & 0x2) ? 1 : 0);
//    HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, (byte & 0x1) ? 1 : 0);

    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (byte & 0x1) ? 1 : 0);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (byte & 0x2) ? 1 : 0);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (byte & 0x4) ? 1 : 0);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (byte & 0x8) ? 1 : 0);
}

void LCD_pulse() {
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, 1);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, 0);
    // Falling edge is the one that is detected
    HAL_Delay(1);
}

void LCD_send_byte(uint8_t c, int rs) {
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rs); // rs = 1
    LCD_put_nibble(c >> 4);
    LCD_pulse();

    LCD_put_nibble(c & 0x0F);
    LCD_pulse();
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, 0);
}

void LCD_send_cmd(uint8_t cmd) {
    LCD_send_byte(cmd, 0);
}

void LCD_send_data(uint8_t data) {
    LCD_send_byte(data, 1);
}

void LCD_init() {
    HAL_Delay(50);
    HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, 0); // Write only
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, 0); // Sending command

    LCD_put_nibble(0x3);
    LCD_pulse();
    HAL_Delay(5);

    LCD_put_nibble(0x3);
    LCD_pulse();
    HAL_Delay(5);

    LCD_put_nibble(0x3);
    LCD_pulse();
    HAL_Delay(1);

    LCD_put_nibble(0x2);   // switch to 4-bit mode
    LCD_pulse();
    HAL_Delay(1);

    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, 0);

    // Now we have a 4-bit interface
    LCD_send_cmd(0x28); // Specify number of display lines and character font
    LCD_send_cmd(0x08); // Display off
    LCD_send_cmd(LCD_CLEAR); // Display clear
    HAL_Delay(10);
    LCD_send_cmd(0x06); // Entry mode set: move cursor right and don't shift display
    LCD_send_cmd(0x0C); // Display on
}

void LCD_send_string(char *s) {
    while(*s) {
        LCD_send_data(*s++);
    }
}

void LCD_DisplayTime(uint32_t time) {
    char buffer[16];

    uint32_t seconds = time / 1000;
    uint32_t ms = time % 1000;

//    LCD_send_cmd(0x80);          // first line

    LCD_send_string("Time: ");

    snprintf(buffer, sizeof(buffer), "%lu.%03lu s  ", seconds, ms);

    LCD_send_string(buffer);
}
