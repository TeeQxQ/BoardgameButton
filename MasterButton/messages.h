#include "ArduinoJson.h"

/* This is the definition (as JSON) for the message.
 *  If this definition is changed later, the following tool
 *  can be used to calculate new value for msgCapacity
 *  https://arduinojson.org/v6/assistant/
 *  
 *  {
 *   "color": 0,
 *   "event": 2,
 *   "data": 100
 *  }
 *  
 */

namespace msg
{

const int msgCapacity = 48;
StaticJsonDocument<msgCapacity> msg;
DeserializationError err;

}
//------------------------------------------------

//Old messages:
const String msg_led_on = "LED_ON";
const String msg_led_off = "LED_OFF";
const String msg_led_blink_on = "LED_BLINKING";
const String msg_led_blink_off = "LED_BLINKING_OFF";
const String msg_btn_pressed_short = "BTN_SHORT";
const String msg_btn_pressed_long = "BTN_LONG";
const String msg_ask_color = "GET_COLOR";      //kysy varia
const String msg_color_green = "COLOR_GREEN";   //olen vihrea nappi
