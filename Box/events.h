#ifndef Event_h
#define Event_h

enum EventType {
  UNKNOWN = 0,
  LED = 1,
  LED_ON = 2,
  LED_OFF = 3,
  BLINK = 4,
  BLINK_ON = 5,
  BLINK_OFF = 6,
  BTN = 7, 
  BTN_SHORT = 8,
  BTN_LONG = 9,
  COLOR = 10,
  ADDED = 11,
  REMOVED = 12,
  ALL_OFF_EXCEPT_ONE = 13,
  BLINK_ALL = 14
};

typedef struct Event
{
  EventType type;
  int data;

  Event()
  {
    this->type = UNKNOWN;
    this->data = 0;
  }

  Event(EventType type, int data = 0)
  {
    this->type = type;
    this->data = data;
  }
} Event;

#endif
