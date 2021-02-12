#include "game.h"

Game::Game()
{
  init();
}

void Game::init()
{
  for (int i = 0; i < maxNofPlayers; i++)
  {
    if (i < nofColors)
    {
      players[i].setColor(static_cast<Color>(i));
    }
  }
}

void Game::play()
{
  if (orderIsSelected())
  {
    
  }
  selectOrder();
}

void Game::reset()
{
  //reset players
  for (int i = 0; i < maxNofPlayers; i++)
  {
    players[i].reset();
  }
}

void Game::addPlayer(Color color)
{
  int indexOfColor = static_cast<int>(color);
  if (indexOfColor < maxNofPlayers)
  {
    players[indexOfColor].isPlaying = true;
  }
}

void Game::removePlayer(Color color)
{
  int indexOfColor = static_cast<int>(color);
  if (indexOfColor < maxNofPlayers)
  {
    players[indexOfColor].isPlaying = false;
  }
}

bool Game::orderIsSelected()
{
  for (int i = 0; i < maxNofPlayers; i++)
  {
    if (players[i].isPlaying != players[i].turnSelected)
    {
      return false;
    }
  }
  return true;
}

void Game::selectOrder()
{
  
}
