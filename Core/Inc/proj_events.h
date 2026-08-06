/*
 * proj_events.h
 *
 *  Created on: 5 Aug 2026
 *      Author: matth
 */

#ifndef INC_PROJ_EVENTS_H_
#define INC_PROJ_EVENTS_H_

#include <stdlib.h>
#include <stdint.h>
#include "stdbool.h"

typedef struct
{
	uint8_t month;
	uint8_t day;
	uint8_t hour;
    uint8_t minute;
    uint8_t second;
} Time;

typedef struct Event
{
    Time start;
    int state;

    struct Event *next;
    struct Event *prev;

} Event;

// Creates a sorted doubly linked list
void EventInit(Event **head);

// Takes in the time for an event and inserts into a sorted linked list
int AddEvent(Event **head, Time time, int state);

// Deletes the event currently
void DeleteEvent(Event **head, Event **curr);

// Reads input from keypad and changes the list up/down
void ReadKeypad(Event **head, Event **curr);

bool GetTimeFromKeypad(Time *time);

bool GetDateFromKeypad(Time *time);

int GetStateFromKeypad();

void DisplayScreen(Event *curr);
void CheckEvents(Event **head, Event **curr);

// Conversion
uint32_t HMSToSeconds(Time time);
Time SecondsToHMS(uint32_t seconds);

// Validation
bool IsValidTime(Time time);

// Display
void DisplayTime(Time time);
int CompareTime(Time a, Time b);

void SetStartTime(void);

#endif /* INC_PROJ_EVENTS_H_ */
