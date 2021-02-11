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
