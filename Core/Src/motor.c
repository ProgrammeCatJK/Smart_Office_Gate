#include "motor.h"
#include "main.h"

// Target is a
static int16_t CurrentTick = 0;
int16_t Target = 0;

void Motor_Stop() {
    Target = CurrentTick;
}

void Motor_Init() {
	HAL_GPIO_WritePin(COILA_GPIO_Port, COILA_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(COILB_GPIO_Port, COILB_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(COILC_GPIO_Port, COILC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(COILD_GPIO_Port, COILD_Pin, GPIO_PIN_RESET);
}

void Motor_Reset() {
	Target = 0;
	CurrentTick = 0;
}

void Motor_ManualSetLocation() {
	if (HAL_GPIO_ReadPin(GPIOB, SW3_Pin) == 1) {
		HAL_Delay(12);
		Motor_SetPosition(20);
		HAL_Delay(200);
		Motor_Reset();
	}


	if (HAL_GPIO_ReadPin(GPIOC, SW4_Pin) == 1) {
		HAL_Delay(12);
		Motor_SetPosition(-20);
		HAL_Delay(200);
		Motor_Reset();
	}

}

void Motor_SetPosition(int16_t position) {
	Target = position;
}

/*
 * 1. Takes in Target & CurrentTick to decide the location and rotation
 * 2. Stopping condition: CurrentTick == Target;
 * 3. Rotate Anti-Clockwise: Target > CurrentTick
 * 4. Rotate Clockwise: Target < CurrentTick
 * 5. CurrentTick gets updated for every iteration
 * */

void Motor_MoveToTarget() {
	// function takes in Target and Current to decide the location

	if (CurrentTick == Target) return;
	// Target > CurrentTick --> AntiClockwise Rotation
	// Update CurrentTick for each loop

	if (Target > CurrentTick) {
		CurrentTick++;

		int step = CurrentTick % 4;
		if (step < 0) step += 4;

		if (step == 0) {
			HAL_GPIO_WritePin(COILB_GPIO_Port, COILB_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(COILA_GPIO_Port, COILA_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILD_GPIO_Port, COILD_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILC_GPIO_Port, COILC_Pin, GPIO_PIN_RESET);
		} else if (step == 1) {
			HAL_GPIO_WritePin(COILB_GPIO_Port, COILB_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILA_GPIO_Port, COILA_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(COILD_GPIO_Port, COILD_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILC_GPIO_Port, COILC_Pin, GPIO_PIN_RESET);
		} else if (step == 2) {
			HAL_GPIO_WritePin(COILB_GPIO_Port, COILB_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILA_GPIO_Port, COILA_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILD_GPIO_Port, COILD_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(COILC_GPIO_Port, COILC_Pin, GPIO_PIN_RESET);
		} else if (step == 3) {
			HAL_GPIO_WritePin(COILB_GPIO_Port, COILB_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILA_GPIO_Port, COILA_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILD_GPIO_Port, COILD_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILC_GPIO_Port, COILC_Pin, GPIO_PIN_SET);
		}
	}


		// Clockwise Direction
	else if (Target < CurrentTick) {
		CurrentTick--;

		int step = CurrentTick % 4;
		if (step < 0) step += 4;

		if (step == 3) {
			HAL_GPIO_WritePin(COILC_GPIO_Port, COILC_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(COILD_GPIO_Port, COILD_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILA_GPIO_Port, COILA_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILB_GPIO_Port, COILB_Pin, GPIO_PIN_RESET);
		} else if (step == 2) {
			HAL_GPIO_WritePin(COILC_GPIO_Port, COILC_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILD_GPIO_Port, COILD_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(COILA_GPIO_Port, COILA_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILB_GPIO_Port, COILB_Pin, GPIO_PIN_RESET);
		} else if (step == 1) {
			HAL_GPIO_WritePin(COILC_GPIO_Port, COILC_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILD_GPIO_Port, COILD_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILA_GPIO_Port, COILA_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(COILB_GPIO_Port, COILB_Pin, GPIO_PIN_RESET);
		} else if (step == 0) {
			HAL_GPIO_WritePin(COILC_GPIO_Port, COILC_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILD_GPIO_Port, COILD_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILA_GPIO_Port, COILA_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COILB_GPIO_Port, COILB_Pin, GPIO_PIN_SET);
		}
	}

}

int DoorIsClosed(void) {
	return CurrentTick == CLOSE;
}
