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

    //Add new turn to the cache
    void addTurn(unsigned long length_ms);

    //Remove the latest turn
    unsigned long removeTurn();
    
    //Add more time to the latest added turn
    void extendLatestTurn(unsigned long lengthToAdd_ms);

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
