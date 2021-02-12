#ifndef Event_h
#define Event_h

enum EventType {
  UNKNOWN = 0,
  LED,
  LED_ON,
  LED_OFF,
  BLINK,
  BLINK_ON = 5,
  BLINK_OFF = 6,
  BTN_SHORT,
  BTN_LONG,
  COLOR
};

typedef struct Event
{
  EventType type;
  int data;
} Event;

#endif
