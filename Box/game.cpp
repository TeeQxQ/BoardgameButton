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
  fetchGameSettings();
  
  //reset players
  for (int i = 0; i < mMaxNofPlayers; i++)
  {
    mPlayers[i].reset();
  }

  mNofTurnsSelected = 0;
  mIndexOfPlayerInTurn = 0;
  mState = ORDER_SELECTION;
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

void Game::fetchGameSettings()
{
  //TODO

  //mPredictablePlayerOrder = true;  
  //mPassOver = false;
  //mChangePlayerOrder = true;
  //mChangeOrderByOneStep = false;
  //mOnlyOneTurnPerPhase = true;
  mTurnsPerRound = UNLIMITED;               //Number of turns in a single round (0 - UNLIMITED)
  mRoundOverWhenPassed = true;     //Round ends only after passing
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

void Game::nextPlayer()
{
  unsigned long currentTime_ms = millis();
  Player& player = mPlayers[nextInOrder()];
  player.addTurnLength(currentTime_ms - mTurnStartTime_ms);
  player.addTurn(currentTime_ms - mTurnStartTime_ms);
  mTurnStartTime_ms = currentTime_ms;

  if (!allPassed())
  {
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
  }
}

const Game::Action Game::playSingleTurn(const Action action)
{
  Player& player = mPlayers[static_cast<unsigned int>(action.color)];

  //If someone not in turn pressed, return
  if (player.turnIndex() != mIndexOfPlayerInTurn)
  {
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
  unsigned long turnLengths_s[mMaxNofPlayers];
  
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
