#include "screen-menu.h"
#include "img_utils.h"
#include <iomanip>
#include <iostream>
#include <sstream>

ScreenMenu::ScreenMenu(int letter_spaceing, Font *font, int screen_width,
                       ScreenState *state, GPIO::PushButton *button_ok,
                       GPIO::PushButton *button_up,
                       GPIO::PushButton *button_down,
                       std::vector<Screen *> *screens)
    : menu_line{ScrollingLineSettings(&state->speed, 4, letter_spaceing, font,
                                      Color(130, 100, 73), screen_width, 0)},
      menu_sub_line{ScrollingLineSettings(&state->speed, 15, letter_spaceing,
                                          font, Color(130, 100, 73),
                                          screen_width, 0)},
      screens{screens}, name{std::string("Menu")} {
  this->is_visible = false;
  this->state = state;
  this->current_menu_item = 0;
  this->current_screen = 0;
  this->menu_items = {"Brightness", "Speed", "Screen", "Exit"};
  this->last_button_press = std::chrono::system_clock::now();

  button_ok->f_released = [&](std::chrono::nanoseconds nano) {
    this->modeChange();
  };

  button_up->f_released = [&](std::chrono::nanoseconds nano) {
    this->scrollMenu(true);
  };

  button_down->f_released = [&](std::chrono::nanoseconds nano) {
    this->scrollMenu(false);
  };

  button_ok->start();
  button_up->start();
  button_down->start();
}

bool ScreenMenu::debounceTimePassed() {
  using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

  const auto now = std::chrono::system_clock::now();

  if (((now - this->last_button_press) / 1ms) < 100) {
    return false;
  } else {
    this->last_button_press = now;
    return true;
  }
}

std::string *ScreenMenu::getName() { return &this->name; }

void ScreenMenu::scrollMenu(bool up) {
  if (!debounceTimePassed()) {
    return;
  }

  if (this->current_mode == MenuMode::main_menu) {

    if (up) {
      std::cout << "Main Menu. Scroll up!!" << this->current_menu_item
                << ", Count == " << menu_items.size() << std::endl;

      if (this->current_menu_item + 1 >= this->menu_items.size()) {
        this->current_menu_item = 0;
      } else {
        this->current_menu_item++;
      }
    } else {
      std::cout << "Main Menu. Scroll down!!" << this->current_menu_item
                << ", Count == " << menu_items.size() << std::endl;

      if (this->current_menu_item == 0) {
        this->current_menu_item = (this->menu_items.size() - 1);
      } else {
        this->current_menu_item--;
      }
    }
  } else if (this->current_mode == MenuMode::switch_screen) {
    if (up) {
      if (this->current_screen + 1 >= this->screens->size()) {
        this->current_screen = 0;
      } else {
        this->current_screen++;
      }
    } else {
      if (this->current_screen == 0) {
        this->current_screen = this->screens->size() - 1;
      } else {
        this->current_screen--;
      }
    }
  } else if (this->current_mode == MenuMode::brightness_menu) {
    if (up) {
      std::cout << "Brightness Menu. Scroll up!!"
                << this->state->current_brightness << std::endl;

      if (this->state->current_brightness != 100) {
        int increment_by = 5;
        if (this->state->current_brightness + increment_by > 100) {
          this->state->current_brightness = 100;
        } else {
          this->state->current_brightness += increment_by;
        }
      }
    } else {
      std::cout << "Brightness Menu. Scroll down!!"
                << this->state->current_brightness << std::endl;

      if (this->state->current_brightness != 0) {
        int decrement_by = 5;
        if (this->state->current_brightness - decrement_by < 0) {
          this->state->current_brightness = 0;
        } else {
          this->state->current_brightness -= decrement_by;
        }
      }
    }
  } else if (this->current_mode == MenuMode::speed_menu) {
    if (up) {
      std::cout << "Speed Menu. Scroll up!!" << this->state->speed << std::endl;

      if (this->state->speed != 100) {
        float increment_by = 0.1f;
        if (this->state->speed + increment_by > 10) {
          this->state->speed = 10;
        } else {
          this->state->speed += increment_by;
        }
      }
    } else {
      std::cout << "Speed Menu. Scroll down!!" << this->state->speed
                << std::endl;

      if (this->state->speed != 0) {
        float decrement_by = 0.1f;
        if (this->state->speed - decrement_by < 0) {
          this->state->speed = 0;
        } else {
          this->state->speed -= decrement_by;
        }
      }
    }
  }
}

void ScreenMenu::modeChange() {
  if (!debounceTimePassed()) {
    return;
  }
  std::cout << "Pressed! Current Mode " << this->state->current_mode
            << std::endl;
  if (!this->is_visible) {
    std::cout << "pressed go to menu" << std::endl;
    this->is_visible = true;
    this->current_mode = MenuMode::main_menu;
  } else if (this->current_mode == MenuMode::main_menu &&
             this->current_menu_item == 0) {
    std::cout << "pressed go to brightness" << std::endl;
    this->current_mode = MenuMode::brightness_menu;
  } else if (this->current_mode == MenuMode::main_menu &&
             this->current_menu_item == 1) {
    std::cout << "pressed go to speed menu" << std::endl;
    this->current_mode = MenuMode::speed_menu;
  } else if (this->current_mode == MenuMode::main_menu &&
             this->current_menu_item == 2) {
    std::cout << "pressed go to switch order menu" << std::endl;
    this->current_screen = 0;
    this->current_mode = MenuMode::switch_screen;
  } else if (this->current_mode == MenuMode::switch_screen) {
    std::cout << "pressed go to change screen" << std::endl;

    int x = 0;
    for (std::vector<Screen *>::iterator screen = this->screens->begin();
         screen != this->screens->end(); screen++) {
      if (x++ != this->current_screen)
        (*screen)->set_hidden();
      else
        (*screen)->set_visible();
    }

    this->is_visible = false;
    this->current_mode = MenuMode::main_menu;
  } else {
    std::cout << "pressed go to leave menu" << std::endl;
    this->is_visible = false;
    this->current_mode = MenuMode::main_menu;
  }
  return;
}

void ScreenMenu::render(FrameCanvas *offscreen_canvas, char opacity) {
  if (!this->is_visible) {
    return;
  }
  using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

  const auto now = std::chrono::system_clock::now();

  if (((now - this->last_button_press) / 1s) > 100) {
    this->is_visible = false;
    return;
  }

  offscreen_canvas->SetPixels(1, 1, offscreen_canvas->width() - 2,
                              offscreen_canvas->height() - 2, 130, 100, 73);
  offscreen_canvas->SetPixels(2, 2, offscreen_canvas->width() - 4,
                              offscreen_canvas->height() - 4, 0, 0, 0);
  rgb_matrix::DrawLine(offscreen_canvas, 2, 13, offscreen_canvas->width() - 4,
                       13, Color(130, 100, 73));
  if (this->current_mode == brightness_menu) {
    menu_sub_line.updateText(
        &std::to_string(this->state->current_brightness).append("%"));
  }
  if (this->current_mode == speed_menu) {
    std::stringstream temp_str_stream;
    temp_str_stream << std::fixed << std::setprecision(2) << this->state->speed;
    std::string temp_str = temp_str_stream.str();
    menu_sub_line.updateText(&temp_str);
  } else if (this->current_mode == switch_screen) {
    menu_sub_line.updateText(
        this->screens->at(this->current_screen)->getName());
  } else {
    std::string enter{"..."};
    menu_sub_line.updateText(&enter);
  }
  menu_sub_line.renderLine(offscreen_canvas);

  menu_line.updateText(&this->menu_items[this->current_menu_item]);
  menu_line.renderLine(offscreen_canvas);
}
