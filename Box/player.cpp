#include "player.h"

Player::Player()
{
  color = UNDEFINED;
  isPlaying = false;
  reset();
}

Player::Player(Color color)
{
  this->color = color;
  isPlaying = false;
  reset();
}

void Player::reset()
{
  passed = false;
  turnSelected = false;
  turnDone = false;
  turnLength_ms = 0; 
}

void Player::setColor(Color newColor)
{
  this->color = newColor;
}
