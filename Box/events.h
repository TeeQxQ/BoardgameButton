//This file defines different IDs for different events

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

String eventToString(Event e)
{
  String s = "";
  switch(static_cast<int>(e.type))
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
      s = "BLINK_ON";
      break;
    case BLINK_ON:
      s = "BLINK_ON";
      break;
    case BLINK_OFF:
      s = "BLINK_OFF";
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
      s = "DEFAULT";
      break;
  }

  return s;
}
