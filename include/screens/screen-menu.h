#ifndef SCREEN_MENU_H
#define SCREEN_MENU_H
#include "lines/scrolling-line.h"
#include "screens/screen.h"
#include "screens/screen_state.h"
#include <chrono>
#include <cppgpio.hpp>

class ScreenMenu : public Screen {
public:
  ScreenMenu(int letter_spaceing, Font *font, int screen_width,
             ScreenState *state, std::vector<Screen *> *screens);
  void render(FrameCanvas *offscreen_canvas, char opacity = 0xFF);
  std::string *getName();

private:
  void scrollMenu(bool up);
  void modeChange();
  bool debounceTimePassed();
  std::string name;
  std::vector<Screen *> *screens;
  enum MenuMode { main_menu, brightness_menu, speed_menu, switch_screen };
  MenuMode current_mode = main_menu;
  ScrollingLine menu_line;
  ScrollingLine menu_sub_line;
  ScreenState *state;
  int current_menu_item;
  int current_screen;
  std::vector<std::string> menu_items;
  std::chrono::time_point<std::chrono::system_clock> last_button_press;
};
#endif /*SCREEN_MENU_H*/
