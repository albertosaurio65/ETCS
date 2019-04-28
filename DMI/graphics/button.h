#ifndef _BUTTON_H
#define _BUTTON_H
#include "component.h"
class Button : public Component
{
    protected:
    Button() {isButton = true;}
    public:
    bool enabled = true;
    bool showBorder = true;
    virtual void paint();
    void setPressedAction(function<void()> pressed)
    {
        pressedAction = pressed;
    }
    virtual void setEnabled(bool val) {enabled = val;}
    Button(float sx, float sy, function<void()> display = nullptr, function<void()> pressed = nullptr) : Component(sx,sy,display)
    {
        isButton = true;
        setPressedAction(pressed);
    }
    virtual ~Button(){}
};
#endif