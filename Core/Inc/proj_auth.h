/*
 * proj_auth.h
 *
 *  Created on: 6 Aug 2026
 *      Author: matth
 */

#ifndef INC_PROJ_AUTH_H_
#define INC_PROJ_AUTH_H_

#include "main.h"
#include "proj_lcd.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define UID_LEN 4

extern uint8_t managerUID[UID_LEN];
extern uint8_t doorUID[UID_LEN];
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c1;

/*
 * Elechouse PN532 I2C address
 *
 * 7-bit address = 0x24
 * HAL requires shifted address
 */
#define PN532_I2C_ADDR        (0x24 << 1)

/*
 * PN532 frame constants
 */
#define PN532_PREAMBLE        0x00
#define PN532_START_CODE1     0x00
#define PN532_START_CODE2     0xFF
#define PN532_POSTAMBLE       0x00
#define PN532_HOST_TO_PN532   0xD4
#define PN532_PN532_TO_HOST   0xD5

/*
 * PN532 commands
 */
#define PN532_COMMAND_GETFIRMWAREVERSION    0x02
#define PN532_COMMAND_SAMCONFIGURATION      0x14
#define PN532_COMMAND_INLISTPASSIVETARGET   0x4A

/*
 * Driver functions
 */
bool PN532_Init(void);
bool PN532_GetFirmwareVersion(uint32_t *version);
bool PN532_SAMConfig(void);
bool PN532_ReadPassiveTarget(uint8_t *uid, uint8_t *uidLength);

/*
 * Internal driver functions
 */
bool PN532_WriteCommand(uint8_t *cmd, uint8_t length);
bool PN532_ReadResponse(uint8_t *buffer, uint8_t length);

/*
 * Application functions
 */
void Access_Check(int *dopen_pending_ptr, int *keypad_auth_ptr);
bool UID_Match(uint8_t *uid, uint8_t *stored);

#endif /* INC_PROJ_AUTH_H_ */
