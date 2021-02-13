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
  COLOR,
  ADDED,
  REMOVED
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
