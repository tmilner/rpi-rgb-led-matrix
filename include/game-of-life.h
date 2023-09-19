#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#include "screen.h"

class GameOfLfeScreen : public Screen
{
public:
    GameOfLfeScreen(rgb_matrix::FrameCanvas *canvas, int delay_ms = 500, bool torus = true);
    ~GameOfLfeScreen();
    void render(rgb_matrix::FrameCanvas *offscreen_canvas);

private:
    int numAliveNeighbours(int x, int y);
    void updateValues();
    int **values_;
    int **newValues_;
    int delay_ms_;
    int r_;
    int g_;
    int b_;
    int width_;
    int height_;
    bool torus_;
    std::string name;
};

#endif /*GAME_OF_LIFE_H*/