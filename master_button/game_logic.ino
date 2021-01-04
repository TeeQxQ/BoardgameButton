//Game variables
int GREEN = 0;
int RED = 1;
int BLUE = 2;
int YELLOW = 3;

String pelaajat [MAX_NOF_CLIENTS +1] = {"green", "red", "blue", "yellow"}; 

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

void gameLogic(){
  if (all_passed == true) playerOrder();
  if (all_passed == false) playPhase();

}
//---------------Valitaan pelaajajärjestys----------------
void playerOrder(){
  
  for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
    if (i == MAX_NOF_CLIENTS && player_order[i] != -1){
      all_passed = false;
      Serial.print("player order selected order is: ");
      for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
        sendMessage(msg_led_off, i);
        passed_players[i] = false;
        Serial.print(player_order[i]);    
      }
      Serial.print("");
    }
    else if (player_order[0] == -1 && getNewMessage(i) == msg_btn_pressed_short && passed_players[i] == false){
      player_order[0] = i;
      Serial.println ("First player is: " + pelaajat [i]);
      sendMessage(msg_led_on, i);
      passed_players[i] = true;
    }
    else if(player_order[1] == -1 && getNewMessage(i) == msg_btn_pressed_short && passed_players[i] == false){
      player_order[1] = i;
      Serial.println ("Second player is: " + pelaajat [i]);
      sendMessage(msg_led_on, i);
      passed_players[i] = true;
    }
    else if(player_order[2] == -1 && getNewMessage(i) == msg_btn_pressed_short && passed_players[i] == false){
      player_order[2] = i;
      Serial.println ("Trhird player is: " + pelaajat [i]);
      sendMessage(msg_led_on, i);
      passed_players[i] = true;
    }
    else if(player_order[3] == -1 && getNewMessage(i) == msg_btn_pressed_short && passed_players[i] == false){
      player_order[3] = i;
      Serial.println ("fourth player is: " + pelaajat [i]);
      sendMessage(msg_led_on, i);
      passed_players[i] = true;
    }
    
  }
  clearNewMessages();  
}

//-----------pelivaihe----------------
void playPhase(){
  // laitetaan tähän, että blinkkaa 5 kertaa jokaista  nappia kierroksen alun merkiksi

  if(turnBegings == true){ 
    startTime = millis ();
    turnBegings = false;
    message = true;
  }
//--------- Pelaaja 1 ---------
  if (turnDone_player [0] == false){
    if (message == true && passed_players[0] == false){
      sendMessage(msg_led_on, player_order[0]);
      message = false;
    }
    
    if (passed_players[0] == true){ //Jos pelaaja passanut, tallennetaan arvo "0"
      passed(0);      
    }
    else if (getNewMessage(player_order[0]) == msg_btn_pressed_short){
      saveTurnLength(0);
    }
    else if (getNewMessage(player_order[0]) == msg_btn_pressed_long){
      savePass(0);
    }
  }
  //--------- Pelaaja 2 ---------
  else if (turnDone_player [1] == false){
    if (message == true && passed_players[1] == false){
      sendMessage(msg_led_on, player_order[1]);
      message = false;
    }
    
    if (passed_players[1] == true){ 
      passed(1);      
    }
    else if (getNewMessage(player_order[1]) == msg_btn_pressed_short){
      saveTurnLength(1);
    }
    else if (getNewMessage(player_order[1]) == msg_btn_pressed_long){
      savePass(1);
    }
  }
  //--------- Pelaaja 3 ---------
  else if (turnDone_player [2] == false){
    if (message == true && passed_players[2] == false){
      sendMessage(msg_led_on, player_order[2]);
      message = false;
    }
    
    if (passed_players[2] == true){ 
      passed(2);      
    }
    else if (getNewMessage(player_order[2]) == msg_btn_pressed_short){
      saveTurnLength(2);
    }
    else if (getNewMessage(player_order[2]) == msg_btn_pressed_long){
      savePass(2);
    }
  }
  //--------- Pelaaja 4 ---------
  else if (turnDone_player [3] == false){
    if (message == true && passed_players[3] == false){
      sendMessage(msg_led_on, player_order[3]);
      message = false;
    }
    
    if (passed_players[3] == true){ //Jos pelaaja passanut, tallennetaan arvo "0")
      passed(3);      
    }
    else if (getNewMessage(player_order[3]) == msg_btn_pressed_short){
      saveTurnLength(3);
    }
    else if (getNewMessage(player_order[3]) == msg_btn_pressed_long){
      savePass(3);
    }
  }
  // ------ kaikki tehneet vuoronsa ------
  else if (turnDone_player[0] == true && turnDone_player[1] == true && turnDone_player[2] == true && turnDone_player[3] == true){ 
    sendData ("KISSA");

    Serial.println("Round ended");
    for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
      turnDone_player [i] = false;
    }
    
    if (passed_players[0] == true && passed_players[1] == true && passed_players[2] == true && passed_players[3] == true){ 
      all_passed = true;
      clearOrder();
      Serial.println("playPhase ended");
      for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
        passed_players[i] = false;
      }
    }
  }
  clearNewMessages();         
  //Serial.print("Täällä pyöritään!"); 
}



void passed(int i){
  turnLength[i] = "0";
  turnBegings = true; 
  turnDone_player [i] = true;
  Serial.println("player is passed");
  Serial.println(i); 
}

void saveTurnLength(int i){
  endTime = millis ();
  length_of_turn_s = (endTime - startTime) / 1000;
  turnLength[i] = String(length_of_turn_s, DEC);
  sendMessage(msg_led_off, player_order[i]);
  turnBegings = true;
  turnDone_player [i] = true;
  passed_players[1] = true;
  Serial.println("TURN END");
  Serial.println(i); 
  Serial.println(length_of_turn_s);
}

void savePass(int i){
  endTime = millis ();
  length_of_turn_s = (endTime - startTime) / 1000;
  turnLength[i] = String(length_of_turn_s, DEC);
  sendMessage(msg_led_off, player_order[i]);
  turnBegings = true; 
  turnDone_player [i] = true;
  passed_players[i] = true;
  
  Serial.println("PASS player");
  Serial.println(i); 
  Serial.println(length_of_turn_s); 
}

void sendData (String i){
  Serial.println ("send data");
}

void clearOrder(){
  for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
    player_order[i] = -1;    
   }  
}
