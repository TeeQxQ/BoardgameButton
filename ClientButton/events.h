//This file defines different IDs for different events

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
  BLINK_ALL = 15
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


String eventToString(EventType e)
{
  String s = "";
  switch(static_cast<int>(e))
  {
    case UNKNOWN:
      s = "UNKNOWN";
      break;
    case LED:
      s = "LED";
      break;
    case LED_ON:
      s = "LED_ON";
      break;
    case LED_OFF:
      s = "LED_OFF";
      break;
    case BLINK:
      s = "BLINK";
      break;
    case BLINK_ON:
      s = "BLINK_ON";
      break;
    case BLINK_OFF:
      s = "BLINK_OFF";
      break;
    case BTN:
      s = "BTN";
      break;
    case BTN_DOUBLE:
      s = "BTN_DOUBLE";
      break;
    case BTN_SHORT:
      s = "BTN_SHORT";
      break;
    case BTN_LONG:
      s = "BTN_LONG";
      break;
    case COLOR:
      s = "COLOR";
      break;
    default:
      break;
  }

  return s;
}
