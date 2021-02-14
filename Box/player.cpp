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

void Player::addTurnLength(unsigned long length_ms)
{
  mTurnLength_ms += length_ms;
}

void Player::resetTurnLength()
{
  mTurnLength_ms = 0;
}
