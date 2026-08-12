/*
 * proj_auth.c
 *
 *  Created on: 6 Aug 2026
 *      Author: matth
 */

#include "proj_auth.h"
#include <string.h>

extern uint32_t authTime;

uint8_t managerUID[UID_LEN] = { 0x93, 0x18, 0x26, 0x07 };
uint8_t doorUID[UID_LEN] = { 0x8E, 0xF8, 0x09, 0x07 };

/*
 * Send command frame to PN532
 */
bool PN532_WriteCommand(uint8_t *cmd, uint8_t length) {
    uint8_t frame[64];
    uint8_t checksum = PN532_HOST_TO_PN532;

    for (uint8_t i = 0; i < length; i++) {
        checksum += cmd[i];
    }

    uint8_t frameLength = length + 1;

    frame[0] = PN532_PREAMBLE;
    frame[1] = PN532_START_CODE1;
    frame[2] = PN532_START_CODE2;
    frame[3] = frameLength;
    frame[4] = (~frameLength) + 1;
    frame[5] = PN532_HOST_TO_PN532;

    for (uint8_t i = 0; i < length; i++) {
        frame[6 + i] = cmd[i];
    }

    frame[6 + length] = (~checksum) + 1;
    frame[7 + length] = PN532_POSTAMBLE;

    return HAL_I2C_Master_Transmit(&hi2c1, PN532_I2C_ADDR, frame, length + 8, 100) == HAL_OK;
}

/*
 * Read response
 *
 * Handles:
 *
 * status byte
 * ACK frame
 * response frame
 */
bool PN532_ReadResponse(uint8_t *buffer, uint8_t length) {
    uint8_t status;
    uint8_t ack[6];
    uint32_t timeout = HAL_GetTick() + 200;

    /*
     * Wait for PN532 ready
     */
    do {
        HAL_I2C_Master_Receive(&hi2c1, PN532_I2C_ADDR, &status, 1, 100);

        if (HAL_GetTick() > timeout)
            return false;
    } while (status != 0x01);

    /*
     * Clear ACK frame
     */
    HAL_I2C_Master_Receive(&hi2c1, PN532_I2C_ADDR, ack, 6, 100);

    /*
     * Wait for response
     */
    timeout = HAL_GetTick() + 200;

    do {
        HAL_I2C_Master_Receive(&hi2c1, PN532_I2C_ADDR, &status, 1, 100);

        if(HAL_GetTick() > timeout)
            return false;
    } while (status != 0x01);

    /*
     * Read actual response
     */
    if (HAL_I2C_Master_Receive(&hi2c1, PN532_I2C_ADDR, buffer, length, 100) != HAL_OK) {
        return false;
    }

    return true;
}


bool PN532_Init(void) {
    uint32_t version;

    HAL_Delay(100);

    if (!PN532_GetFirmwareVersion(&version)) {
        return false;
    }

    if (!PN532_SAMConfig()) {
        return false;
    }

    return true;
}

bool PN532_GetFirmwareVersion(uint32_t *version) {
    uint8_t command = PN532_COMMAND_GETFIRMWAREVERSION;
    uint8_t response[16];

    if (!PN532_WriteCommand(&command, 1))
        return false;

    HAL_Delay(10);

    if(!PN532_ReadResponse(response, sizeof(response)))
        return false;

    /*
     * Response:
     *
     * D5 03 IC VER REV SUPPORT
     */
    if (response[6] != 0xD5)
        return false;

    *version =
        ((uint32_t)response[8] << 24) |
        ((uint32_t)response[9] << 16) |
        ((uint32_t)response[10] << 8) |
        response[11];

    return true;
}

bool PN532_SAMConfig(void) {
    uint8_t command[] = { PN532_COMMAND_SAMCONFIGURATION, 0x01, 0x14, 0x01 };
    uint8_t response[16];

    if (!PN532_WriteCommand(command, sizeof(command)))
        return false;

    HAL_Delay(10);

    return PN532_ReadResponse(response, sizeof(response));
}

bool PN532_ReadPassiveTarget(uint8_t *uid, uint8_t *uidLength) {
    uint8_t command[] = { PN532_COMMAND_INLISTPASSIVETARGET, 0x01, 0x00 };
    uint8_t response[32];

    if (!PN532_WriteCommand(command, sizeof(command)))
        return false;

    HAL_Delay(10);

    if (!PN532_ReadResponse(response, sizeof(response)))
        return false;

    /*
     * Response:
     *
     * D5 4B NbTg Tg UIDLen UID...
     */
    if (response[6] != 0xD5)
        return false;

    if (response[8] != 1)
        return false;

    *uidLength = response[13];
    memcpy(uid, &response[14], *uidLength);

    return true;
}

bool UID_Match(uint8_t *uid, uint8_t *stored) {
    for (uint8_t i = 0; i < UID_LEN; i++) {
        if (uid[i] != stored[i])
            return false;
    }

    return true;
}

void Access_Check(int *dopen_pending_ptr, int *keypad_auth_ptr) {
    uint8_t uid[7];
    uint8_t uidLength = 0;

    if (!PN532_ReadPassiveTarget(uid, &uidLength)) {
        return;
    }

    printf("UID: ");

    for (uint8_t i = 0; i < uidLength; i++) {
        printf("%02X ", uid[i]);
    }

    printf("\r\n");

    if (uidLength != UID_LEN) {
        printf("Unknown card\r\n");
        return;
    }

    if (UID_Match(uid, managerUID)) {
        printf("MANAGER CARD\r\n");

        authTime = HAL_GetTick();

        *keypad_auth_ptr = 1;

        return;
    }

    if (UID_Match(uid, doorUID)) {
        printf("DOOR CARD\r\n");

        authTime = HAL_GetTick();

        *dopen_pending_ptr = 1;

        return;
    }

    printf("DENIED\r\n");
    return;
}
