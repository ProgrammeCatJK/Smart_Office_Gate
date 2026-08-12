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
#include <stdbool.h>

typedef struct {
	uint8_t month;
	uint8_t day;
	uint8_t hour;
    uint8_t minute;
    uint8_t second;
} Time;

typedef struct Event {
    Time start;
    uint32_t duration;
    bool startEvent;

    struct Event *next;
    struct Event *prev;
} Event;

#define PREV_BUTTON '1'
#define NEXT_BUTTON '3'
#define ADD_BUTTON 'A'
#define DOOR_BUTTON 'B'
#define EXIT_BUTTON 'C'
#define DEL_BUTTON 'D'
#define CONFIRM '#'
#define BACK_BUTTON '*'

#define LCD_ROW_1 0x80
#define LCD_ROW_2 0xC0
#define LCD_LINE_LENGTH 16
#define LCD_CLEAR_LINE "                "

#define EVENT_START true
#define EVENT_END false

#define TIME_DIGITS_LENGTH        6
#define DATE_DIGITS_LENGTH        4

#define MAX_HOUR                  23
#define MAX_MINUTE                59
#define MAX_SECOND                59

#define MAX_DAY                   31
#define MAX_MONTH                 12


static Time startTime = {1,1,0,0,0};
static uint32_t startTick = 0;
extern int activeEvents;
extern uint32_t authTime;
extern bool doorOpen;
extern uint32_t doorOpenTime;

// Creates a sorted doubly linked list
void EventInit(Event **head);

// Takes in the time for an event and inserts into a sorted linked list
int AddEvent(Event **head, Time time, uint32_t duration, bool startEvent);

// Deletes the event currently
void DeleteEvent(Event **head, Event **curr);

// Reads input from keypad and changes the list up/down
void ReadKeypad(Event **head, Event **curr);

// Reads input from keypad for date, time and duration
bool GetTimeFromKeypad(Time *time);
bool GetDateFromKeypad(Time *time);
uint32_t GetDurationFromKeypad(void);

// Display current event on the lcd
void DisplayScreen(Event *curr);

// Handler for the events set
void CheckEvents(Event **head, Event **curr);

// Conversion
uint32_t HMSToSeconds(Time time);
Time SecondsToHMS(uint32_t seconds);

// Validation
bool IsValidTime(Time time);

// Display
void DisplayTime(Time time);
int CompareTime(Time a, Time b);

// Initialiser for the start date/time
void SetStartTime(void);

// Calculates when the next event is
Time AddSeconds(Time start, uint32_t seconds);

#endif /* INC_PROJ_EVENTS_H_ */
