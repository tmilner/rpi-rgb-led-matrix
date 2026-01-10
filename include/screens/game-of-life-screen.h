#ifndef GAME_OF_LIFE_SCREEN_H
#define GAME_OF_LIFE_SCREEN_H

#include "screens/updatable-screen.h"
#include <chrono>
#include <mutex>
class GameOfLifeScreen : public UpdatableScreen {
public:
  GameOfLifeScreen(rgb_matrix::FrameCanvas *canvas, int delay_ms = 500,
                   bool torus = true);
  ~GameOfLifeScreen();
  void render(rgb_matrix::FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();
  void update();

private:
  void seed();
  std::chrono::time_point<std::chrono::system_clock> last_reseed;
  int numAliveNeighbours(int x, int y);
  int **values_;
  int **newValues_;
  int delay_ms_;
  int r_;
  int g_;
  int b_;
  int width_;
  int height_;
  int game_width;
  int game_height;
  bool torus_;
  std::string name;
  mutable std::mutex life_mutex;
};

#endif /*GAME_OF_LIFE_SCREEN_H*/
