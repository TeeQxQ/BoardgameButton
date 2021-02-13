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

const Game::Action Game::play(const Action action)
{
  if (orderIsSelected())
  {
    Serial.println("Order selected");
  }
  return selectOrder(action);
}

void Game::reset()
{
  //reset players
  for (int i = 0; i < mMaxNofPlayers; i++)
  {
    mPlayers[i].reset();
  }

  mNofTurnsSelected = 0;
}

bool Game::addPlayer(Color color)
{
  //Add player unless it is already in game
  int indexOfColor = static_cast<int>(color);
  if (indexOfColor < mMaxNofPlayers && !mPlayers[indexOfColor].isPlaying())
  {
    mPlayers[indexOfColor].isPlaying(true);
    mJoinedPlayers++;
    return true;
  }

  return false;
}

bool Game::removePlayer(Color color)
{
  //Remove player unless it is not in game
  int indexOfColor = static_cast<int>(color);
  if (indexOfColor < mMaxNofPlayers && mPlayers[indexOfColor].isPlaying())
  {
    mPlayers[indexOfColor].isPlaying(false);
    mJoinedPlayers--;
    return true;
  } 

  return false;
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
    if (player.isPlaying() && !player.turnSelected())
    {
      player.setTurnIndex(mNofTurnsSelected++);
      return Action(action.color, LED_ON);
    }
  }

  return Action(UNDEFINED, UNKNOWN);
}
