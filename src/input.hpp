#ifndef INPUT_H
#define INPUT_H

#include <iostream>

typedef enum InputType {
    MOUSE, KEYBOARD
} InputType;

typedef enum KEY {
    KEY_ENTER
} KEY;


typedef enum MOUSE_BUTTON {
    MB_MIDDLE, MB_LEFT, MB_RIGHT
} MOUSE_BUTTON;

class Input
{
private:
    InputType type;
    int x, y; // mouse position
    MOUSE_BUTTON mb; // mouse button
    KEY key;  // key press
public:
    Input(MOUSE_BUTTON mb, int x, int y);
    Input(KEY key);
    ~Input();

    InputType getType() const;
    int getX() const;
    int getY() const;
    MOUSE_BUTTON getButton() const;
    KEY getKey() const;

};

std::ostream& operator<<(std::ostream& out, Input in);

#endif // INPUT_H
