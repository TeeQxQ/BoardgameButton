//Game variables
int GREEN = 0;
int RED = 1;
int BLUE = 2;
int YELLOW = 3;

String pelaajat [MAX_NOF_CLIENTS +1] = {"green", "red", "blue", "yellow"}; 
//pelaajat [0] = "green";
//pelaajat [1] = "Red";
//pelaajat [2] = "Blue";
//pelaajat [3] =  "Yellow";


boolean all_passed = true;
boolean passed_players[MAX_NOF_CLIENTS +1] = {false, false, false, false};

int player_order [MAX_NOF_CLIENTS +1];


void gameLogic(){
  if (all_passed == true) playerOrder();
  if (all_passed == false) playPhase();
  
}
//---------------Valitaan pelaajajärjestys----------------
void playerOrder(){
  
  if (player_order[0] == NULL){
    //Read messages
    for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
      if (getNewMessage(i) == msg_btn_pressed_short){
        player_order[0] = i;
        Serial.println ("First player is: " + pelaajat [i]);
        sendMessage(msg_led_on, i);
        passed_players[i] = true;
        }
      }
  }else if (player_order[1] == NULL){
    for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
      if (getNewMessage(i) == msg_btn_pressed_short){
        if(passed_players[i] == false){
          player_order[1] = i;
          Serial.println ("Second player is: " + pelaajat [i]);
          sendMessage(msg_led_on, i);
          passed_players[i] = true;
        }
      }
    }        
  }else if (player_order[2] == NULL){
    for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
      if (getNewMessage(i) == msg_btn_pressed_short){
        if(passed_players[i] == false){
          player_order[2] = i;
          Serial.println ("Third player is: " + pelaajat [i]);
          sendMessage(msg_led_on, i);
          passed_players[i] = true;
        }
      }
    }
  }else if (player_order[3] == NULL){
    for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
      if (getNewMessage(i) == msg_btn_pressed_short){
        if(passed_players[i] == false){
          player_order[3] = i;
          Serial.println ("fourth player is: " + pelaajat [i]);
          sendMessage(msg_led_on, i);
          passed_players[i] = true;
        }
      }
    }
  } /* Jostain syystä mun koodi ei koskaan saavuta tätä. Mikä mahtaa olla syynä? Tein erillisen ifin tohon alas. Sen kanssa toimii
  
  
  else{
    all_passed = false;
    Serial.println ("tan kautta");
    for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
      sendMessage(msg_led_off, i);
      passed_players[i] = false;
      Serial.println ("hetki tassa");    
    }
  } */

  if( passed_players[0] == true && passed_players[1] == true && passed_players[2] == true && passed_players[3] == true){
    all_passed = false;
    Serial.println ("tan kautta");
    for (size_t i = 0; i < MAX_NOF_CLIENTS +1 ; i++){
      sendMessage(msg_led_off, i);
      passed_players[i] = false;
      Serial.println ("hetki tassa");    
    }
  }
  clearNewMessages();  
}

void playPhase(){
  // laitetaan tähän, että blinkkaa 5 kertaa jokaista  nappia kierroksen alun merkiksi

  
  //Serial.print("Täällä pyöritään!");
}
