//This file defines different kind of effects to apply for LED

enum EffectType {
  //BLINK,
  //FADE
};

typedef struct Effect
{
  EffectType type;
  int duration_ms;
  int frequency;
  
} Effect;
