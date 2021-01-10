typedef struct Player
{
  Color color;
  bool isPlaying;
  
} Player;

Player players[nofColors];


void initializePlayers()
{
  for (size_t color = 0; color < nofColors; color++)
  {
    players[static_cast<int>(color)].color = static_cast<Color>(color);
  }
}
