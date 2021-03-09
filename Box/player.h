#ifndef Player_h
#define Player_h

#include "colors.h"
#include "common.h"

class Player
{
  public:
    static const unsigned int undefinedTurnIndex = 99;

    Player();
    Player(Color color);
    void reset();
    void setColor(Color newColor);

    //Getters and setters
    const Color color();
    bool isPlaying();
    void isPlaying(bool newState);
    bool passed();
    void passed(bool newState);
    bool turnSelected();
    void turnSelected(bool newState);
    bool turnDone();
    void turnDone(bool newState);
    unsigned int turnIndex();
    void turnIndex(const unsigned int newTurnIndex);
    unsigned int turnCount();
    void turnCount(unsigned int newCount);
    unsigned long turnLength();
    unsigned long turnLength_s();
    void addTurnLength(unsigned long length_ms);
    void resetTurnLength();
    void addTurn(unsigned long length_ms);
    unsigned long getTurnLength_s(unsigned int atIndex);
    void resetTurns();
    unsigned int getNofCachedTurns();

  private:
    //static const unsigned int mMaxNofCachedTurns = 100;
  
    Color mColor;
    bool mIsPlaying;
    bool mPassed;
    bool mTurnSelected;
    unsigned int mTurnCount;  //Cumulative count of the turns done during this phase
    bool mTurnDone;
    unsigned int mTurnIndex;
    unsigned long mTurnLength_ms;
    unsigned long mCachedTurns[maxNofCachedTurns];
    unsigned int mNofCachedTurns;

};

#endif
/*
typedef struct Player
{
  Color color;
  bool isPlaying;
  bool passed;
  bool turnSelected;
  bool turnDone;
  String turnLength;
  
} Player;

//Array to store all possible players
Player players[nofColors];
int nofPlayers = 0;

//Add unique color for every player
void initializePlayers()
{
  for (size_t color = 0; color < nofColors; color++)
  {
    players[static_cast<int>(color)].color = static_cast<Color>(color);
    players[static_cast<int>(color)].isPlaying = false;
    players[static_cast<int>(color)].passed = false;
    players[static_cast<int>(color)].turnSelected = false;
    players[static_cast<int>(color)].turnDone = false;
    players[static_cast<int>(color)].turnLength = "";
    
  }
}*/
