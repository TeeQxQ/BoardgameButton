#ifndef Game_h
#define Game_h

#include "colors.h"
#include "player.h"

class Game
{
  public:
    Game();
    void init();

  private:
    static const int maxNofPlayers = 5;
    Player players[maxNofPlayers];
};

#endif
