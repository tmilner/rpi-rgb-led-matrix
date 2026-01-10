#include "screens/game-of-life.h"
#include <iostream>

GameOfLfeScreen::GameOfLfeScreen(rgb_matrix::FrameCanvas *canvas, int delay_ms,
                                 bool torus)
    : delay_ms_(delay_ms), torus_(torus), name{std::string("Game of Life")} {
  width_ = canvas->width();
  height_ = canvas->height();
  game_width = 500;
  game_height = 500;
  this->last_reseed = std::chrono::system_clock::now();

  // Allocate memory
  values_ = new int *[game_width];
  for (int x = 0; x < game_width; ++x) {
    values_[x] = new int[game_height];
  }
  newValues_ = new int *[game_width];
  for (int x = 0; x < game_width; ++x) {
    newValues_[x] = new int[game_height];
  }
  this->seed();
}

void GameOfLfeScreen::seed() {
  // Init values randomly
  srand(time(NULL));
  for (int x = 0; x < game_width; ++x) {
    for (int y = 0; y < game_height; ++y) {
      values_[x][y] = rand() % 2;
    }
  }
  r_ = rand() % 255;
  g_ = rand() % 255;
  b_ = rand() % 255;

  if (r_ < 150 && g_ < 150 && b_ < 150) {
    int c = rand() % 3;
    switch (c) {
    case 0:
      r_ = 200;
      break;
    case 1:
      g_ = 200;
      break;
    case 2:
      b_ = 200;
      break;
    }
  }
}

GameOfLfeScreen::~GameOfLfeScreen() {
  for (int x = 0; x < game_width; ++x) {
    delete[] values_[x];
  }
  delete[] values_;
  for (int x = 0; x < game_width; ++x) {
    delete[] newValues_[x];
  }
  delete[] newValues_;
}

std::string *GameOfLfeScreen::getName() { return &this->name; }

void GameOfLfeScreen::render(rgb_matrix::FrameCanvas *offscreen_canvas,
                             char opacity) {
  if (!is_visible) {
    return;
  }

  std::lock_guard<std::mutex> lock(life_mutex);
  for (int x = 100; x < width_ + 100; ++x) {
    for (int y = 100; y < height_ + 100; ++y) {
      if (values_[x][y])
        offscreen_canvas->SetPixel(x - 100, y - 100, r_, g_, b_);
      else
        offscreen_canvas->SetPixel(x - 100, y - 100, 0, 0, 0);
    }
  }
}
int GameOfLfeScreen::numAliveNeighbours(int x, int y) {
  int num = 0;
  if (torus_) {
    // Edges are connected (torus)
    num += values_[(x - 1 + game_width) % game_width]
                  [(y - 1 + game_height) % game_height];
    num += values_[(x - 1 + game_width) % game_width][y];
    num += values_[(x - 1 + game_width) % game_width][(y + 1) % game_height];
    num += values_[(x + 1) % game_width][(y - 1 + game_height) % game_height];
    num += values_[(x + 1) % game_width][y];
    num += values_[(x + 1) % game_width][(y + 1) % game_height];
    num += values_[x][(y - 1 + game_height) % game_height];
    num += values_[x][(y + 1) % game_height];
  } else {
    // Edges are not connected (no torus)
    if (x > 0) {
      if (y > 0)
        num += values_[x - 1][y - 1];
      if (y < game_height - 1)
        num += values_[x - 1][y + 1];
      num += values_[x - 1][y];
    }
    if (x < game_width - 1) {
      if (y > 0)
        num += values_[x + 1][y - 1];
      if (y < 31)
        num += values_[x + 1][y + 1];
      num += values_[x + 1][y];
    }
    if (y > 0)
      num += values_[x][y - 1];
    if (y < game_height - 1)
      num += values_[x][y + 1];
  }
  return num;
}

void GameOfLfeScreen::update() {
  if (!is_visible) {
    return;
  }

  std::cout << "Update Game of Life" << std::endl;

  std::lock_guard<std::mutex> lock(life_mutex);
  // Copy values to newValues
  for (int x = 0; x < game_width; ++x) {
    for (int y = 0; y < game_height; ++y) {
      newValues_[x][y] = values_[x][y];
    }
  }
  // update newValues based on values
  for (int x = 0; x < game_width; ++x) {
    for (int y = 0; y < game_height; ++y) {
      int num = numAliveNeighbours(x, y);
      if (values_[x][y]) {
        // cell is alive
        if (num < 2 || num > 3)
          newValues_[x][y] = 0;
      } else {
        // cell is dead
        if (num == 3)
          newValues_[x][y] = 1;
      }
    }
  }
  // copy newValues to values
  for (int x = 0; x < game_width; ++x) {
    for (int y = 0; y < game_height; ++y) {
      values_[x][y] = newValues_[x][y];
    }
  }
  std::cout << "Updated Game of Life" << std::endl;
}
