#ifndef _DISPLAY_H
#define _DISPLAY_H
#include <unordered_set>
#include "../window/window.h"
using namespace std;
extern window main_window;
extern window navigation_bar;
//extern window menu_main;
extern window planning_area;;
extern unordered_set<window*> active_windows;
void displayETCS();
#endif