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
  mTurnLength_ms = 0; 
}

void Player::setColor(Color newColor)
{
  this->mColor = newColor;
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
  mPassed = newState;
}

bool Player::turnSelected()
{
  return mTurnSelected;
}

void Player::turnSelected(bool newState)
{
  mTurnSelected = newState;
}

bool Player::turnDone()
{
  return mTurnDone;
}

void Player::turnDone(bool newState)
{
  mTurnDone = newState;
}

unsigned int Player::turnIndex()
{
  return mTurnIndex;
}

void Player::setTurnIndex(const unsigned int newTurnIndex)
{
  mTurnIndex = newTurnIndex;
  this->turnSelected(true);
}
