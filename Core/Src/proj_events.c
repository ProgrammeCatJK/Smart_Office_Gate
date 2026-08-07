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
#include "motor.h"
#include "proj_auth.h"

extern bool scheduledOpen;
int activeEvents = 0;

// Creates a sorted doubly linked list
void EventInit(Event **head) {
	*head = NULL;
}

// Takes in the time for an event and inserts into a sorted linked list
int AddEvent(Event **head, Time time, uint32_t duration, bool startEvent) {
    // Allocate memory
    Event *newEvent = malloc(sizeof(Event));
    if (!newEvent) {
        return 0;
    }

    newEvent->start = time;
    newEvent->duration = duration;
    newEvent->startEvent = startEvent;
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
           CompareTime(curr->next->start, time) <= 0) {
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
			updateLCD = true;
			HAL_Delay(500);
			return;
		} else if (button == ADD_BUTTON) {
			Time eventTime;
			Time endTime;
			uint32_t duration;
			LCD_send_cmd(0x80);
			LCD_send_string("Add Event       ");
			LCD_send_cmd(0xC0);
			LCD_send_string("Enter date:     ");
			if (!GetDateFromKeypad(&eventTime)) {
				updateLCD = true;
			    return;
			}

			LCD_send_cmd(0x80);
			LCD_send_string("Enter time:     ");
			if (!GetTimeFromKeypad(&eventTime)) {
				updateLCD = true;
				return;
			}

			LCD_send_cmd(0x80);
			LCD_send_string("Enter duration: ");
			duration = GetDurationFromKeypad();
			if (duration == 0) {
				updateLCD = true;
			    return;
			}

			// Add in start and end
			AddEvent(head, eventTime, duration, true);
			endTime = AddSeconds(eventTime, duration);
			AddEvent(head, endTime, 0, false);

			// Display first item in list
			*curr = *head;
			updateLCD = true;
			return;
		} else if (button == DOOR_BUTTON) {
			updateLCD = true;
			authLevel = AUTH_NONE;
			authTime = HAL_GetTick();
			LCD_send_cmd(LCD_CLEAR);
			LCD_send_cmd(0x80);
			LCD_send_string("Opening door");

			Motor_SetPosition(OPEN_ForEntrance);
			doorOpen = true;
		    doorOpenTime = HAL_GetTick();
		    HAL_Delay(500);
			LCD_send_cmd(LCD_CLEAR);
			return;
		} else if (button == EXIT_BUTTON) {
			authLevel = AUTH_NONE;
			updateLCD = true;
			authTime = HAL_GetTick();
			LCD_send_cmd(LCD_CLEAR);
			LCD_send_cmd(0x80);
			LCD_send_string("Exiting menu");
			HAL_Delay(500);
			LCD_send_cmd(LCD_CLEAR);
			return;
		}
	}
}

// Get input for time
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
                } else if (index <= 4) {
                    snprintf(buffer, sizeof(buffer), "%c%c:%s",
                             digits[0], digits[1], &digits[2]);
                } else {
                    snprintf(buffer, sizeof(buffer), "%c%c:%c%c:%s", digits[0],
                    		digits[1], digits[2], digits[3], &digits[4]);
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
                HAL_Delay(500);
                return false;
            }

            return true;
        } else if (key == DEL_BUTTON) {
        	if (index > 0) {
        	        digits[--index] = '\0';

        	        LCD_send_cmd(0x80);
        	        LCD_send_string("                ");
        	        LCD_send_cmd(0x80);

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

        	        LCD_send_string(buffer);

        	        LCD_send_cmd(0xC0);
        	        LCD_send_string("# to confirm");
        	    }
        } else if (key == '*') {
        	LCD_send_cmd(0x80);
			LCD_send_string("                ");
            return false;
        }
    }
}

// Get input for date
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
        } else if (key == DEL_BUTTON) {
        	if (index > 0) {
        	        digits[--index] = '\0';
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
        } else if (key == '*') {
        	LCD_send_cmd(0x80);
			LCD_send_string("                ");
            return false;
        }
    }
}

// Get input for duration
uint32_t GetDurationFromKeypad(void) {
    char digits[7] = "";
    char buffer[16];
    int index = 0;

    while (1) {
        char key = Keypad_GetKey();

        if (key >= '0' && key <= '9') {
            if (index < 6) {
                digits[index++] = key;
                digits[index] = '\0';

                // Display as HH:MM:SS
                if (index <= 2) {
                    snprintf(buffer, sizeof(buffer), "%s", digits);
                } else if (index <= 4) {
                    snprintf(buffer, sizeof(buffer), "%c%c:%s",
                             digits[0], digits[1], &digits[2]);
                } else {
                    snprintf(buffer, sizeof(buffer), "%c%c:%c%c:%s", digits[0],
                    		digits[1], digits[2], digits[3], &digits[4]);
                }

                LCD_send_cmd(0x80);
                LCD_send_string("                ");
                LCD_send_cmd(0x80);
                LCD_send_string(buffer);

                LCD_send_cmd(0xC0);
                LCD_send_string("# to confirm");
            }
        } else if (key == CONFIRM) {
            // Need exactly HHMMSS
            if (index != 6)
                continue;

            uint32_t hours = (digits[0] - '0') * 10 + (digits[1] - '0');
            uint32_t minutes = (digits[2] - '0') * 10 + (digits[3] - '0');
            uint32_t seconds = (digits[4] - '0') * 10 + (digits[5] - '0');

            // Validate duration
            if (hours > 23 || minutes > 59 || seconds > 59) {
                LCD_send_cmd(LCD_CLEAR);
                LCD_send_cmd(0x80);
                LCD_send_string("Invalid Duration");
                HAL_Delay(500);
                continue;
            }

            return (hours * 3600UL) + (minutes * 60UL) + seconds;
        } else if (key == DEL_BUTTON) {
        	if (index > 0) {
        	        digits[--index] = '\0';

        	        LCD_send_cmd(0x80);
        	        LCD_send_string("                ");
        	        LCD_send_cmd(0x80);

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

        	        LCD_send_string(buffer);

        	        LCD_send_cmd(0xC0);
        	        LCD_send_string("# to confirm");
        	    }
        } else if (key == '*') {
            return 0;
        }
    }
}

// Display current event on the lcd
void DisplayScreen(Event *curr) {
    LCD_send_cmd(LCD_CLEAR);
    LCD_send_cmd(0x80);

    if (curr == NULL) {
        LCD_send_string("NO EVENTS");
        LCD_send_cmd(0xC0);
        LCD_send_string("A to add event");
    } else {
    	DisplayTime(curr->start);
        LCD_send_cmd(0xC0);    // Second line

        if (curr->startEvent) {
            LCD_send_string("UNLOCK START");
        } else {
            LCD_send_string("LOCK END");
        }
    }
}

// Handler for the events set
void CheckEvents(Event **head, Event **curr) {
	uint32_t elapsedSeconds = (HAL_GetTick() - startTick) / 1000;
	Time currentTime = AddSeconds(startTime, elapsedSeconds);

    while (*head != NULL && CompareTime(currentTime, (*head)->start) >= 0) {
        Event *temp = *head;

        // Perform event action
        if (temp->startEvent) {
            activeEvents++;
            if (activeEvents == 1) {
                scheduledOpen = true;
                Motor_SetPosition(OPEN_ForEntrance);
            }
        } else {
            activeEvents--;
            if (activeEvents == 0) {
                scheduledOpen = false;
                if (!doorOpen) {   // no manual open active
                    Motor_SetPosition(CLOSE);
                }
            }
        }

        // Remove event from list
        *head = temp->next;
        if (*head != NULL) {
            (*head)->prev = NULL;
        }

        // Keep current pointer valid
        *curr = *head;

        free(temp);
    }

    if (*head == NULL) {
        *curr = NULL;
    }
}

// Converts HHMMSS into seconds
uint32_t HMSToSeconds(Time time) {
    return (time.hour * 3600UL) + (time.minute * 60UL) + time.second;
}

// Converts seconds into HHMMSS format
Time SecondsToHMS(uint32_t seconds) {
    Time time = startTime;

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

    snprintf(buffer, sizeof(buffer), "%02d/%02d %02d:%02d:%02d", time.day,
    		time.month, time.hour, time.minute, time.second);

    LCD_send_string(buffer);
}

// Compares two given times to see which one comes first
int CompareTime(Time a, Time b) {
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

// Initialiser for the start date/time
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

	HAL_Delay(500);
	LCD_send_cmd(LCD_CLEAR);
}

// Calculates when the next event is
Time AddSeconds(Time start, uint32_t seconds) {
    Time end = start;

    /* Jan starts at index 1 */
    uint8_t daysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    // Add seconds
    end.second += seconds % 60;
    seconds /= 60;

    // Carry seconds -> minutes
    if (end.second >= 60) {
        end.second -= 60;
        seconds++;
    }

    // Add minutes
    end.minute += seconds % 60;
    seconds /= 60;

    // Carry minutes -> hours
    if (end.minute >= 60) {
        end.minute -= 60;
        seconds++;
    }

    // Add hours
    end.hour += seconds % 24;
    seconds /= 24;

    // Carry hours -> days
    if (end.hour >= 24) {
        end.hour -= 24;
        seconds++;
    }

    // Add remaining days
    while (seconds > 0) {
        end.day++;
        seconds--;

        if (end.day > daysInMonth[end.month]) {
            end.day = 1;
            end.month++;
            if (end.month > 12) {
                end.month = 1;
            }
        }
    }
    return end;
}
