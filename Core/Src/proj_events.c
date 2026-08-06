/*
 * proj_events.c
 *
 *  Created on: 5 Aug 2026
 *      Author: matth
 */

/*
typedef struct Event
{
    uint32_t time;
    int state;

    struct Event *next;
    struct Event *prev;

} Event;
*/

#include "proj_events.h"
#include "proj_lcd.h"
#include "proj_keypad.h"

#define PREV_BUTTON '1'
#define NEXT_BUTTON '3'
#define ADD_BUTTON 'A'
#define DOOR_BUTTON 'B'
#define EXIT_BUTTON 'C'
#define DEL_BUTTON 'D'
#define CONFIRM '#'
#define LOCKED 1
#define UNLOCKED 0

static Time startTime = {1,1,0,0,0};
static uint32_t startTick = 0;

// Creates a sorted doubly linked list
void EventInit(Event **head) {
	*head = NULL;
}

// Takes in the time for an event and inserts into a sorted linked list
int AddEvent(Event **head, Time time, int state) {
    // Allocate memory
    Event *newEvent = malloc(sizeof(Event));
    if (!newEvent) {
        return 0;
    }

    newEvent->start = time;
    newEvent->state = state;
    newEvent->next = NULL;
    newEvent->prev = NULL;

    // Empty list
    if (*head == NULL) {
        *head = newEvent;
        return 1;
    }

    Event *curr = *head;

    // Insert before head
    if (CompareTime(time, curr->start) < 0) {
        newEvent->next = curr;
        curr->prev = newEvent;
        *head = newEvent;
        return 1;
    }

    // Find insertion point
    while (curr->next &&
           CompareTime(curr->next->start, time) < 0) {
        curr = curr->next;
    }

    newEvent->next = curr->next;
    newEvent->prev = curr;

    if (curr->next) {
        curr->next->prev = newEvent;
    }

    curr->next = newEvent;

    return 1;
}

// Deletes the event currently pointed to
void DeleteEvent(Event **head, Event **curr) {
	if (*curr == NULL) {
	    return;
	}
	Event *temp = *curr;

	// Move current pointer to not delete it
	if (temp->next) {
		*curr = temp->next;
	} else {
		*curr = temp->prev;
	}

	// Fix previous node
	if (temp->prev) {
		temp->prev->next = temp->next;
	} else { // Delete first node
		*head = temp->next;
	}

	// Fix next node
	if (temp->next) {
		temp->next->prev = temp->prev;
	}

	free(temp);
}

// Reads input from keypad and changes the list up/down
void ReadKeypad(Event **head, Event **curr) {
	while (1) {
		char button = Keypad_GetKey();

//		if (*curr == NULL && button != ADD_BUTTON) {
//			return;
//		}
		if (button == NEXT_BUTTON) {
			if (*curr != NULL && (*curr)->next != NULL) {
				*curr = (*curr)->next;
				DisplayScreen(*curr);
			}
		} else if (button == PREV_BUTTON) {
			if (*curr != NULL && (*curr)->prev != NULL) {
				*curr = (*curr)->prev;
				DisplayScreen(*curr);
			}
		} else if (button == DEL_BUTTON) {
			LCD_send_cmd(LCD_CLEAR);
			LCD_send_cmd(0x80);
			LCD_send_string("Deleted Event");
			if (*curr == NULL) {
				LCD_send_cmd(LCD_CLEAR);
				LCD_send_cmd(0x80);
				LCD_send_string("No events to del");

			} else {
				DeleteEvent(head, curr);
			}
			HAL_Delay(500);
			DisplayScreen(*head);
		} else if (button == ADD_BUTTON) {
			Time eventTime;
			int state;
			LCD_send_cmd(LCD_CLEAR);
			LCD_send_cmd(0x80);
			LCD_send_string("Add Event");
			LCD_send_cmd(0xC0);
			LCD_send_string("Enter date:");
			if (!GetDateFromKeypad(&eventTime)) {
			    return;
			}

			LCD_send_cmd(LCD_CLEAR);
			LCD_send_cmd(0x80);
			LCD_send_string("Enter time:");

			if (GetTimeFromKeypad(&eventTime) && (state = GetStateFromKeypad()) >= 0) {

				// add in date/time config
				AddEvent(head, eventTime, state);

				// Display first item in list
				*curr = *head;
			} else {
				// user cancelled
				return;
			}
			// If adding the first event, select it
			if (*curr == NULL) {
				*curr = *head;
			}
			return;
		} else if (button == DOOR_BUTTON) {
			LCD_send_cmd(LCD_CLEAR);
			LCD_send_cmd(0x80);
			LCD_send_string("Opening door");
			HAL_Delay(500);
			return;
		} else if (button == EXIT_BUTTON) {
			LCD_send_cmd(LCD_CLEAR);
			LCD_send_cmd(0x80);
			LCD_send_string("Exiting menu");
			HAL_Delay(500);
			return;
			// set flag for lcd/auth off
		}
	}
	return;
}

bool GetTimeFromKeypad(Time *time) {
    char digits[7] = "";
    char buffer[16];
    int index = 0;

    while (1) {
        char key = Keypad_GetKey();

        if (key >= '0' && key <= '9') {
            if (index < 6) {
                digits[index++] = key;
                digits[index] = '\0';

                if (index <= 2) {
                    snprintf(buffer, sizeof(buffer), "%s", digits);
                }
                else if (index <= 4) {
                    snprintf(buffer, sizeof(buffer), "%c%c:%s",
                             digits[0], digits[1], &digits[2]);
                }
                else {
                    snprintf(buffer, sizeof(buffer), "%c%c:%c%c:%s",
                             digits[0], digits[1],
                             digits[2], digits[3],
                             &digits[4]);
                }

                LCD_send_cmd(0x80);
                LCD_send_string("                ");
                LCD_send_cmd(0x80);
                LCD_send_string(buffer);

                LCD_send_cmd(0xC0);
                LCD_send_string("# to confirm");
            }
        } else if (key == CONFIRM) {
            if (index != 6) {
                continue;   // Need exactly 6 digits
            }

            time->hour   = (digits[0] - '0') * 10 + (digits[1] - '0');
            time->minute = (digits[2] - '0') * 10 + (digits[3] - '0');
            time->second = (digits[4] - '0') * 10 + (digits[5] - '0');

            if (!IsValidTime(*time)) {
                LCD_send_cmd(LCD_CLEAR);
                LCD_send_cmd(0x80);
                LCD_send_string("Invalid Time");
                HAL_Delay(1000);
                return false;
            }

            return true;
        } else if (key == '*') {
            return false;
        }
    }
}

bool GetDateFromKeypad(Time *time) {
    char digits[5] = "";
    char buffer[16];
    int index = 0;

    while (1) {
        char key = Keypad_GetKey();

        if (key >= '0' && key <= '9') {
            if (index < 4) {
                digits[index++] = key;
                digits[index] = '\0';

                if (index <= 2) {
                    snprintf(buffer, sizeof(buffer), "%s", digits);
                } else {
                    snprintf(buffer, sizeof(buffer), "%c%c/%s",
                             digits[0], digits[1], &digits[2]);
                }

                LCD_send_cmd(0x80);
                LCD_send_string("                ");
                LCD_send_cmd(0x80);
                LCD_send_string(buffer);

                LCD_send_cmd(0xC0);
                LCD_send_string("# to confirm");
            }
        }
        else if (key == CONFIRM) {
            if (index != 4)
                continue;

            time->day   = (digits[0] - '0') * 10 + (digits[1] - '0');
            time->month = (digits[2] - '0') * 10 + (digits[3] - '0');

            if (time->day < 1 || time->day > 31 ||
                time->month < 1 || time->month > 12) {

                LCD_send_cmd(LCD_CLEAR);
                LCD_send_cmd(0x80);
                LCD_send_string("Invalid Date");
                HAL_Delay(500);
                return false;
            }

            return true;
        }
        else if (key == '*') {
            return false;
        }
    }
}

int GetStateFromKeypad() {
	int state = UNLOCKED;
	LCD_send_cmd(LCD_CLEAR);
	LCD_send_cmd(0x80);
	LCD_send_string("Enter state:");
	while (1) {
		char key = Keypad_GetKey();
		LCD_send_cmd(0xC0);
//		char buffer[2];
//		snprintf(buffer, sizeof(buffer), "%c", key);
//		LCD_send_string(buffer);
		if (key == '1') {
			LCD_send_cmd(0xC0);
			LCD_send_string("1: LOCK    ");
			state = LOCKED;
		} else if (key == '0') {
			LCD_send_cmd(0xC0);
			LCD_send_string("0: UNLOCK  ");
			state = UNLOCKED;
		} else if (key == CONFIRM) {
			return state;
		} else if (key == '*') {
			return -1;
		}
	}
}

void DisplayScreen(Event *curr) {
//    char buffer[17];

    LCD_send_cmd(LCD_CLEAR);
    LCD_send_cmd(0x80);

    if (curr == NULL) {
        LCD_send_string("NO EVENTS");
        LCD_send_cmd(0xC0);
        LCD_send_string("A to add event");
    } else {
    	DisplayTime(curr->start);

        LCD_send_cmd(0xC0);    // Second line

        if (curr->state == LOCKED) {
            LCD_send_string("LOCK DOOR");
        } else {
            LCD_send_string("UNLOCK DOOR");
        }
    }
}

//
void CheckEvents(Event **head, Event **curr)
{
//    Time currentTime = GetCurrentTime();
	uint32_t elapsedSeconds = (HAL_GetTick() - startTick) / 1000;

	Time currentTime = SecondsToHMS(
		HMSToSeconds(startTime) + elapsedSeconds
	);

    while (*head != NULL &&
           CompareTime(currentTime, (*head)->start) >= 0)
    {
        Event *temp = *head;

        // Perform event action
        if (temp->state == LOCKED) {
            // Lock door
        } else {
            // Unlock door
        }

        // Keep current pointer valid
        if (*curr == temp) {
            *curr = temp->next;
        }

        // Remove event from list
        *head = temp->next;

        if (*head != NULL) {
            (*head)->prev = NULL;
        }

        free(temp);
    }

    if (*head == NULL) {
        *curr = NULL;
    }
}

// Converts HHMMSS into seconds
uint32_t HMSToSeconds(Time time)
{
    return (time.hour * 3600UL) +
           (time.minute * 60UL) +
            time.second;
}

// Converts seconds into HHMMSS format
Time SecondsToHMS(uint32_t seconds)
{
    Time time;

    seconds %= 86400;

    time.hour = seconds / 3600;
    seconds %= 3600;

    time.minute = seconds / 60;
    time.second = seconds % 60;

    return time;
}

// Validates time struct input
bool IsValidTime(Time time) {
	if (time.month < 1 || time.month > 12)
	    return false;

	if (time.day < 1 || time.day > 31)
	    return false;

    if (time.hour > 23)
        return false;

    if (time.minute > 59)
        return false;

    if (time.second > 59)
        return false;

    return true;
}

// Displays time on the LCD in a readable format
void DisplayTime(Time time) {
    char buffer[20];

    snprintf(buffer,
             sizeof(buffer),
			 "%02d/%02d %02d:%02d:%02d",
			 time.day,
			 time.month,
             time.hour,
             time.minute,
             time.second);

    LCD_send_string(buffer);
}

int CompareTime(Time a, Time b)
{
    if (a.month != b.month)
        return a.month - b.month;

    if (a.day != b.day)
        return a.day - b.day;

    if (a.hour != b.hour)
        return a.hour - b.hour;

    if (a.minute != b.minute)
        return a.minute - b.minute;

    return a.second - b.second;
}

void SetStartTime(void) {
	Time inputTime;

	LCD_send_cmd(LCD_CLEAR);
	LCD_send_cmd(0x80);
	LCD_send_string("Set Date:");

	while (!GetDateFromKeypad(&inputTime));

	LCD_send_cmd(LCD_CLEAR);
	LCD_send_cmd(0x80);
	LCD_send_string("Set Time:");

	while (!GetTimeFromKeypad(&inputTime));

	startTime = inputTime;
	startTick = HAL_GetTick();

	LCD_send_cmd(LCD_CLEAR);
	LCD_send_cmd(0x80);
	LCD_send_string("Time Set");

	HAL_Delay(1000);
}

