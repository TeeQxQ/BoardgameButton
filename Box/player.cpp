#include "player.h"

Player::Player()
{
  mColor = UNDEFINED;
  mIsPlaying = false;
  reset();
}

Player::Player(Color color)
{
  setColor(color);
  mIsPlaying = false;
  reset();
}

void Player::reset()
{
  mPassed = false;
  mTurnSelected = false;
  mTurnDone = false;
  mTurnCount = 0;
  mTurnIndex = undefinedTurnIndex;
  resetTurnLength();
  resetTurns();
}

void Player::setColor(Color newColor)
{
  this->mColor = newColor;
}

const Color Player::color()
{
  return mColor;
}

bool Player::isPlaying()
{
  return mIsPlaying;
}

void Player::isPlaying(bool newState)
{
  mIsPlaying = newState;
}

bool Player::passed()
{
  return mPassed;
}

void Player::passed(bool newState)
{
  if (newState == true)
  {
    mTurnCount = 0;
  }
  mPassed = newState;
}

bool Player::turnSelected()
{
  return mTurnSelected;
}

void Player::turnSelected(bool newState)
{
  if (newState == false)
  {
    mTurnIndex = undefinedTurnIndex;
  }
  mTurnSelected = newState;
}

bool Player::turnDone()
{
  return mTurnDone;
}

void Player::turnDone(bool newState)
{
  if (newState == true)
  {
    mTurnCount = 0;
  }
  mTurnDone = newState;
}

unsigned int Player::turnIndex()
{
  return mTurnIndex;
}

void Player::turnIndex(const unsigned int newTurnIndex)
{
  if (newTurnIndex < undefinedTurnIndex)
  {
    mTurnIndex = newTurnIndex;
    this->turnSelected(true);
  }
}

unsigned int Player::turnCount()
{
  return mTurnCount;
}

void Player::turnCount(unsigned int newCount)
{
  mTurnCount = newCount;
}

unsigned long Player::turnLength()
{
  return mTurnLength_ms;
}

unsigned long Player::turnLength_s()
{
  return (mTurnLength_ms > 0) ? mTurnLength_ms / 1000 + 1 : 0;
}

void Player::addTurnLength(unsigned long length_ms)
{
  mTurnLength_ms += length_ms;
}

void Player::resetTurnLength()
{
  mTurnLength_ms = 0;
}

void Player::addTurn(unsigned long length_ms)
{
  if (mNofCachedTurns < maxNofCachedTurns)
  {
    mCachedTurns[mNofCachedTurns] = length_ms;
    ++mNofCachedTurns;
  }
}

unsigned long Player::removeTurn()
{
  if (mNofCachedTurns > 0)
  {
    --mNofCachedTurns;
    return mCachedTurns[mNofCachedTurns];
  }

  return 0;
  
}

void Player::extendLatestTurn(unsigned long lengthToAdd_ms)
{
  if (mNofCachedTurns > 0)
  {
    mCachedTurns[mNofCachedTurns -1] += lengthToAdd_ms;
  }
  else
  {
    mCachedTurns[mNofCachedTurns] += lengthToAdd_ms;
  }
}

unsigned long Player::getTurnLength_s(unsigned int atIndex)
{
  if (atIndex < mNofCachedTurns)
  {
    return (mCachedTurns[atIndex] > 0) ? mCachedTurns[atIndex] / 1000 + 1 : 0;
  }

  return 0;
}

void Player::resetTurns()
{
  mNofCachedTurns = 0;
}

unsigned int Player::getNofCachedTurns()
{
  return mNofCachedTurns;
}
