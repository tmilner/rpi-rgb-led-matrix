#include "game-of-life.h"

GameOfLfeScreen::GameOfLfeScreen(rgb_matrix::FrameCanvas *canvas, int delay_ms, bool torus) : delay_ms_(delay_ms), torus_(torus), name{std::string("Game of Life")}
{
    width_ = canvas->width();
    height_ = canvas->height();

    // Allocate memory
    values_ = new int *[width_];
    for (int x = 0; x < width_; ++x)
    {
        values_[x] = new int[height_];
    }
    newValues_ = new int *[width_];
    for (int x = 0; x < width_; ++x)
    {
        newValues_[x] = new int[height_];
    }

    // Init values randomly
    srand(time(NULL));
    for (int x = 0; x < width_; ++x)
    {
        for (int y = 0; y < height_; ++y)
        {
            values_[x][y] = rand() % 2;
        }
    }
    r_ = rand() % 255;
    g_ = rand() % 255;
    b_ = rand() % 255;

    if (r_ < 150 && g_ < 150 && b_ < 150)
    {
        int c = rand() % 3;
        switch (c)
        {
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

GameOfLfeScreen::~GameOfLfeScreen()
{
    for (int x = 0; x < width_; ++x)
    {
        delete[] values_[x];
    }
    delete[] values_;
    for (int x = 0; x < width_; ++x)
    {
        delete[] newValues_[x];
    }
    delete[] newValues_;
}

std::string *GameOfLfeScreen::getName()
{
    return &this->name;
}

void GameOfLfeScreen::render(rgb_matrix::FrameCanvas *offscreen_canvas)
{
    if (!is_visible)
    {
        return;
    }

    updateValues();

    for (int x = 0; x < width_; ++x)
    {
        for (int y = 0; y < height_; ++y)
        {
            if (values_[x][y])
                offscreen_canvas->SetPixel(x, y, r_, g_, b_);
            else
                offscreen_canvas->SetPixel(x, y, 0, 0, 0);
        }
    }
}
int GameOfLfeScreen::numAliveNeighbours(int x, int y)
{
    int num = 0;
    if (torus_)
    {
        // Edges are connected (torus)
        num += values_[(x - 1 + width_) % width_][(y - 1 + height_) % height_];
        num += values_[(x - 1 + width_) % width_][y];
        num += values_[(x - 1 + width_) % width_][(y + 1) % height_];
        num += values_[(x + 1) % width_][(y - 1 + height_) % height_];
        num += values_[(x + 1) % width_][y];
        num += values_[(x + 1) % width_][(y + 1) % height_];
        num += values_[x][(y - 1 + height_) % height_];
        num += values_[x][(y + 1) % height_];
    }
    else
    {
        // Edges are not connected (no torus)
        if (x > 0)
        {
            if (y > 0)
                num += values_[x - 1][y - 1];
            if (y < height_ - 1)
                num += values_[x - 1][y + 1];
            num += values_[x - 1][y];
        }
        if (x < width_ - 1)
        {
            if (y > 0)
                num += values_[x + 1][y - 1];
            if (y < 31)
                num += values_[x + 1][y + 1];
            num += values_[x + 1][y];
        }
        if (y > 0)
            num += values_[x][y - 1];
        if (y < height_ - 1)
            num += values_[x][y + 1];
    }
    return num;
}

void GameOfLfeScreen::updateValues()
{
    // Copy values to newValues
    for (int x = 0; x < width_; ++x)
    {
        for (int y = 0; y < height_; ++y)
        {
            newValues_[x][y] = values_[x][y];
        }
    }
    // update newValues based on values
    for (int x = 0; x < width_; ++x)
    {
        for (int y = 0; y < height_; ++y)
        {
            int num = numAliveNeighbours(x, y);
            if (values_[x][y])
            {
                // cell is alive
                if (num < 2 || num > 3)
                    newValues_[x][y] = 0;
            }
            else
            {
                // cell is dead
                if (num == 3)
                    newValues_[x][y] = 1;
            }
        }
    }
    // copy newValues to values
    for (int x = 0; x < width_; ++x)
    {
        for (int y = 0; y < height_; ++y)
        {
            values_[x][y] = newValues_[x][y];
        }
    }
}
