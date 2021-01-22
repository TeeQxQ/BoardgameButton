int player_order [nofColors] = {-1, -1, -1, -1, -1};
boolean all_passed = true;

unsigned long startTime;
unsigned long endTime;
int length_of_turn_s;

boolean turnBegings = true;
boolean message = false;

int nofOrderSelected = 0;
int nofPasses = 0;
int nofTurnDone = 0;

void gameLogic(){
  if (all_passed == true) playerOrder();
//  if (all_passed == false) playModeNations(); //select only one play mode
  if (all_passed == false) playModeBrass();
}

//---------------Select player order----------------
void playerOrder(){
  for (size_t i = 0; i < nofColors ; i++){
    if (nofOrderSelected == nofPlayers){
      all_passed = false;
      turnBegings = true;
      nofOrderSelected = 0;
      Serial.println("player order: ");
      for (size_t i = 0; i < nofColors ; i++){
        players[i].turnSelected = false;
        setEvent(static_cast<Color>(i),LED_OFF);
        Serial.println(players[player_order[i]].color);  //KATOTAAN, mitä tuottaa
        setEvent(static_cast<Color>(i), BLINK_ON);  
      }
      delay(3000);
      for (size_t i = 0; i < nofColors ; i++){
        setEvent(static_cast<Color>(i), BLINK_OFF);
      }
      Serial.println("");
    }
    else{
      for (size_t n = 0; n < nofColors ; n++){  
        if (player_order[n] == -1 && getEvent(static_cast<Color>(i)).type == BTN_SHORT && players[i].turnSelected == false){
          player_order[n] = i;
          setEvent(static_cast<Color>(i), LED_ON);
          Serial.println ("Selected player was: " + static_cast<String>(players[i].color));
          players[i].turnSelected = true;
          nofOrderSelected++;
          break;
        }
      }
    }
  }
}

//-----------Play Mode Nations----------------
void playModeNations(){ 
  if(turnBegings == true){ 
    startTime = millis ();
    turnBegings = false;
    message = true;
  }
  for (size_t i = 0; i < nofColors ; i++){
    if (players[player_order[i]].turnDone == false){
      if (message == true && players[player_order[i]].passed == false){
        setEvent(static_cast<Color>(player_order[i]), LED_ON);
        message = false;
      }
            
      if (players[player_order[i]].passed == true){ //Jos pelaaja passanut, tallennetaan arvo "0"
        players[player_order[i]].turnLength = "pass";
        endTurn (i);
        Serial.print("player ");
        Serial.print(players[player_order[i]].color);
        Serial.print(" is already passed"); 
        Serial.println("");    
      }
      else if (getEvent(players[player_order[i]].color).type == BTN_SHORT){
        saveTurnLength(i);
        endTurn (i);
        setEvent(static_cast<Color>(player_order[i]), LED_OFF);
      }
      else if (getEvent(players[player_order[i]].color).type == BTN_LONG){
        Serial.println("player pass");
        setEvent(static_cast<Color>(player_order[i]), LED_OFF); 
        saveTurnLength(i);
        endTurn (i);
        nofPasses++;
        players[player_order[i]].passed = true;
   
      }
      
      break;
    }
  }
  resetTurns();

}

//-----------Play Mode Brassa----------------
void playModeBrass(){ 
  if(turnBegings == true){ 
    startTime = millis ();
    turnBegings = false;
    message = true;
  }
  for (size_t i = 0; i < nofColors ; i++){
    if (players[player_order[i]].turnDone == false){
      if (message == true && players[player_order[i]].passed == false){
        setEvent(static_cast<Color>(player_order[i]), LED_ON);
        message = false;
      }
    
      if (getEvent(players[player_order[i]].color).type == BTN_SHORT){
        Serial.println("player pass"); 
        setEvent(static_cast<Color>(player_order[i]), LED_OFF); 
        saveTurnLength(i);
        endTurn (i);
        nofPasses++;
        players[player_order[i]].passed = true;
      }
      
      break;
    }
  }
  resetTurns();
}

//----------Reset Turns----------
void resetTurns(){
  if (nofTurnDone == nofPlayers){ 
    sendToDrive();
    nofTurnDone = 0;
    Serial.println("Round ended");
      for (size_t i = 0; i < nofColors ; i++){
        players[i].turnDone = false;
      } 
    if (nofPasses == nofPlayers){ 
      all_passed = true;
      for (size_t i = 0; i < nofColors ; i++){
        player_order[i] = -1;
        players[i].passed = false;    
      }  
    }
  }

}

//-----------Save Data ------------
void saveTurnLength(int i){
  endTime = millis ();
  length_of_turn_s = (endTime - startTime) / 1000;
  players[player_order[i]].turnLength = String(length_of_turn_s, DEC);
  Serial.print("player: ");
  Serial.print(players[player_order[i]].color);
  Serial.println(""); 
  
  Serial.println(length_of_turn_s);
  Serial.println("");
}

void endTurn (int i){
  turnBegings = true;
  players[player_order[i]].turnDone = true;
  nofTurnDone++;
}
