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
      int data;
  
      Action(Color color, EventType type, int data = 0)
      {
        this->color = color;
        this->type = type;
        this->data = data;
      }
      
    } Action;
  
    Game();
    void init();
    const Game::Action play(const Action action);
    void reset();
    void save();
    void load();
    const Game::Action addPlayer(Color color);
    const Game::Action removePlayer(Color color);
    const Game::Action rejoinPlayer(Color color);

  private:

    enum state_t
    {
      ORDER_SELECTION,
      PLAY_TURNS
    };

    Game::state_t state;
    static const int mMaxNofPlayers = 5;
    Player mPlayers[mMaxNofPlayers];

    unsigned int mJoinedPlayers;
    
    //Cumulative count of allocated turn orders
    unsigned int mNofTurnsSelected;

    //Settings
    bool mPredictablePlayerOrder = true;  
    bool mPassOver = false;
    bool mChangePlayerOrder = true;
    bool mChangeOrderByOneStep = false;
    bool mOnlyOneTurnPerPhase = true;

    state_t currentGameState();
    void nextState();
    
    const Game::Action playOrderSelection(const Action action);
    bool orderIsSelected();
    const Game::Action selectOrder(const Action action);
    const Game::Action deSelectOrder(const Action action);
    
    void finishTurn();
    
};

#endif
