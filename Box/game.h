#ifndef Game_h
#define Game_h

#include <Arduino.h>
#include "colors.h"
#include "events.h"
#include "player.h"

class Game
{
  public:

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
  
    Game();
    void init();
    const Game::Action play(const Action action);
    void reset();
    void save();
    void load();
    bool addPlayer(Color color);
    bool removePlayer(Color color);

  private:
    static const int mMaxNofPlayers = 5;
    Player mPlayers[mMaxNofPlayers];

    unsigned int mJoinedPlayers;
    
    //Cumulative count of allocated turn orders
    unsigned int mNofTurnsSelected;

    //Settings
    bool mPredictablePlayerOrderEnabled = true;  
    bool mPassOverEnabled = false;
    bool mChangePlayerOrderEnabled = true;
    bool mChangeOrderByOneStepEnabled = false;
    bool mOnlyOneTurnPerPhase = true;

    bool orderIsSelected();
    const Game::Action selectOrder(const Action action);
    const Game::Action deSelectOrder(const Action action);
    void finishTurn();
    
};

#endif
