#include "display.h"
#include "button.h"
#include "text_button.h"
#include "../window/menu.h"
using namespace std;
unordered_set<window*> active_windows;
#include <iostream>
void displayETCS()
{
    for(auto it=active_windows.begin(); it!=active_windows.end(); ++it)
    {
        window *w = *it;
        if(w->active) w->display();
    }
}