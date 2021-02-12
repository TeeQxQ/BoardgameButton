#ifndef Game_h
#define Game_h

#include <Arduino.h>
#include "colors.h"
#include "events.h"
#include "player.h"

class Game
{
  typedef struct Action
  {
    Color color;
    EventType type;

    Action(Color color, EventType type)
    {
      this->color = color;
      this->type = type;
    }
    
  } Action;

  public:
    Game();
    void init();
    void play();
    void reset();
    void save();
    void load();
    void addPlayer(Color color);
    void removePlayer(Color color);

  private:
    static const int maxNofPlayers = 5;
    Player players[maxNofPlayers];

    //Settings
    bool predictablePlayerOrderEnabled = true;  
    bool passOverEnabled = false;
    bool changePlayerOrderEnabled = true;
    bool changeOrderByOneStepEnabled = false;
    bool onlyOneTurnPerPhase = true;

    bool orderIsSelected();
    void selectOrder();
    void finishTurn();
    
};

#endif
