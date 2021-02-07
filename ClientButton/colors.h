//This file defines different IDs for different colors

//This must match with the enum definitions
//UNDEFINED not counted
const int nofColors = 5;

enum Color {
  RED = 0,
  BLUE = 1,
  GREEN = 2,
  YELLOW = 3,
  WHITE = 4,
  UNDEFINED = 5
};

String colorToString(Color c)
{
  String s = "";
  switch(static_cast<int>(c))
  {
    case RED:
      s = "RED";
      break;
    case BLUE:
      s = "BLUE";
      break;
    case GREEN:
      s = "GREEN";
      break;
    case YELLOW:
      s = "YELLOW";
      break;
    case WHITE:
      s = "WHITE";
      break;
    default:
      s = "UNDEFINED";
      break;
  }

  return s;
}
