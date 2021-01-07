#include "ArduinoJson.h"

/* This is the definition (as JSON) for the message.
 *  If this definition is changed later, the following tool
 *  can be used to calculate new value for msgCapacity
 *  https://arduinojson.org/v6/assistant/
 *  
 *  {
 *   "color": 0,
 *   "event": 2,
 *   "duration": 100
 *  }
 *  
 */

const int msgCapacity = 48;
StaticJsonDocument<msgCapacity> msg;
DeserializationError err;

//Common function for sending events to the client/master button
template <typename ClientType>
void sendMsgCommon(ClientType client, const int color, const int event, const int duration = 0)
{
  //Construct a message
  msg["color"] = color;
  msg["event"] = event;
  msg["duration"] = duration;

  //Serialize the message and send it
  serializeJson(msg, client);
}

//Function for receiving messages from a client/server button
template <typename ClientType>
bool receiveMsgCommon(ClientType client)
{
  err = deserializeJson(msg, client);
  if (err)
  {
    //Serial.print(F("deserializeJson() failed: "));
    //Serial.println(err.f_str());
    return -1;
  }

  return 0;
}

template <typename ClientType>
int receiveEventCommon(ClientType client)
{
  if (receiveMsgCommon(client) != 0)
  {
    //On error
    return -1;
  }

  return msg["event"];
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
