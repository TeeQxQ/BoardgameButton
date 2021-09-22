#ifndef Game_h
#define Game_h

#include <Arduino.h>
#include "colors.h"
#include "common.h"
#include "events.h"
#include "player.h"

class Game
{
  typedef void(*dbFunc_t)(unsigned long[nofColors]);
  
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

    typedef struct Settings
    {
      int mTurnsPerRound;
      bool mRoundOverWhenPassed;
      bool mOrderMayChange;
      unsigned int mChangeOrderBySteps;
      bool mPredictablePlayerOrder;
      unsigned int mLogAfterRounds;
      const unsigned int nofSettings = 6;
    } Settings;
  
    Game();
    void init();
    void init(dbFunc_t saveToDriveFunc);
    void reset();
    void save();
    void load();
    void printSettings();
    void changeSettings(int turnsPerRound,
                        bool roundOverWhenPassed,
                        bool orderMayChange,
                        unsigned int changeOrderBySteps,
                        bool predictablePlayerOrder,
                        unsigned int logAfterRounds);
    const unsigned int getNofSettings();
    const Game::Action play(const Action action);
    const Game::Action addPlayer(Color color);
    const Game::Action removePlayer(Color color);
    const Game::Action rejoinPlayer(Color color);

  private:

    enum state_t
    {
      ORDER_SELECTION,
      PLAY_TURNS
    };

    //Current state of the game
    Game::state_t mState;

    static const int mMaxNofPlayers = 5;
    Player mPlayers[mMaxNofPlayers];

    //Cumulative count of player joined
    unsigned int mJoinedPlayers;
    
    //Cumulative count of allocated turn orders
    unsigned int mNofTurnsSelected;

    //Current player in turn (not the index in array)
    //E.g. first 0, then 1 etc.
    unsigned int mIndexOfPlayerInTurn;
    //Previous player in turn (not the index in array)
    unsigned int mIndexOfPlayerPreviouslyInTurn;

    //Start time of a single turn
    unsigned long mTurnStartTime_ms;
    //Start time of the turn selection
    unsigned long mOrderSelectionStartTime_ms;
    unsigned long mOrderSelectionTime_ms;

    //Pointer to a function which stores results to the drive
    dbFunc_t saveToDb;

    //Settings
    //bool mPredictablePlayerOrder;  
    //bool mPassOver;
    //bool mChangePlayerOrder;
    //bool mChangeOrderByOneStep;
    //bool mOnlyOneTurnPerPhase;
    unsigned int mTurnsPerRound;
    bool mRoundOverWhenPassed;

    //Definition for unlimited number of turns per player
    static const unsigned int unlimitedTurns = 9999;

    //Settings for the current game
    Game::Settings gameSettings;

    //Initialize game settings
    void initializeSettings();
    
    //Get settings from drive or SD card
    void fetchGameSettings();

    state_t currentGameState();
    void nextState();

    bool orderIsSelected();
    const Game::Action playOrderSelection(const Action action);
    const Game::Action playTurns(const Action action);
    const Game::Action selectOrder(const Action action);
    const Game::Action deSelectOrder(const Action action);
    const Game::Action playSingleTurn(const Action action);
    const Game::Action continueLastTurn(const Action action);
    const Game::Action finishRound();

    //Because of the order deselect/select, turn indexes may include gaps
    //This method removes them
    void cleanOrder();
    
    //Get index of the next (current) player in turn
    unsigned int nextInOrder();

    //Get index of the last player in turn
    unsigned int previousInOrder();
    
    //Pass turn to the next player
    void nextPlayer();

    //Pass turn to the previous player
    void previousPlayer();

    //Indicates if all players have passed their turn
    bool allPassed();

    //As players can pass different times, get the max number of turns a player has done in this round
    unsigned int mostCachedTurns();

    
};

#endif
