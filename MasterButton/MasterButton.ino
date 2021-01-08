#include "ArduinoJson.h"
#include "ESP8266WiFi.h"
#include "events.h"
#include "effects.h"

#define ISR_PREFIX ICACHE_RAM_ATTR

//Hardware parameters:
const int LED_PIN = 0;
const int BTN_PIN = 14;
const Color BTN_COLOR = MASTER;

//Wifi parameters:
const char* ssid = "Kalat_ja_Rapu_2G";
const char* password = "rutaQlli";
const int WIFI_PORT = 80;
const int MAX_NOF_CLIENTS = 3;

//Wifi related variables:
WiFiServer wifiServer(WIFI_PORT);
WiFiClient *clients[MAX_NOF_CLIENTS] = { NULL };
String input_msgs[MAX_NOF_CLIENTS +1];     //Messages from myself are stored also (old)

//Blinking related parameters
const int blinkDelay_ms = 500;
bool blink_enabled = false;

//Button related parameters
volatile byte btn_pressed_counter = 0;
volatile byte btn_released_counter = 0;
int btn_state = HIGH;
volatile unsigned long btn_pressed_time_ms = 0;
volatile unsigned long btn_released_time_ms = 0;
const unsigned long btn_long_press_threshold_MIN_ms = 1000;
const unsigned long btn_long_press_threshold_MAX_ms = 2000;
const unsigned long debounceDelay_ms = 120;
volatile unsigned long lastDebounceTime_ms = 0;

//Global variables:
//volatile byte interruptCounter = 0;

int blink_state = -1;
long lastBlinkTime = 0;

//Game logic related variables:
int led_states[MAX_NOF_CLIENTS +1];
//int GREEN = -1;


//--------------------Event handling--------------------

//Set new event to be sent later to the <color>
void setEvent(Color color, Event event)
{
  outgoingEvents[color] = event;
}

//Get the latest available event from the <color>
Event getEvent(Color color)
{
  return receivedEvents[static_cast<int>(color)];
}

//Sends a single event to a single client
int sendEvent(const Color color, const Event event)
{
  //Don't send unknown events
  if (event.type == UNKNOWN) return -1;
  
  //Sends message to the master button itself
  if(color == MASTER)
  {
    //Store the event
    receivedEvents[color] = event;
    return 0;
  }
  else if(clients[color] != NULL && clients[color]->connected())
  {
    sendEventCommon(*clients[color], color, event);
    //Serial.println("Event sent.");
  }
  else
  {
    Serial.print("Client at index ");
    Serial.print(static_cast<int>(color));
    Serial.println(" is not connected!");
    return -1;
  }

  return 0;
}

//Send all buffered events to all available clients
void sendAllEvents()
{
  for (size_t color = 0; color < nofColors; color++)
  {
    if(clients[color] != NULL && clients[color]->connected())
    {
      sendEvent(static_cast<Color>(color), outgoingEvents[color]);
    }
  }
}


//Receive new event (if available) from the <color> client
Event receiveEvent(Color color)
{
  if (clients[color] != NULL && clients[color]->available())
  {
    return receiveEventCommon(*clients[static_cast<int>(color)]);
  }
}

//Receive all new events from available clients
void receiveAllEvents()
{
  for(size_t color = 0; color < nofColors; color++)
  {
    receivedEvents[color] = receiveEvent(static_cast<Color>(color));
  }
}


//--------------------Blinking--------------------

//Blink the led <times> times
void blink(int times)
{
  for(int i = 0; i < times; i++)
  {
    delay(blinkDelay_ms);
    digitalWrite(LED_PIN, HIGH);
    delay(blinkDelay_ms);
    digitalWrite(LED_PIN, LOW);
  }
}
/*
//Handles how blinking works on this device
void handleBlinking()
{
  if(blink_enabled)
  {
    if(millis() - lastBlinkTime > blinkDelay_ms)
    {
      blink_state = -blink_state;
      if(blink_state > 0)
      {
        digitalWrite(LED_PIN, HIGH);
      }
      else
      {
        digitalWrite(LED_PIN, LOW);
      }
      lastBlinkTime = millis();
    }
  }
}
*/
//--------------------Interrupts--------------------

//Handles interrupt caused by the button press
ISR_PREFIX void handleInterrupt()
{
  unsigned long currentTime_ms = millis();
  
  if (currentTime_ms - lastDebounceTime_ms > debounceDelay_ms)
  {
    if (digitalRead(BTN_PIN) == HIGH)
    {
      btn_released_counter++;
      btn_released_time_ms = currentTime_ms;
    }
    else
    {
      btn_pressed_counter++;
      btn_pressed_time_ms = currentTime_ms;
    }
    lastDebounceTime_ms = currentTime_ms;
  }
  
}

//--------------------Buttons--------------------

//Handles button press
void handleButtonPress()
{
  if (btn_released_counter > 0)
  {
    if (btn_released_time_ms - btn_pressed_time_ms > btn_long_press_threshold_MIN_ms &&
        btn_released_time_ms - btn_pressed_time_ms < btn_long_press_threshold_MAX_ms)
    {
      input_msgs[MAX_NOF_CLIENTS] = msg_btn_pressed_long;
      Serial.println("Long btn press");
    }
    else if (btn_released_time_ms - btn_pressed_time_ms <= btn_long_press_threshold_MIN_ms)
    {
      input_msgs[MAX_NOF_CLIENTS] = msg_btn_pressed_short;
      Serial.println("Short btn press");
    }
    btn_released_counter = 0;
    btn_pressed_counter = 0;
  }
  else if (btn_pressed_counter > 0)
  {
    if (millis() - btn_pressed_time_ms > btn_long_press_threshold_MAX_ms)
    {
      input_msgs[MAX_NOF_CLIENTS] = msg_btn_pressed_short;
      Serial.println("Short btn press without release event");
      btn_pressed_counter = 0;
    }
    
  }
}



void handleMessage(const String msg)
{
  if (msg == msg_led_on) digitalWrite(LED_PIN, HIGH);
  if (msg == msg_led_off) digitalWrite(LED_PIN, LOW);
}


//--------------------Connections--------------------

//Check new clients 
//If new clients are available add them to the client list
int checkNewClients()
{
  WiFiClient newClient = wifiServer.available();
  if (newClient)
  {
    //New client found
    //Add it to the list if there is space left
    for (size_t i = 0; i < MAX_NOF_CLIENTS; i++)
    {
      if (clients[i] == NULL)
      {
        clients[i] = new WiFiClient(newClient);
        Serial.println("New client added successfully");
        return 0;
      }
    }
    Serial.println("New client available, but no space left on the client list.");
    return -1;
  }
  return 0;
}

//Check if some clients have disconnected
//Remove them from the list
void removeDisconnectedClients()
{
  for (size_t i = 0; i < MAX_NOF_CLIENTS; i++)
  {
    if (clients[i] != NULL && !clients[i]->connected())
    {
      clients[i]->stop();
      delete clients[i];  //Removes memory allocation
      clients[i] = NULL;
      Serial.print("Client at index: ");
      Serial.print(i);
      Serial.println(" disconnected and removed");
    }
  }
}

//Check whether clients have new messages
//If new messages are available add messages to the list
int checkNewMessages()
{
  for (size_t i = 0; i < MAX_NOF_CLIENTS; i++)
  {
    //Check if client exists and has any messages
    if (clients[i] != NULL && clients[i]->available())
    {
      //Serial.println("New received data as hex:");
      //Serial.println("---");
      while(clients[i]->available())
      {
        char c = clients[i]->read();
        //Serial.println(c, HEX);
        if (!(c == '\n' || c=='\r')) input_msgs[i] += String(c);
      }
      Serial.println("---");
    }
  }
  
  return 0;
}

//Get new message from memory, optionally clear the message after reading
String getNewMessage(int index, bool clear = false)
{
  String msg = input_msgs[index];
  if (clear) input_msgs[index] = "";
  return msg;
}

//Clear all new messages from memory
void clearNewMessages()
{
  for (size_t i = 0; i < MAX_NOF_CLIENTS +1; i++)
  {
    input_msgs[i] = "";
  }
}


//Send message to the client
int sendMessage(const String newMessage, size_t atIndex)
{
  if(atIndex == MAX_NOF_CLIENTS)
  {
    //This is the master button itself
    handleMessage(newMessage);
    return 0;
  }
  else if(clients[atIndex] != NULL && clients[atIndex]->connected())
  {
    clients[atIndex]->println(newMessage);
    Serial.println("Message sent");
  }
  else
  {
    Serial.print("Client at index ");
    Serial.print(atIndex);
    Serial.println(" is not connected!");
    return -1;
  }

  return 0;
}

//This is only for testing purposes
void testLogic()
{
  
  //Read messages
  for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++)
  {

    if (clients[i] != NULL && GREEN == -1)
    {
      sendMessage(msg_ask_color, i);
    }
    
    //Handle button press
    if (getNewMessage(i) == msg_btn_pressed_short)
    {
      /*
      if (asia = true)
      {
        tee tämä
      }
      else
      {
        tee toisin
      }

      (asia == true) ? tee tämä : tee toisin
      */
      
      //Send message to change led state
      (led_states[i] == HIGH) ? led_states[i] = LOW : led_states[i] = HIGH;
      (led_states[i] == HIGH) ? sendMessage(msg_led_on, i) : sendMessage(msg_led_off, i);
    }
    else if (getNewMessage(i) == msg_btn_pressed_long)
    {
      //Serial.println("long press");
    }
    else if (getNewMessage(i) == msg_color_green)
    {
      //GREEN = i;
      Serial.print("Vihree: ");
      Serial.println(i);
    }
    else
    {
      //Serial.println(getNewMessage(i));
    }
  }

  //clear messages after handling
  clearNewMessages();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  digitalWrite(LED_PIN, LOW);  //turn off by LOW

  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleInterrupt, CHANGE);

  //Serial connection for debugging purposes
  Serial.begin(9600);
  delay(1000);

  //Connect to wifi network:
  WiFi.begin(ssid, password);

  Serial.println("Wifi parameters:");
  Serial.print("ssid: ");
  Serial.println(ssid);
  Serial.print("password: ");
  Serial.println(password);
    
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Trying to connect wifi...");
  }
  Serial.print("\nWifi connected! My ip: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  //5 blinks indicates successfull wifi connection
  blink(5);

  //Start the server
  wifiServer.begin();
  Serial.println("Server started");
}

//--------------------Main program--------------------

void loop() {
  
  //Check if some existing clients disconnected:
  removeDisconnectedClients();

  //Check if new clients are available and store them
  if (checkNewClients() != 0) Serial.println("Error while adding client");

  //Handle buttons
  handleButtonPress();

  //clearReceivedEvents();
  //receiveAllEvents();
  
  //Check whether clients have new messages
  checkNewMessages();

  //Handle gameplay here
  //gameLogic();

  testLogic();

  //sendAllEvents();
  //clearOutgoingEvents();
  
  delay(10);
}