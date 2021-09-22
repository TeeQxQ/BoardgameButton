#include "game.h"

Game::Game()
{
  init();
}

void Game::init()
{
  this->reset();

  //Set unique color for each possible player
  for (int i = 0; i < mMaxNofPlayers; i++)
  {
    if (i < nofColors)
    {
      mPlayers[i].setColor(static_cast<Color>(i));
    }
  }

  mJoinedPlayers = 0;
}

void Game::init(dbFunc_t saveToDriveFunc)
{
  this->init();
  saveToDb = saveToDriveFunc;
}

const Game::Action Game::play(const Action action)
{
  switch (currentGameState())
  {
    case ORDER_SELECTION:
      return playOrderSelection(action);
      break;
    case PLAY_TURNS:
      return playTurns(action);
      break;
    default:
      break;
  }
  return Action(UNDEFINED, UNKNOWN);
}

void Game::reset()
{
  initializeSettings();
  fetchGameSettings();
  
  //reset players
  for (int i = 0; i < mMaxNofPlayers; i++)
  {
    mPlayers[i].reset();
  }

  mNofTurnsSelected = 0;
  mIndexOfPlayerInTurn = 0;
  mIndexOfPlayerPreviouslyInTurn = 0;
  mState = ORDER_SELECTION;
  mOrderSelectionStartTime_ms = millis();
  mOrderSelectionTime_ms = 0;
}

const Game::Action Game::addPlayer(Color color)
{
  //Add player unless it is already in game
  int indexOfColor = static_cast<int>(color);
  if (indexOfColor < mMaxNofPlayers)
  {
    //Add new player
    if (!mPlayers[indexOfColor].isPlaying())
    {
      mPlayers[indexOfColor].isPlaying(true);
      mJoinedPlayers++;
      return Action(color, ADDED);
    }
  
    //Rejoin existing player
    return rejoinPlayer(color);
  }

  return Action(UNDEFINED, UNKNOWN);
}

const Game::Action Game::removePlayer(Color color)
{
  //Remove player unless it is not in game
  int indexOfColor = static_cast<int>(color);
  if (indexOfColor < mMaxNofPlayers && mPlayers[indexOfColor].isPlaying())
  {
    mPlayers[indexOfColor].isPlaying(false);
    mJoinedPlayers--;
    return Action(color, REMOVED);
  } 

  return Action(UNDEFINED, UNKNOWN);
}

const Game::Action Game::rejoinPlayer(Color color)
{
  int indexOfColor = static_cast<int>(color);
  if (indexOfColor < mMaxNofPlayers && mPlayers[indexOfColor].isPlaying())
  {
    switch (mState)
    {
      case ORDER_SELECTION:
        if (mPlayers[indexOfColor].turnSelected())
        {
          return Action(color, LED_ON);
        }
        return Action(color, LED_OFF);
        break;
      default:
        break;
    }
  }

  return Action(UNDEFINED, UNKNOWN);
}

void Game::initializeSettings()
{
  gameSettings.mTurnsPerRound = 1;
  gameSettings.mRoundOverWhenPassed = false;
}

//Old
void Game::fetchGameSettings()
{
  //TODO

  //mPredictablePlayerOrder = true;  
  //mPassOver = false;
  //mChangePlayerOrder = true;
  //mChangeOrderByOneStep = false;
  //mOnlyOneTurnPerPhase = true;
  mTurnsPerRound = 1; //UNLIMITED;               //Number of turns in a single round (0 - UNLIMITED)
  mRoundOverWhenPassed = false; //true;     //Round ends only after passing
}

void Game::printSettings()
{
  Serial.println("Game settings:");
  Serial.print("  *Turns per round: ");
  Serial.println(this->gameSettings.mTurnsPerRound);
  Serial.print("  *Round over when passed: ");
  Serial.println(this->gameSettings.mRoundOverWhenPassed);
  Serial.print("  *Order may change: ");
  Serial.println(this->gameSettings.mOrderMayChange);
  Serial.print("  *Chane order by steps: ");
  Serial.println(this->gameSettings.mChangeOrderBySteps);
  Serial.print("  *Predictable player order: ");
  Serial.println(this->gameSettings.mPredictablePlayerOrder);
  Serial.print("  *Log after rounds: ");
  Serial.println(this->gameSettings.mLogAfterRounds);
}

void Game::changeSettings(int turnsPerRound,
                          bool roundOverWhenPassed,
                          bool orderMayChange,
                          unsigned int changeOrderBySteps,
                          bool predictablePlayerOrder,
                          unsigned int logAfterRounds)
{
  this->gameSettings.mTurnsPerRound = turnsPerRound;
  this->gameSettings.mRoundOverWhenPassed = roundOverWhenPassed;
  this->gameSettings.mOrderMayChange = orderMayChange;
  this->gameSettings.mChangeOrderBySteps = changeOrderBySteps;
  this->gameSettings.mPredictablePlayerOrder = predictablePlayerOrder;
  this->gameSettings.mLogAfterRounds = logAfterRounds;
}

const unsigned int Game::getNofSettings()
{
  return this->gameSettings.nofSettings;
}


Game::state_t Game::currentGameState()
{
  return mState;
}

const Game::Action Game::playOrderSelection(const Action action)
{
  if (orderIsSelected())
  {
    if (action.type == BTN_SHORT)
    {
      //selectOrder method will deSelect order if necessary
      return selectOrder(action);
    }

    if (action.type == BTN_LONG)
    {
      cleanOrder();
      nextState();
      return Action(mPlayers[nextInOrder()].color(), ALL_OFF_EXCEPT_ONE);
    }
  }
  return selectOrder(action);
}

const Game::Action Game::playTurns(const Action action)
{
  if (action.type == BTN_SHORT ||
      action.type == BTN_LONG)
  {
    return playSingleTurn(action);
  }
  
  return Action(UNDEFINED, UNKNOWN);
}

bool Game::orderIsSelected()
{
  if (mJoinedPlayers <= 0)
  {
    return false;
  }

  for (int i = 0; i < mMaxNofPlayers; i++)
  {
    if (mPlayers[i].isPlaying() != mPlayers[i].turnSelected())
    {
      return false;
    }
  }
  return true;
}

const Game::Action Game::selectOrder(const Action action)
{
  if (action.type == BTN_SHORT)
  {
    Player& player = mPlayers[static_cast<unsigned int>(action.color)];
    
    //If player hasn't yet chosen turn, allocate now
    if (player.isPlaying())
    {
      if (!player.turnSelected())
      {
        player.turnIndex(mNofTurnsSelected++);
        return Action(action.color, LED_ON);
      }
      return deSelectOrder(action);
    }
  }

  return Action(UNDEFINED, UNKNOWN);
}

const Game::Action Game::deSelectOrder(const Action action)
{
  if (action.type == BTN_SHORT)
  {
    Player& player = mPlayers[static_cast<unsigned int>(action.color)];

    if (player.isPlaying() && player.turnSelected())
    {
      player.turnSelected(false);
      return Action(action.color, LED_OFF);
    }
  }

  return Action(UNDEFINED, UNKNOWN);
}

void Game::cleanOrder()
{
  for (unsigned int orderIndex = 0; orderIndex < mJoinedPlayers; ++orderIndex)
  {
    unsigned int smallestIndex = Player::undefinedTurnIndex;
    unsigned int playerIndex = 0;
    for (unsigned int i = 0; i < mMaxNofPlayers; ++i)
    {
      if (mPlayers[i].isPlaying())
      {
        if (mPlayers[i].turnIndex() < smallestIndex &&
            mPlayers[i].turnIndex() >= orderIndex)
           {
              smallestIndex = mPlayers[i].turnIndex();
              playerIndex = i;
           }
      }
    }
    mPlayers[playerIndex].turnIndex(orderIndex);
  }
}

unsigned int Game::nextInOrder()
{
  for (unsigned int i = 0; i < mMaxNofPlayers; ++i)
  {
    if (mPlayers[i].turnIndex() == mIndexOfPlayerInTurn)
    {
      return i;
    }
  }

  return 0;
}

unsigned int Game::previousInOrder()
{
  for (unsigned int i = 0; i < mMaxNofPlayers; ++i)
  {
    if (mPlayers[i].turnIndex() == mIndexOfPlayerPreviouslyInTurn)
    {
      return i;
    }
  }

  return 0;
}

void Game::nextPlayer()
{
  unsigned long currentTime_ms = millis();
  Player& player = mPlayers[nextInOrder()];
  player.addTurnLength(currentTime_ms - mTurnStartTime_ms);
  player.addTurn(currentTime_ms - mTurnStartTime_ms);
  mTurnStartTime_ms = currentTime_ms;

  if (!allPassed())
  {
    mIndexOfPlayerPreviouslyInTurn = mIndexOfPlayerInTurn;
    for (unsigned int i = 0; i < mJoinedPlayers; i++)
    {
      mIndexOfPlayerInTurn = ++mIndexOfPlayerInTurn % mJoinedPlayers;
      if(!mPlayers[nextInOrder()].passed())
      {
        break;
      }
    }
  }
  else
  {
    mIndexOfPlayerInTurn = 0;
    mIndexOfPlayerPreviouslyInTurn = 0;
  }
}

void Game::previousPlayer()
{
  mIndexOfPlayerInTurn = mIndexOfPlayerPreviouslyInTurn;
  //Todo append turn length
}

const Game::Action Game::playSingleTurn(const Action action)
{
  Player& player = mPlayers[static_cast<unsigned int>(action.color)];

  //If someone not in turn pressed:
  if (player.turnIndex() != mIndexOfPlayerInTurn)
  {
    if(action.type == BTN_LONG)
    {
      return continueLastTurn(action);
    }
    
    return Action(UNDEFINED, UNKNOWN);
  }
  
  if(player.isPlaying() && !player.passed())
  {
    unsigned int currentTurnCount = player.turnCount();
    player.turnCount(++currentTurnCount);
    
    if (player.turnCount() >= mTurnsPerRound ||
        (mRoundOverWhenPassed && action.type == BTN_LONG))
    {
      player.passed(true);
      nextPlayer();
    }
    else if (mRoundOverWhenPassed && action.type == BTN_SHORT)
    {
      nextPlayer();
    }
  }
  
  if (allPassed())
  {
    return finishRound();
  }

  return Action(mPlayers[nextInOrder()].color(), ALL_OFF_EXCEPT_ONE);
}

const Game::Action Game::continueLastTurn(const Action action)
{
  Player& player = mPlayers[static_cast<unsigned int>(action.color)];

  //If someone not previously in turn pressed, return
  if (player.turnIndex() != mIndexOfPlayerPreviouslyInTurn)
  {
    return Action(UNDEFINED, UNKNOWN);
  }

  if(player.isPlaying())
  {
    if (player.passed())
    {
      player.passed(false);
    }

    unsigned int currentTurnCount = player.turnCount();
    player.turnCount(--currentTurnCount);

    //Remove recently added turn and restore turn length
    unsigned long turnLengthToContinue_ms = player.removeTurn();
    mTurnStartTime_ms = millis() - turnLengthToContinue_ms;

    previousPlayer();

    return Action(mPlayers[nextInOrder()].color(), ALL_OFF_EXCEPT_ONE);
  }
  
  return Action(UNDEFINED, UNKNOWN);
}

bool Game::allPassed()
{
  for (unsigned int i = 0; i < mMaxNofPlayers; ++i)
  {
    if (mPlayers[i].isPlaying() && !mPlayers[i].passed())
    {
      return false;
    }
  }

  return true;
}

void Game::nextState()
{
  switch (mState)
  {
    case ORDER_SELECTION:
      mState = PLAY_TURNS;
      Serial.println(mOrderSelectionStartTime_ms);
      Serial.println(millis());
      Serial.println(mOrderSelectionTime_ms);
      mOrderSelectionTime_ms = millis() - mOrderSelectionStartTime_ms;
      mTurnStartTime_ms = millis();
      break;
    case PLAY_TURNS:
      break;
    default:
      break;
  }
}

const Game::Action Game::finishRound()
{
  unsigned long turnLengths_s[nofColors];
  
  //Save turn lengths to the db (Google drive)
  if (saveToDb)
  {
    for (unsigned int turn = 0; turn < mostCachedTurns(); ++turn)
    {
      for(unsigned int i = 0; i < mMaxNofPlayers; ++i)
      {
        //return 0 if player didn't play turn number "turn"
        turnLengths_s[i] = mPlayers[i].getTurnLength_s(turn);
      }

      //Add turn selection time
      if (turn == 0)
      {
        turnLengths_s[nofColors-1] = mOrderSelectionTime_ms / 1000 +1;
        Serial.println("here");
      }
      else
      {
        turnLengths_s[nofColors-1] = 0;
      }
      Serial.println("Ready to store");
      saveToDb(turnLengths_s);
    }
  }
  
  this->reset();
  return Action(UNDEFINED, BLINK_ALL);
}

unsigned int Game::mostCachedTurns()
{
  unsigned int maxTurns = 0;
  for(unsigned int i = 0; i < mMaxNofPlayers; ++i)
  {
    if (mPlayers[i].getNofCachedTurns() > maxTurns)
    {
      maxTurns = mPlayers[i].getNofCachedTurns();
    }
  }
  return maxTurns;
}
