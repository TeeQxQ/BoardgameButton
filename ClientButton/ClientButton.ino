#include "colors.h"
#include "ESP8266WiFi.h"
#include "events.h"
#include "messages.h"4

#define ISR_PREFIX ICACHE_RAM_ATTR

//Hardware parameters:
const int LED_PIN = 15;
const int BTN_PIN = 13;
const int DIP_PIN_1 = 2;
const int DIP_PIN_2 = 0;
const int DIP_PIN_3 = 4;
const int DIP_PIN_4 = 5;
Color BTN_COLOR = UNDEFINED;

//LED related parameters
const int LED_BRIGHTNESS_MAX = 1023;
const int LED_BRIGHTNESS_MIN = 0;
bool ledState = LOW;
int ledBrightness = 0;

//Blinking effect
bool blinkEnabled = false;
const long defaultBlinkDelay_ms = 500;
int blinkDelay_ms;
int blinkCount = 0;
long lastBlinkTime_ms = 0;

//Breathing effect
bool breathingEnabled = false;
const long defaultBreathingDelay_ms = 20;
int breathingDelay_ms ;
bool inhale = true;
long lastBreathingTime_ms = 0;

//Wifi parameters:
const char* ssid = "BoardGameBox";
const char* password = "box12345";

//Wifi related variables:
static WiFiClient wifiClient;
const char* clientAddress = "192.168.4.1";
const int clientPort = 80;

//Button related parameters
volatile byte btn_pressed_counter = 0;
volatile byte btn_released_counter = 0;
volatile int btn_state = HIGH;
volatile unsigned long btn_pressed_time_ms = 0;
volatile unsigned long btn_released_time_ms = 0;
const unsigned long btn_long_press_threshold_ms = 750;
const unsigned long debounceDelay_ms = 50;
volatile unsigned long lastDebounceTime_ms = 0;

//Increased every time BTN_PIN interrupts
volatile byte btnChangedCounter = 0;

//Time when the first BTN_PIN change occured
volatile unsigned long btnChangedTime_ms = 0;

//Number of changes needed to determine button release, 
//otherwise considered as button pressing
const byte thresholdForBtnRelease = 20;

//Time to wait for new state changes from the first BTN_PIN change
const unsigned long btnChangeRecordTime_ms = 40;

unsigned long btnPressedTime_ms = 0;
const unsigned long btnLongPressThreshold_ms = 750;


//Communication related:
//Buffer events to be send/received
Event receivedEvent;
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
      Serial.print("- Event type: ");
      Serial.println(eventToString(outgoingEvent.type));
      Serial.print("- Event data: ");
      Serial.println(static_cast<int>(outgoingEvent.data));
      Serial.println("");
    }
  }
  else
  {
    Serial.println("Game server was lost");
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
        Serial.println(eventToString(receivedEvent.type));
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
      analogWrite(LED_PIN, map(ledBrightness, 0, 100, LED_BRIGHTNESS_MIN, LED_BRIGHTNESS_MAX));
      break;
    case LED_ON:
      analogWrite(LED_PIN, LED_BRIGHTNESS_MAX);
      break;
    case LED_OFF:
      blinkEnabled = false;
      analogWrite(LED_PIN, LED_BRIGHTNESS_MIN);
      break;
    case BLINK:
      blinkEnabled = true;
      break;
    case BLINK_ON:
      blinkEnabled = true;
      break;
    case BLINK_OFF:
      blinkEnabled = false;
      analogWrite(LED_PIN, LED_BRIGHTNESS_MIN);
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

//--------------------Effects--------------------

void startBlinking()
{
  stopBreathing();
  blinkEnabled = true;
}

void stopBlinking()
{
  blinkEnabled = false;
  blinkCount = 0;
}

//Blink the led <times> times
void blink(int times, int delay_ms = defaultBlinkDelay_ms)
{
  blinkEnabled = true;
  blinkCount = times;
  blinkDelay_ms = delay_ms;
  lastBlinkTime_ms = millis();
}

//Handles how blinking works on this device
void handleBlinking()
{
  if(blinkEnabled)
  {
    breathingEnabled = false;
    if(millis() - lastBlinkTime_ms > blinkDelay_ms)
    {
      ledState = !ledState;
      ledState ? analogWrite(LED_PIN, LED_BRIGHTNESS_MAX) : analogWrite(LED_PIN, LED_BRIGHTNESS_MIN);
      lastBlinkTime_ms = millis();

      if (ledState)
      {
        blinkCount--;
      }
      else
      {
        if(blinkCount == 0) blinkEnabled = false;
      }

    }
  }
}

//Starts breathing effect if not already started
void startBreathing()
{
  if (!breathingEnabled)
  {
    stopBlinking();
    breathingEnabled = true;
    inhale = true;
    lastBreathingTime_ms = millis();
    ledBrightness = 0;
    breathingDelay_ms = defaultBreathingDelay_ms;
  }
}

//Stops breathing effect if not already stopped
void stopBreathing()
{
  if (breathingEnabled)
  {
    breathingEnabled = false;
    ledBrightness = 100;
  }
}

//Handles how breathing works on this device
void handleBreathing()
{
  if (breathingEnabled)
  {
    if (millis() - lastBreathingTime_ms > breathingDelay_ms)
    {
      lastBreathingTime_ms = millis();
      if (inhale)
      {
        ledBrightness++;
        if (ledBrightness > 100)
        {
          ledBrightness = 100;
          inhale = false;
        }
      }
      else
      {
        ledBrightness--;
        if (ledBrightness < 0)
        {
          ledBrightness = 0;
          inhale = true;
        }
      }
      analogWrite(LED_PIN, map(ledBrightness, 0, 100, LED_BRIGHTNESS_MIN, LED_BRIGHTNESS_MAX));
     }
  }
}

//--------------------Interrupts--------------------

//Handles interrupt caused by the button press
ISR_PREFIX void handleInterrupt()
{
  if (btnChangedCounter == 0)
  {
    btnChangedTime_ms = millis();
  }
  btnChangedCounter++;

  
  /*const int btnState = digitalRead(BTN_PIN);
  Serial.print(millis());
  Serial.print(":");
  Serial.println(btnState);*/
  /*unsigned long currentTime_ms = millis();
  
  if (currentTime_ms - lastDebounceTime_ms > debounceDelay_ms)
  {
    if (btnState == HIGH)
    {
      btn_pressed_counter++;
      btn_pressed_time_ms = currentTime_ms;
      //Serial.print("HIGH: ");
    }
    else if (btn_pressed_counter > 0)
    {
      btn_released_counter++;
      btn_released_time_ms = currentTime_ms;
      //Serial.print("LOW and btn pressed earlier: ");
    }
    else
    {
      Serial.println("error");
    }
    lastDebounceTime_ms = currentTime_ms;
    //Serial.println(currentTime_ms);
  }*/
}

//--------------------Buttons--------------------

//Handles button press
void handleButtonPress(bool debug = false)
{
  Event btnEvent;
  btnEvent.type = UNKNOWN;
  btnEvent.data = 0;

  if (btnChangedCounter > 0)
  {
      unsigned long currentTime_ms = millis();
      if (currentTime_ms - btnChangedTime_ms > btnChangeRecordTime_ms)
      {
        Serial.println(btnChangedCounter);
        if (btnChangedCounter >= thresholdForBtnRelease)
        {
          //Button was released
          if (btnChangedTime_ms - btnPressedTime_ms >= btnLongPressThreshold_ms)
          {
            btnEvent.type = BTN_LONG;
          }
          else
          {
            btnEvent.type = BTN_SHORT;
          }
          btnEvent.data = static_cast<int>(btnChangedTime_ms - btnPressedTime_ms);
        }
        else
        {
          //Button was pressed
          btnPressedTime_ms = btnChangedTime_ms;
        }

        //Changes handled, reset the counter
        btnChangedCounter = 0;
      }
  }

  /*
  if (btn_released_counter > 0)
  {
    if (btn_released_time_ms - btn_pressed_time_ms > btn_long_press_threshold_ms)
    {
      btnEvent.type = BTN_LONG;
      if (debug) Serial.print("Long (");
    }
    else
    {
      btnEvent.type = BTN_SHORT;
      if (debug) Serial.print("Short (");
    }
    btnEvent.data = static_cast<int>(btn_released_time_ms - btn_pressed_time_ms);
    if (debug) Serial.print(btn_released_time_ms - btn_pressed_time_ms);
    if (debug) Serial.println("ms)");

    btn_released_counter = 0;
    btn_pressed_counter = 0;
    btn_released_time_ms = millis();
    btn_pressed_time_ms = btn_released_time_ms;
    
  }*/

  if (btnEvent.type != UNKNOWN)
  {
    setEvent(btnEvent.type, btnEvent.data);
  }
}

//--------------------Button color--------------------

Color getColor(bool debug)
{
  if (debug)
  {
    Serial.println(digitalRead(DIP_PIN_1));
    Serial.println(digitalRead(DIP_PIN_2));
    Serial.println(digitalRead(DIP_PIN_3));
    Serial.println(digitalRead(DIP_PIN_4));
  }
  
  int bit0 = !digitalRead(DIP_PIN_1) << 3;
  int bit1 = !digitalRead(DIP_PIN_2) << 2;
  int bit2 = !digitalRead(DIP_PIN_3) << 1;
  int bit3 = !digitalRead(DIP_PIN_4);

  int dipValue = bit0 + bit1 + bit2 + bit3;
  if (debug)
  {
    Serial.print("DIP value: ");
    Serial.println(dipValue);
  }

  if (dipValue >= UNDEFINED)
  {
    return UNDEFINED;
  }

  return static_cast<Color>(dipValue);
}

//--------------------wifiClient connections--------------------

//Wait until main button is available and then connect
int connectToGameServer(const int waitTime_ms = 3000)
{
  if (!wifiClient.connected())
  {
    if (!wifiClient.connect(clientAddress, clientPort))
    {
      //delay(waitTime_ms);
      return -1;
    }
    Serial.println("Connected to Game server:");
    Serial.print("- ip: ");
    Serial.println(clientAddress);
    Serial.print("- port: ");
    Serial.println(clientPort);
    blink(5, 200);
  }
  return 0;
}

//--------------------Setup--------------------

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(DIP_PIN_1, INPUT);
  pinMode(DIP_PIN_2, INPUT);
  pinMode(DIP_PIN_3, INPUT);
  pinMode(DIP_PIN_4, INPUT);

  digitalWrite(LED_PIN, LOW);  //turn off by LOW

  //Serial connection for debugging purposes
  Serial.begin(115200);
  delay(1000);

  //Determine own color:
  BTN_COLOR = getColor(false);
  Serial.println("");
  Serial.print("Color of the button: ");
  Serial.println(colorToString(BTN_COLOR));

  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleInterrupt, CHANGE);

  //Explicitely define this is a station, not access point
  WiFi.mode(WIFI_STA);

  //Connect to wifi network:
  WiFi.begin(ssid, password);

  Serial.println("Wifi (AP) parameters:");
  Serial.print("- ssid: ");
  Serial.println(ssid);
  Serial.print("- password: ");
  Serial.println(password);

  /*
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(3000);
    Serial.println("Trying to connect wifi...");
  }
  Serial.println("Wifi Connected:");
  Serial.print("- ip: ");
  Serial.println(WiFi.localIP());*/
}

//--------------------Main program--------------------

void loop() {
  
  //Handle button press
  handleButtonPress();

  //Handle blinking
  handleBlinking();

  //Handle breathing
  handleBreathing();

  if (WiFi.status() != WL_CONNECTED)
  {
    //Effect to indicate lost connection
    startBreathing();

    //Disconnect client (if not already disconnected) if wifi was lost
    if (wifiClient)
    {
      wifiClient.stop();
      //Serial.println("Server disconnected");
    }
  }
  else
  {
    stopBreathing();

    //Connect to the box running game server
    if(connectToGameServer() != 0)
    {
      //Serial.println("Trying to connect to the game server...");
    }
    else
    {
      if (wifiClient && wifiClient.connected())
      {
        //Read if there are any new messages from the main button
        receiveEvent();
  
        //Handle new messages
        handleEvent(getEvent(), true);
        clearReceivedEvent();
  
        //Send new messages
        sendEvent(false);
        clearOutgoingEvent();
      }
    }
  }

  /*
  //Connect to the box running game server
  if(connectToGameServer() != 0)
  {
    Serial.println("Trying to connect to the game server...");
    delay(500);
  }
  else
  {
    if (wifiClient)
    {
      //While connection to the main button is alive:
      if(wifiClient.connected())
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
      else
      {
        wifiClient.stop();
        Serial.print("Disconnected: ");
        Serial.print(clientAddress);
        Serial.print(" port: ");
        Serial.println(clientPort);
      }
    }
  }*/
}
