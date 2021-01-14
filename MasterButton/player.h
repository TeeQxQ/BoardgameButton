typedef struct Player
{
  Color color;
  bool isPlaying;
  bool passed;
  bool turnSelected;
  bool turnDone;
  String turnLength;
  
} Player;

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
}
