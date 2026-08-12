#ifndef MOTOR_H_
#define MOTOR_H_

#include "main.h"

#define CLOSE 0
#define OPEN_HOLD -512 // Door should hold at the exit side.
#define OPEN_ForEntrance 512
#define OPEN_ForExit -512

void Motor_Init();
void Motor_Reset();
void Motor_MoveToTarget();

/*
 * Usage:
 * 1. Enter from Entrance: Motor_SetPosition(OPEN_ForEntrance);
 * 2. Exit from Exit: Motor_SetPosition(OPEN_ForExit);
 * 3. Stop Motor for current state: Motor_Stop();
 * 4. Resume Motor after stop: Motor_SetPosition(OPEN_ForEntrance | OPEN_ForExit | CLOSE | OPEN_HOLD);
 *
 * */

/*============Use in main.c===================*/
void Motor_Stop();
void Motor_ManualSetLocation();
void Motor_SetPosition(int16_t position);
int DoorIsClosed(void);
/*============================================*/
#endif /* MOTOR_H_ */
