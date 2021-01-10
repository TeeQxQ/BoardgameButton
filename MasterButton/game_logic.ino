//Game variables
String green_time;
String red_time;
String blue_time; 
String yellow_time;

String pelaajat [MAX_NOF_CLIENTS +1] = {"green", "red", "blue", "yellow"}; // Tarvitaan koodia: kun nappi liittyy, kertoo se, mkä on sen veri ja väri tallennetaan listaan liittymisjärjestyksessä 

boolean all_passed = true;
boolean passed_players[MAX_NOF_CLIENTS +1] = {false, false, false, false};
boolean turnDone_player[MAX_NOF_CLIENTS +1] = {false, false, false, false};


int player_order [MAX_NOF_CLIENTS +1] = {-1, -1, -1, -1};

unsigned long startTime;
unsigned long endTime;
int length_of_turn_s;

boolean turnBegings = true;
String turnLength [MAX_NOF_CLIENTS +1];
boolean message = false;
boolean blinking;

//-----Googleen liittyvät arvot
#include <WiFiClientSecure.h>
const char* host = "script.google.com";
const int httpsPort = 443;
String readString;
WiFiClientSecure client;
const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
String GAS_ID = "AKfycbyyMIrQdgzFpbFaOPDEEMih1gN-afdZAdnqPT-_egqosxMO9NBL";  // Replace by your GAS service id


/*
void gameLogic(){
  if (all_passed == true) playerOrder();
  if (all_passed == false) playPhase();

}
//---------------Valitaan pelaajajärjestys----------------
void playerOrder(){
  
  for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
    if (i == MAX_NOF_CLIENTS && player_order[i] != -1){
      all_passed = false;
      blinking = true;
      turnBegings = true;
      clearPasses();
      Serial.print("player order selected order is: ");
      for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
        //sendMessage(msg_led_off, i);
        Serial.println(player_order[i]);    
      }
      Serial.println("");
    }
    else if (player_order[0] == -1 && getNewMessage(i) == msg_btn_pressed_short && passed_players[i] == false){
      player_order[0] = i;
      Serial.println ("First player is: " + pelaajat [i]);
      //sendMessage(msg_led_on, i);
      passed_players[i] = true;
    }
    else if(player_order[1] == -1 && getNewMessage(i) == msg_btn_pressed_short && passed_players[i] == false){
      player_order[1] = i;
      Serial.println ("Second player is: " + pelaajat [i]);
      //sendMessage(msg_led_on, i);
      passed_players[i] = true;
    }
    else if(player_order[2] == -1 && getNewMessage(i) == msg_btn_pressed_short && passed_players[i] == false){
      player_order[2] = i;
      Serial.println ("Trhird player is: " + pelaajat [i]);
      //sendMessage(msg_led_on, i);
      passed_players[i] = true;
    }
    else if(player_order[3] == -1 && getNewMessage(i) == msg_btn_pressed_short && passed_players[i] == false){
      player_order[3] = i;
      Serial.println ("fourth player is: " + pelaajat [i]);
      //sendMessage(msg_led_on, i);
      passed_players[i] = true;
    }
    
  }
  clearNewMessages();  
}

//-----------pelivaihe----------------
void playPhase(){
   // laitetaan tähän, että blinkkaa 5 kertaa jokaista  nappia kierroksen alun merkiksi
   
  if (blinking == true){
    blinking = false;
    blink(5);      
  }
  
  if(turnBegings == true){ 
    startTime = millis ();
    turnBegings = false;
    message = true;
  }

  for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
    if (turnDone_player [i] == false){
      if (message == true && passed_players[i] == false){
        //sendMessage(msg_led_on, player_order[i]);
        message = false;
      }
            
      if (passed_players[i] == true){ //Jos pelaaja passanut, tallennetaan arvo "0"
        turnLength[i] = "0";
        turnBegings = true; 
        turnDone_player [i] = true;
        Serial.println("player is passed");
        Serial.println(pelaajat [player_order[i]]); 
        Serial.println("");    
      }
      else if (getNewMessage(player_order[i]) == msg_btn_pressed_short){
        saveTurnLength(i);
        Serial.println("TURN END");

      }
      else if (getNewMessage(player_order[i]) == msg_btn_pressed_long){
        Serial.println("player pass");
        saveTurnLength(i);
        passed_players[i] = true;  
      }
      
      break;
    }
  }
 
  // ------ kaikki tehneet vuoronsa ------
  if (turnDone_player[0] == true && turnDone_player[1] == true && turnDone_player[2] == true && turnDone_player[3] == true){ 
    send_data();

    Serial.println("Round ended");
    for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
      turnDone_player [i] = false;
    }
    
    if (passed_players[0] == true && passed_players[1] == true && passed_players[2] == true && passed_players[3] == true){ 
      all_passed = true;
      clearOrder();
      clearPasses();
    }
  }
  clearNewMessages();         
  //Serial.print("Täällä pyöritään!"); 
}


void saveTurnLength(int i){
  endTime = millis ();
  length_of_turn_s = (endTime - startTime) / 1000;
  turnLength[i] = String(length_of_turn_s, DEC);
  //sendMessage(msg_led_off, player_order[i]);
  turnBegings = true;
  turnDone_player [i] = true;
  Serial.println(pelaajat [player_order[i]]); 
  Serial.println(length_of_turn_s);
  Serial.println("");
}

void send_data (){
  client.setInsecure();
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, host)) {
  Serial.println("certificate matches");
  } else {
  Serial.println("certificate doesn't match");
  }
  
  for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
    if (pelaajat [player_order[i]].equals("green"))green_time = turnLength[i];
    if (pelaajat [player_order[i]].equals("red")) red_time = turnLength[i];
    if (pelaajat [player_order[i]].equals("blue")) blue_time = turnLength[i];
    if (pelaajat [player_order[i]].equals("yellow")) yellow_time = turnLength[i];
  }
  
  String url = "/macros/s/" + GAS_ID + "/exec?green=" + green_time + "&red=" + red_time+ "&blue=" + blue_time + "&yellow=" + yellow_time;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
    String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
  Serial.println("esp8266/Arduino CI successfull!");
  } else {
  Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

  Serial.println ("green: " + green_time);
  Serial.println ("red: " + red_time);
  Serial.println ("blue: " + blue_time);
  Serial.println ("yellow :" + yellow_time);

  
}

void clearOrder(){
  for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
    player_order[i] = -1;    
   }  
}

void clearPasses(){
  for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
    passed_players[i] = false;
  }
}
*/
