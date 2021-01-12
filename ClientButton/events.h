//This file defines different IDs for different events

enum EventType {
  UNKNOWN,
  LED,
  LED_ON,
  LED_OFF,
  BLINK,
  BLINK_ON,
  BLINK_OFF,
  BTN_SHORT,
  BTN_LONG,
  COLOR
};

typedef struct Event
{
  EventType type;
  int data;
} Event;
