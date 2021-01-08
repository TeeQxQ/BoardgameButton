#include "messages.h"
#include "colors.h"

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
  BTN_LONG
};

typedef struct Event
{
  EventType type;
  int data;
} Event;

//Arrays to buffer events to be send/received
//There is slot for every defined color
Event receivedEvents[nofColors] = { UNKNOWN };
Event outgoingEvents[nofColors] = { UNKNOWN };

//--------------------Functions--------------------

//Common function for sending message to the client/master button
template <typename ClientType>
void sendMsgCommon(ClientType client, const int color, EventType event, const int data = 0)
{
  //Construct a message
  msg::msg["color"] = color;
  msg::msg["event"] = static_cast<int>(event);
  msg::msg["data"] = data;

  //Serialize the message and send it
  serializeJson(msg::msg, client);
}

//Common function for sending an event to the <client> client
template <typename ClientType>
void sendEventCommon(ClientType client, const Color color, Event event)
{
  sendMsgCommon(client, static_cast<int>(color), event.type, event.data);
}

//Function for receiving messages from a client/server button
template <typename ClientType>
bool receiveMsgCommon(ClientType client)
{
  msg::err = deserializeJson(msg::msg, client);
  if (msg::err)
  {
    //Serial.print(F("deserializeJson() failed: "));
    //Serial.println(err.f_str());
    return -1;
  }

  return 0;
}

template <typename ClientType>
Event receiveEventCommon(ClientType client)
{
  Event e;
  e.type = UNKNOWN;
  e.data = 0;
  
  if (!receiveMsgCommon(client))
  {
    e.type = msg::msg["event"];
    e.data = msg::msg["data"];
  }

  return e;
}

//Clear the outgoing buffer
void clearOutgoingEvents()
{
  for(size_t color = 0; color < nofColors; color++)
  {
    Event *e = &outgoingEvents[static_cast<Color>(color)];
    e->type = UNKNOWN;
    e->data = 0;
  }
}

//Clear the receiving buffer
void clearReceivedEvents()
{
  for(size_t color = 0; color < nofColors; color++)
  {
    Event *e = &receivedEvents[static_cast<Color>(color)];
    e->type = UNKNOWN;
    e->data = 0;
  }
}
