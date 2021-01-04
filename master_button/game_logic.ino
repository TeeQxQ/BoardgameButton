//Game variables
int GREEN = 0;
int RED = 1;
int BLUE = 2;
int YELLOW = 3;

String pelaajat [MAX_NOF_CLIENTS +1] = {"green", "red", "blue", "yellow"}; 

boolean all_passed = true;
boolean passed_players[MAX_NOF_CLIENTS +1] = {false, false, false, false};

int player_order [MAX_NOF_CLIENTS +1] = {-1, -1, -1, -1};


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


void playPhase(){
  // laitetaan tähän, että blinkkaa 5 kertaa jokaista  nappia kierroksen alun merkiksi

  
  //Serial.print("Täällä pyöritään!");
}


