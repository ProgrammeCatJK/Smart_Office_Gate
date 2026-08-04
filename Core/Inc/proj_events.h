/*
 * proj_scheduler.h
 *
 *  Created on: 3 Aug 2026
 *      Author: matth
 */

#ifndef INC_PROJ_EVENTS_H_
#define INC_PROJ_EVENTS_H_

#include <stdlib.h>
#include <stdint.h>
#include "stdbool.h"

typedef struct Event
{
    uint32_t time;
    int state;

    struct Event *next;
    struct Event *prev;

} Event;

typedef enum
{
    LCD_SCREEN_VIEW,
    LCD_SCREEN_NO_EVENTS,
    LCD_SCREEN_ADD,
    LCD_SCREEN_DELETE
} LCDScreen;

// Creates a sorted doubly linked list
void EventInit(Event **head);

// Takes in the time for an event and inserts into a sorted linked list
int AddEvent(Event **head, uint32_t time, int state);

// Deletes the event currently
void DeleteEvent(Event **head, Event **curr);

// Reads input from keypad and changes the list up/down
void ReadKeypad(Event **head, Event **curr);

bool GetTimeFromKeypad(uint32_t *time);
int GetStateFromKeypad();

void DisplayScreen(Event *curr);
void CheckEvents(Event **head, Event **curr);

#endif /* INC_PROJ_EVENTS_H_ */
