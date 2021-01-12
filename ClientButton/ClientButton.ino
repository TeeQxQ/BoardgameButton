#include "colors.h"
#include "ESP8266WiFi.h"
#include "events.h"
#include "messages.h"

#define ISR_PREFIX ICACHE_RAM_ATTR

//Hardware parameters:
const int LED_PIN = 0;
const int BTN_PIN = 14;
const Color BTN_COLOR = GREEN;

bool ledState = LOW;
bool blinkEnabled = false;
const int blinkDelay_ms = 500;
long lastBlinkTime_ms = 0;
int ledBrightness = 0;

//Wifi parameters:
const char* ssid = "Kalat_ja_Rapu_2G";
const char* password = "rutaQlli";

//Wifi related variables:
WiFiClient wifiClient;
const char* clientAddress = "192.168.1.115";
const int clientPort = 80;

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

//Communication related:
//Buffer events to be send/received
Event receivedEvent;;
Event outgoingEvent;

//--------------------Event sending/receiving--------------------

//Set new event to be sent later
void setEvent(EventType type, int data = 0)
{
  outgoingEvent.type = type;
  outgoingEvent.data = data;
}

//Get the latest available event
Event getEvent()
{
  return receivedEvent;
}

//Sends a single event to a master
void sendEvent(bool debug = false)
{
  //Don't send unknown events
  if (outgoingEvent.type == UNKNOWN) return;

  if(wifiClient.connected())
  {
    //Construct a message
    msg::msg["color"] = BTN_COLOR;
    msg::msg["event"] = static_cast<int>(outgoingEvent.type);
    msg::msg["data"] = static_cast<int>(outgoingEvent.data);

    //Serialize the message and send it
    serializeJson(msg::msg, wifiClient);

    if (debug)
    {
      Serial.println("Event sent:");
      Serial.print("Event type: ");
      Serial.println(static_cast<int>(outgoingEvent.type));
      Serial.print("Event data: ");
      Serial.println(static_cast<int>(outgoingEvent.data));
      Serial.println("");
    }
  }
  else
  {
    Serial.println("Master button was lost");
  }
}

//Receive new event (if available) from the master
void receiveEvent(bool debug = false)
{
  if (wifiClient && wifiClient.available())
  {
    msg::err = deserializeJson(msg::msg, wifiClient);
    if (msg::err)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(msg::err.f_str());
    }
    else
    {
      //Construct an event from the message:
      receivedEvent.type = msg::msg["event"];
      receivedEvent.data = msg::msg["data"];

      if (debug)
      {
        Serial.println("Received event:");
        Serial.print("Event: ");
        Serial.println(static_cast<int>(receivedEvent.type));
        Serial.print("Data: ");
        Serial.println(static_cast<int>(receivedEvent.data));
        Serial.println("");
      }
    }
  }
}

//Clear the outgoing buffer
void clearOutgoingEvent()
{
  outgoingEvent.type = UNKNOWN;
  outgoingEvent.data = 0;
}

//Clear the receiving buffer
void clearReceivedEvent()
{
  receivedEvent.type = UNKNOWN;
  receivedEvent.data = 0;
}

//--------------------Event handling--------------------

void handleEvent(const Event e, bool debug = false)
{
  switch (e.type)
  {
    case UNKNOWN:
      break;
    case LED:
      ledBrightness = static_cast<int>(e.data);
      if (ledBrightness > 100) ledBrightness = 100;
      if (ledBrightness < 0) ledBrightness = 0;
      analogWrite(LED_PIN, map(ledBrightness, 0, 100, 0, 1023));
      break;
    case LED_ON:
      digitalWrite(LED_PIN, HIGH);
      break;
    case LED_OFF:
      digitalWrite(LED_PIN, LOW);
      break;
    case BLINK:
      blinkEnabled = true;
      break;
    case BLINK_ON:
      blinkEnabled = true;
      break;
    case BLINK_OFF:
      blinkEnabled = false;
      digitalWrite(LED_PIN, LOW);
      break;
    case BTN_SHORT:
      break;
    case BTN_LONG:
      break;
    case COLOR:
      setEvent(COLOR, BTN_COLOR);
      if (debug) Serial.println("Color asked");
      break;
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

//Handles how blinking works on this device
void handleBlinking()
{
  if(blinkEnabled)
  {
    if(millis() - lastBlinkTime_ms > blinkDelay_ms)
    {
      ledState = !ledState;
      ledState ? digitalWrite(LED_PIN, HIGH) : digitalWrite(LED_PIN, LOW);
      lastBlinkTime_ms = millis();
    }
  }
}

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
void handleButtonPress(bool debug = false)
{
  Event btnEvent;
  btnEvent.type = UNKNOWN;
  
  if (btn_released_counter > 0)
  {
    if (btn_released_time_ms - btn_pressed_time_ms > btn_long_press_threshold_MIN_ms &&
        btn_released_time_ms - btn_pressed_time_ms < btn_long_press_threshold_MAX_ms)
    {
      btnEvent.type = BTN_LONG;
      if (debug) Serial.println("Long btn press");
    }
    else if (btn_released_time_ms - btn_pressed_time_ms <= btn_long_press_threshold_MIN_ms)
    {
      btnEvent.type = BTN_SHORT;
      if (debug) Serial.println("Short btn press");
    }
    btn_released_counter = 0;
    btn_pressed_counter = 0;
  }
  else if (btn_pressed_counter > 0)
  {
    if (millis() - btn_pressed_time_ms > btn_long_press_threshold_MAX_ms)
    {
      btnEvent.type = BTN_SHORT;
      if (debug) Serial.println("Short btn press without release event");
      btn_pressed_counter = 0;
    }
  }

  if (btnEvent.type != UNKNOWN)
  {
    setEvent(btnEvent.type);
    if (debug) Serial.println(btnEvent.type);
  }

}

//--------------------wifiClient connections--------------------

//Wait until main button is available and then connect
int connectToMainButton(const int waitTime_ms = 1000)
{
  if (!wifiClient.connect(clientAddress, clientPort))
  {
    delay(waitTime_ms);
    return -1;
  }
  Serial.print("Connected to ip: ");
  Serial.print(clientAddress);
  Serial.print(" port: ");
  Serial.println(clientPort);

  return 0;
}

//--------------------Setup--------------------

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  digitalWrite(LED_PIN, LOW);  //turn off by LOW

  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleInterrupt, CHANGE);

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
    delay(3000);
    Serial.println("Trying to connect wifi...");
  }
  Serial.print("\nWifi Connected! My ip: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  //5 blinks indicates successfull wifi connection
  //blink(5);
}

//--------------------Main program--------------------

void loop() {

  //Connect to the main button
  while (connectToMainButton() != 0)
  {
    Serial.println("Trying to connect to the main button...");
    delay(500);
  }

  if (wifiClient)
  {
    //While connection to the main button is alive:
    while(wifiClient.connected())
    {
      delay(10);
      
      //Handle buttons
      handleButtonPress();

      //Handle blinking
      handleBlinking();
      
      //Read if there are any new messages from the main button
      receiveEvent();

      //Handle new messages
      handleEvent(getEvent(), true);
      clearReceivedEvent();

      //Send new messages
      sendEvent(true);
      clearOutgoingEvent();

    }
  }
  
  wifiClient.stop();
  Serial.print("Disconnected: ");
  Serial.print(clientAddress);
  Serial.print(" port: ");
  Serial.println(clientPort);
}
