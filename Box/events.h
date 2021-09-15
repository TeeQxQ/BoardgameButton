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
  BTN_DOUBLE = 8,
  BTN_SHORT = 9,
  BTN_LONG = 10,
  COLOR = 11,
  ADDED = 12,
  REMOVED = 13,
  ALL_OFF_EXCEPT_ONE = 14,
  BLINK_ALL = 15,
  BTN_LONG_LONG
};

typedef struct Event
{
  EventType type;
  int data;
  unsigned long ts;

  Event()
  {
    this->type = UNKNOWN;
    this->data = 0;
    this->ts = 0;
  }

  Event(EventType type, int data = 0, unsigned long ts = 0)
  {
    this->type = type;
    this->data = data;
    this->ts = ts;
  }
} Event;

#endif
