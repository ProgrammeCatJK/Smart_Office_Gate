/*
 * scheduler.c
 *
 *  Created on: 3 Aug 2026
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


// Creates a sorted doubly linked list
void EventInit(Event **head) {
	*head = NULL;
}

// Takes in the time for an event and inserts into a sorted linked list
int AddEvent(Event **head, uint32_t time, int state) {
	// Allocate memory
	Event *newEvent = malloc(sizeof(Event));
	if (!newEvent) {
		return 0;
	}
	newEvent->time = time;
	newEvent->state = state;
	newEvent->next = NULL;
	newEvent->prev = NULL;

	// Insert in sorted order
	// If empty list
	if (*head == NULL) {
		*head = newEvent;
		return 1;
	}

	Event *curr = *head;

	// Insert before head
	if (time < curr->time) {
		newEvent->next = curr;
		curr->prev = newEvent;
		*head = newEvent;
		return 1;
	}

	// Insert after curr
	while(curr->next && curr->next->time < time) {
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
			LCD_send_cmd(LCD_CLEAR);
			LCD_send_cmd(0x80);
			LCD_send_string("Add Event");
			LCD_send_cmd(0xC0);
			LCD_send_string("Enter time:");

			uint32_t eventTime;
			int state;
			if (GetTimeFromKeypad(&eventTime) && (state = GetStateFromKeypad()) >= 0) {
				eventTime = HAL_GetTick() + (eventTime * 1000);
				AddEvent(head, eventTime, state);
//				DisplayScreen(*head);
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

bool GetTimeFromKeypad(uint32_t *time) {
	uint32_t number = 0;
	char numbuffer[16] = "";
	while (1) {
		char key = Keypad_GetKey();
		if (key >= '0' && key <= '9') {
		  number = number * 10 + (key - '0');

		  // send number to lcd
		  snprintf(numbuffer, sizeof(numbuffer), "Time: %lu", number);
		  // UX stuff
		  LCD_send_cmd(0x80);
		  LCD_send_string("                ");
		  LCD_send_cmd(0x80);
		  LCD_send_string(numbuffer);
		  LCD_send_cmd(0xC0);
		  LCD_send_string("# to confirm");
		} else if (key == CONFIRM) {
			LCD_send_cmd(0x80);
		    LCD_send_string("                ");
			LCD_send_cmd(0x80);
			LCD_send_string(numbuffer);
			*time = number;

			return true;
		} else if (key == '*') {
			return false;
		}
	}
	return true;
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

void DisplayScreen(Event *curr)
{
    char buffer[17];

    LCD_send_cmd(LCD_CLEAR);
    LCD_send_cmd(0x80);

    if (curr == NULL) {
        LCD_send_string("NO EVENTS");
        LCD_send_cmd(0xC0);
        LCD_send_string("A to add event");
    } else {
        snprintf(buffer, sizeof(buffer), "Time:%lu", curr->time);
        LCD_send_string(buffer);

        LCD_send_cmd(0xC0);    // Second line

        if (curr->state == LOCKED) {
            LCD_send_string("LOCK DOOR");
        }
        else {
            LCD_send_string("UNLOCK DOOR");
        }
    }
}

void CheckEvents(Event **head, Event **curr) {
    uint32_t currentTime = HAL_GetTick();

    while (*head != NULL && (*head)->time <= currentTime) {
        Event *temp = *head;

        // Perform event action
        if (temp->state == LOCKED) {
            //Lock Door
        } else {
            //Unlock Door
        }

        // If the event being deleted is currently selected
        if (*curr == temp) {
            *curr = temp->next;
        }

        // Remove from linked list
        *head = temp->next;

        if (*head != NULL) {
            (*head)->prev = NULL;
        }

        free(temp);
    }

    // If there are no events left, ensure curr is NULL
    if (*head == NULL) {
        *curr = NULL;
    }
}
