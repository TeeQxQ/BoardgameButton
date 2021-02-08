int player_order [nofColors] = {-1, -1, -1, -1, -1};
boolean playerOrderSelected = false;
boolean waitStart = false;
boolean runPlayPhase = false;

unsigned long startTime;
unsigned long endTime;
int length_of_turn_s;

boolean turnBegings = true;
boolean message = false;

boolean predictablePlayerOrderEnabled;  
boolean passOverEnabled;
boolean changePlayerOrderEnabled;
boolean changeOrderByOneStepEnabled;
boolean onlyOneTurnPerPhase;

int nofOrderSelected = 0;
int nofPasses = 0;
int nofTurnDone = 0;

void gameLogic(){

  if(predictablePlayerOrderEnabled == true){
    if (playerOrderSelected == false) selectPlayerOrder();
    if (waitStart == true)waitStartOfPlayPhase();
    if (runPlayPhase == true) playPhase();
  }
  else{
    if (playerOrderSelected == false) selectStaticOrder();
    if (waitStart == true)waitStartOfPlayPhase();
    if (runPlayPhase == true) runRandomOrderGame();
  }
}

//---------------Set static player order for the game logic -------------------
// The order is static only for game logic. In the game the order is totally free
void selectStaticOrder(){
  for (size_t i = 0; i < nofColors ; i++){
    player_order[i] = i;    
  }
  playerOrderSelected = true;
  waitStart = true;
}


//---------------Select player order----------------
void selectPlayerOrder(){
  for (size_t i = 0; i < nofColors ; i++){
    if (nofOrderSelected == nofPlayers){
      playerOrderSelected = true;
      waitStart =true;
      nofOrderSelected = 0;
      Serial.println("player order is: ");
      for (size_t i = 0; i < nofColors ; i++){
        players[i].turnSelected = false;
        setEvent(static_cast<Color>(i), LED_OFF);     
      }
      Serial.println("");
      setEvent(static_cast<Color>(player_order[0]), BLINK_ON);
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

//---------------Change Order By One Step----------------
void changeOrderByOneStep(){
  int lastPlayer = player_order[0];
  int tempOrder[nofColors];
  for (size_t i = 1; i < nofPlayers - 1; i++){
    tempOrder[i-1]= player_order[i];
  }
  
  tempOrder[nofPlayers-1] = lastPlayer;
  for (size_t i = 0; i < nofPlayers; i++){
    player_order[i] = tempOrder[i];
  }
  
  setEvent(static_cast<Color>(player_order[0]), BLINK_ON);
  waitStart =true;
}


//---------------Wait strat command from the first playes----------------
void waitStartOfPlayPhase(){
  if (getEvent(players[player_order[0]].color).type == BTN_SHORT){
    Serial.println("Start");
    setEvent(static_cast<Color>(player_order[0]),  BLINK_OFF);
    turnBegings = true;
    waitStart =false;
    runPlayPhase = true;
  } 
}
  
//---------------Run phase without player order ----------------
void runRandomOrderGame(){
  if(turnBegings == true){ 
    startTime = millis ();
    turnBegings = false;
  }
  
  for (size_t i = 0; i < nofColors ; i++){
    if (getEvent(players[player_order[i]].color).type == BTN_SHORT){
      Serial.println("BTN_SHORT");
      // setEvent(static_cast<Color>(player_order[i]),XXX); //tähän eventti joka sytyttää ledin ja himmentää sen esim. 1-2 sekunnin aikana
      saveTurnLength(i);
      sendToDrive();
      turnBegings = true;
      for (size_t i = 0; i < nofColors ; i++){
        players[player_order[i]].turnLength = "";
      }

    }
    else if (getEvent(players[player_order[i]].color).type == BTN_LONG){
      Serial.println("BTN_LONG");
      setEvent(static_cast<Color>(player_order[i]), LED_OFF); 
      saveTurnLength(i);
      sendToDrive();
      waitStart = true;
      runPlayPhase = false;
      
      for (size_t i = 0; i < nofColors ; i++){
        players[player_order[i]].turnLength = "";
      } 
    }
  }  
}

//-----------Play phase with predictable player order----------------
void playPhase(){ 
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
            
      if (players[player_order[i]].passed == true){
        players[player_order[i]].turnLength = "pass";
        endTurn (i);
        Serial.print("player ");
        Serial.print(players[player_order[i]].color);
        Serial.print(" is already passed"); 
        Serial.println("");    
      }
      else if (getEvent(players[player_order[i]].color).type == BTN_SHORT){
        Serial.println("BTN_SHORT");
        setEvent(static_cast<Color>(player_order[i]), LED_OFF);
        saveTurnLength(i);
        endTurn (i);
        
        if(passOverEnabled == false && changePlayerOrderEnabled == true && onlyOneTurnPerPhase == true) passHandler(i);
        if(passOverEnabled == false && changePlayerOrderEnabled == true && onlyOneTurnPerPhase == false && nofPasses > 0) passHandler(i);
        
      }
      else if (getEvent(players[player_order[i]].color).type == BTN_LONG){
        Serial.println("BTN_LONG");
        setEvent(static_cast<Color>(player_order[i]), LED_OFF); 
        saveTurnLength(i);
        endTurn (i);

        if(changePlayerOrderEnabled == false)stopTime();
        else passHandler(i);
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
    for (size_t i = 0; i < nofColors ; i++){
        players[player_order[i]].turnLength = "";
      } 
    nofTurnDone = 0;
    Serial.println("Round ended");
      for (size_t i = 0; i < nofColors ; i++){
        players[i].turnDone = false;
      } 
    if (nofPasses == nofPlayers){ 

      for (size_t i = 0; i < nofColors ; i++){
        players[i].passed = false; 
      }
      
      if(passOverEnabled == false && changePlayerOrderEnabled == false){
        waitStart = true;
        runPlayPhase = false;        
      }
      else if(changeOrderByOneStepEnabled == false){
        playerOrderSelected = false;
        runPlayPhase = false; 
      }
      else if(changeOrderByOneStepEnabled == true){
        runPlayPhase = false; 
        changeOrderByOneStep(); 
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


//-----------Stop time ------------
void stopTime(){
  nofTurnDone = nofPlayers;
  nofPasses = nofPlayers;
}

//-----------Pass Handler ------------
void passHandler(int i){
  nofPasses++;
  players[player_order[i]].passed = true;
}

//-----------End Turn Handler ------------
void endTurn (int i){
  turnBegings = true;
  players[player_order[i]].turnDone = true;
  nofTurnDone++;
}
