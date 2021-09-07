#include "button.h"
#include "colors.h"
#include "ESP8266WiFi.h"
#include "events.h"
#include "fifo.h"
#include "messages.h"

#define ISR_PREFIX ICACHE_RAM_ATTR
#define DEBUG

const int SERIAL_BAUNDRATE = 115200;

//Hardware parameters:
const int LED_PIN = 2; //D4: gpio2 (internal led)
const int BTN_PIN = 4; //D2: gpio4
Color BTN_COLOR = UNDEFINED;

//LED related parameters
const int LED_BRIGHTNESS_MAX = 255; //1023;
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
volatile unsigned long btnPressedTime_ms = 0;
volatile unsigned long btnReleasedTime_ms = 0;
const unsigned long btnDebounceThreshold_ms = 10;
const byte btnBufferSize = 10;
volatile int btnBuffer[btnBufferSize];              //bfr to store btn press lengths
volatile unsigned long btnTsBuffer[btnBufferSize];  //bfr to store btn press timestamps
volatile byte btnPressedCount = 0; 
const unsigned long btnDoubleClickDelayMax_ms = 500; //Max time to wait second click
//Fifo<BtnClick> testBuffer(10);

//Communication related:
//Buffer events to be send/received
Event receivedEvent;
Event outgoingEvent;

//--------------------Event sending/receiving--------------------

//Set new event to be sent later
void setEvent(EventType type, int data = 0, unsigned long ts = 0)
{
  outgoingEvent.type = type;
  outgoingEvent.data = data;
  outgoingEvent.ts = ts;
}

//Get the latest available event
Event getEvent()
{
  return receivedEvent;
}

//Sends a single event to a master
void sendEvent()
{
  //Don't send unknown events
  if (outgoingEvent.type == UNKNOWN) return;

  if(wifiClient.connected())
  {
    //Construct a message
    msg::msg["color"] = BTN_COLOR;
    msg::msg["event"] = static_cast<int>(outgoingEvent.type);
    msg::msg["data"] = static_cast<int>(outgoingEvent.data);
    msg::msg["ts"] = static_cast<unsigned long>(outgoingEvent.ts);

    //Serialize the message and send it
    serializeJson(msg::msg, wifiClient);

#ifdef DEBUG
    Serial.print("Event sent: ");
    Serial.print(eventToString(outgoingEvent.type));
    Serial.print(" (");
    Serial.print(static_cast<int>(outgoingEvent.data));
    Serial.print(", ");
    Serial.print(static_cast<unsigned long>(outgoingEvent.ts));
    Serial.println(")");
#endif

    clearOutgoingEvent();
  }

#ifdef DEBUG
  else
  {
    Serial.println("Game server was lost");
  }
#endif

}

//Receive new event (if available) from the master
void receiveEvent()
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
      receivedEvent.ts = msg::msg["ts"];

#ifdef DEBUG
      Serial.print("Received event: ");
      Serial.print(eventToString(receivedEvent.type));
      Serial.print(" (");
      Serial.print(static_cast<int>(receivedEvent.data));
      Serial.print(", ");
      Serial.print(static_cast<unsigned long>(receivedEvent.ts));
      Serial.println(")");
#endif
    }
  }
}

//Clear the outgoing buffer
void clearOutgoingEvent()
{
  outgoingEvent.type = UNKNOWN;
  outgoingEvent.data = 0;
  outgoingEvent.ts = 0;
}

//Clear the receiving buffer
void clearReceivedEvent()
{
  receivedEvent.type = UNKNOWN;
  receivedEvent.data = 0;
  receivedEvent.ts = 0;
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
  const int ledBrightness_max = 100;
  const int ledBrightness_min = 0;
  if (breathingEnabled)
  {
    if (millis() - lastBreathingTime_ms > breathingDelay_ms)
    {
      lastBreathingTime_ms = millis();
      if (inhale)
      {
        ledBrightness++;
        if (ledBrightness > ledBrightness_max)
        {
          ledBrightness = ledBrightness_max;
          inhale = false;
        }
      }
      else
      {
        ledBrightness--;
        if (ledBrightness < ledBrightness_min)
        {
          ledBrightness = ledBrightness_min;
          inhale = true;
        }
      }
      analogWrite(LED_PIN, map(ledBrightness, ledBrightness_min, ledBrightness_max, LED_BRIGHTNESS_MIN, LED_BRIGHTNESS_MAX));
     }
  }
}

//--------------------Event handling--------------------

void handleEvent(const Event e)
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
      blinkEnabled = false;
      analogWrite(LED_PIN, LED_BRIGHTNESS_MAX);
      break;
    case LED_OFF:
      blinkEnabled = false;
      analogWrite(LED_PIN, LED_BRIGHTNESS_MIN);
      break;
    case BLINK:
      blink(static_cast<int>(e.data));
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
      break;
    default:
      break;
  }
}

//--------------------Interrupts--------------------

//Handles interrupt caused by the button press
ISR_PREFIX void handleInterrupt()
{
  const unsigned long t_ms = millis();
  /*if (digitalRead(BTN_PIN))
  {
    btnPressedTime_ms = t_ms;
  }
  else
  {
    btnReleasedTime_ms = t_ms;
    btnPressedCount++;
  }*/


    
  if (t_ms - btnPressedTime_ms >= btnDebounceThreshold_ms &&
      t_ms - btnReleasedTime_ms >= btnDebounceThreshold_ms)
  {
    if (digitalRead(BTN_PIN))
    {
      btnPressedTime_ms = t_ms;
    }
    else
    {
      btnReleasedTime_ms = t_ms;

      btnPressedCount++;
      /*
      //Make sure counter stays within limits
      if (btnPressedCount <= btnBufferSize)
      {
         btnTsBuffer[btnPressedCount - 1] = btnPressedTime_ms;
         btnBuffer[btnPressedCount - 1] = btnReleasedTime_ms - btnPressedTime_ms;
      }*/
    }
  }
}

//--------------------Buttons--------------------

//Handles button press
void handleButtonPress()
{
  Event btnEvent;
  btnEvent.type = UNKNOWN;
  btnEvent.data = 0;
  btnEvent.ts = 0;

  if (btnPressedCount > 0)
  {
    Serial.println("Click");
    --btnPressedCount;
  }

  /*if (btnPressedCount > 0 && 
      millis() - max(btnPressedTime_ms, btnReleasedTime_ms) >= btnDoubleClickDelayMax_ms)
  {
    //double click
    if (btnPressedCount == 2)
    {
      btnEvent.type = BTN_DOUBLE;
      btnEvent.data = btnBuffer[0] + btnBuffer[1];
    }
    //single click
    else
    {
      btnEvent.type = BTN;
      btnEvent.data = btnBuffer[0];
    }
    btnEvent.ts = btnTsBuffer[0];
    btnPressedCount = 0;

    setEvent(btnEvent.type, btnEvent.data, btnEvent.ts);
  }*/
}

//--------------------Button color--------------------

//Determines color based on mac address (unique for every board)
Color getColorByMac()
{
  Color btn_color = UNDEFINED;

  String mac = WiFi.macAddress();

  if (mac == "68:C6:3A:D6:3A:FE")
  {
    btn_color = BLUE;
  }
  else if (mac == "B4:E6:2D:23:2F:4D")
  {
    btn_color = YELLOW;
  }
  else
  {
#ifdef DEBUG
  Serial.print("Unknown color for MAC: ");
  Serial.println(mac);
#endif
  }
  //Add more colors here

  return btn_color;
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

#ifdef DEBUG
    Serial.println("Connected to Game server:");
    Serial.print("- ip: ");
    Serial.println(clientAddress);
    Serial.print("- port: ");
    Serial.println(clientPort);
#endif

    //Connection established
    blink(5, 200);
  }
  return 0;
}

//--------------------Setup--------------------

void setup() {

#ifdef DEBUG
  //Serial connection for debugging purposes
  Serial.begin(SERIAL_BAUNDRATE);
  delay(1000);
#endif

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT);

  digitalWrite(LED_PIN, LOW);  //turn off by LOW

  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleInterrupt, CHANGE);

  //Explicitely define this is a station, not access point
  WiFi.mode(WIFI_STA);

  //Connect to wifi network:
  WiFi.begin(ssid, password);

  //Determine own color based on mac (wifi must be started)
  BTN_COLOR = getColorByMac();


#ifdef DEBUG
  Serial.println("");
  Serial.print("Color of the button: ");
  Serial.println(colorToString(BTN_COLOR));

  Serial.println("Wifi (AP) parameters:");
  Serial.print("- ssid: ");
  Serial.println(ssid);
  Serial.print("- password: ");
  Serial.println(password);
#endif

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
        //Send new messages
        sendEvent();
  
        //Read if there are any new messages from the main button
        receiveEvent();
  
        //Handle new messages
        handleEvent(getEvent());
        clearReceivedEvent();
      }
    }
  }
}
